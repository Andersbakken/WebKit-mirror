/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "TestController.h"

#include "PlatformWebView.h"
#include "StringFunctions.h"
#include "TestInvocation.h"
#include <cstdio>
#include <WebKit2/WKPageGroup.h>
#include <WebKit2/WKContextPrivate.h>
#include <WebKit2/WKPreferencesPrivate.h>
#include <wtf/PassOwnPtr.h>

namespace WTR {

static const double defaultLongTimeout = 30;
static const double defaultShortTimeout = 5;

static WKURLRef blankURL()
{
    static WKURLRef staticBlankURL = WKURLCreateWithUTF8CString("about:blank");
    return staticBlankURL;
}

static TestController* controller;

TestController& TestController::shared()
{
    ASSERT(controller);
    return *controller;
}

TestController::TestController(int argc, const char* argv[])
    : m_dumpPixels(false)
    , m_verbose(false)
    , m_printSeparators(false)
    , m_usingServerMode(false)
    , m_state(Initial)
    , m_doneResetting(false)
    , m_longTimeout(defaultLongTimeout)
    , m_shortTimeout(defaultShortTimeout)
{
    initialize(argc, argv);
    controller = this;
    run();
    controller = 0;
}

TestController::~TestController()
{
}

static WKRect getWindowFrameMainPage(WKPageRef page, const void* clientInfo)
{
    PlatformWebView* view = static_cast<TestController*>(const_cast<void*>(clientInfo))->mainWebView();
    return view->windowFrame();
}

static void setWindowFrameMainPage(WKPageRef page, WKRect frame, const void* clientInfo)
{
    PlatformWebView* view = static_cast<TestController*>(const_cast<void*>(clientInfo))->mainWebView();
    view->setWindowFrame(frame);
}

static WKRect getWindowFrameOtherPage(WKPageRef page, const void* clientInfo)
{
    PlatformWebView* view = static_cast<PlatformWebView*>(const_cast<void*>(clientInfo));
    return view->windowFrame();
}

static void setWindowFrameOtherPage(WKPageRef page, WKRect frame, const void* clientInfo)
{
    PlatformWebView* view = static_cast<PlatformWebView*>(const_cast<void*>(clientInfo));
    view->setWindowFrame(frame);
}

static void closeOtherPage(WKPageRef page, const void* clientInfo)
{
    WKPageClose(page);
    const PlatformWebView* view = static_cast<const PlatformWebView*>(clientInfo);
    delete view;
}

static WKPageRef createOtherPage(WKPageRef oldPage, WKDictionaryRef, WKEventModifiers, WKEventMouseButton, const void*)
{
    PlatformWebView* view = new PlatformWebView(WKPageGetContext(oldPage), WKPageGetPageGroup(oldPage));
    WKPageRef newPage = view->page();

    view->resizeTo(800, 600);

    WKPageUIClient otherPageUIClient = {
        0,
        view,
        createOtherPage,
        0, // showPage
        closeOtherPage,
        0, // runJavaScriptAlert        
        0, // runJavaScriptConfirm
        0, // runJavaScriptPrompt
        0, // setStatusText
        0, // mouseDidMoveOverElement
        0, // missingPluginButtonClicked
        0, // didNotHandleKeyEvent
        0, // toolbarsAreVisible
        0, // setToolbarsAreVisible
        0, // menuBarIsVisible
        0, // setMenuBarIsVisible
        0, // statusBarIsVisible
        0, // setStatusBarIsVisible
        0, // isResizable
        0, // setIsResizable
        getWindowFrameOtherPage,
        setWindowFrameOtherPage,
        0, // runBeforeUnloadConfirmPanel
        0, // didDraw
        0, // pageDidScroll
        0, // exceededDatabaseQuota
        0  // runOpenPanel
    };
    WKPageSetPageUIClient(newPage, &otherPageUIClient);

    WKRetain(newPage);
    return newPage;
}

void TestController::initialize(int argc, const char* argv[])
{
    platformInitialize();

    bool printSupportedFeatures = false;

    for (int i = 1; i < argc; ++i) {
        std::string argument(argv[i]);

        if (argument == "--timeout" && i + 1 < argc) {
            m_longTimeout = atoi(argv[++i]);
            // Scale up the short timeout to match.
            m_shortTimeout = defaultShortTimeout * m_longTimeout / defaultLongTimeout;
            continue;
        }
        if (argument == "--pixel-tests") {
            m_dumpPixels = true;
            continue;
        }
        if (argument == "--verbose") {
            m_verbose = true;
            continue;
        }
        if (argument == "--print-supported-features") {
            printSupportedFeatures = true;
            break;
        }

        // Skip any other arguments that begin with '--'.
        if (argument.length() >= 2 && argument[0] == '-' && argument[1] == '-')
            continue;

        m_paths.push_back(argument);
    }

    if (printSupportedFeatures) {
        // FIXME: On Windows, DumpRenderTree uses this to expose whether it supports 3d
        // transforms and accelerated compositing. When we support those features, we
        // should match DRT's behavior.
        exit(0);
    }

    m_usingServerMode = (m_paths.size() == 1 && m_paths[0] == "-");
    if (m_usingServerMode)
        m_printSeparators = true;
    else
        m_printSeparators = m_paths.size() > 1;

    initializeInjectedBundlePath();
    initializeTestPluginDirectory();

    WKRetainPtr<WKStringRef> pageGroupIdentifier(AdoptWK, WKStringCreateWithUTF8CString("WebKitTestRunnerPageGroup"));
    m_pageGroup.adopt(WKPageGroupCreateWithIdentifier(pageGroupIdentifier.get()));

    m_context.adopt(WKContextCreateWithInjectedBundlePath(injectedBundlePath()));
    platformInitializeContext();

    WKContextInjectedBundleClient injectedBundleClient = {
        0,
        this,
        didReceiveMessageFromInjectedBundle,
        didReceiveSynchronousMessageFromInjectedBundle
    };
    WKContextSetInjectedBundleClient(m_context.get(), &injectedBundleClient);

    _WKContextSetAdditionalPluginsDirectory(m_context.get(), testPluginDirectory());

    m_mainWebView = adoptPtr(new PlatformWebView(m_context.get(), m_pageGroup.get()));

    WKPageUIClient pageUIClient = {
        0,
        this,
        createOtherPage,
        0, // showPage
        0, // close
        0, // runJavaScriptAlert        
        0, // runJavaScriptConfirm
        0, // runJavaScriptPrompt
        0, // setStatusText
        0, // mouseDidMoveOverElement
        0, // missingPluginButtonClicked
        0, // didNotHandleKeyEvent
        0, // toolbarsAreVisible
        0, // setToolbarsAreVisible
        0, // menuBarIsVisible
        0, // setMenuBarIsVisible
        0, // statusBarIsVisible
        0, // setStatusBarIsVisible
        0, // isResizable
        0, // setIsResizable
        getWindowFrameMainPage,
        setWindowFrameMainPage,
        0, // runBeforeUnloadConfirmPanel
        0, // didDraw
        0, // pageDidScroll
        0, // exceededDatabaseQuota
        0  // runOpenPanel
    };
    WKPageSetPageUIClient(m_mainWebView->page(), &pageUIClient);

    WKPageLoaderClient pageLoaderClient = {
        0,
        this,
        0, // didStartProvisionalLoadForFrame
        0, // didReceiveServerRedirectForProvisionalLoadForFrame
        0, // didFailProvisionalLoadWithErrorForFrame
        0, // didCommitLoadForFrame
        0, // didFinishDocumentLoadForFrame
        didFinishLoadForFrame,
        0, // didFailLoadWithErrorForFrame
        0, // didSameDocumentNavigationForFrame
        0, // didReceiveTitleForFrame
        0, // didFirstLayoutForFrame
        0, // didFirstVisuallyNonEmptyLayoutForFrame
        0, // didRemoveFrameFromHierarchy
        0, // didDisplayInsecureContentForFrame
        0, // didRunInsecureContentForFrame
        0, // canAuthenticateAgainstProtectionSpaceInFrame
        0, // didReceiveAuthenticationChallengeInFrame
        0, // didStartProgress
        0, // didChangeProgress
        0, // didFinishProgress
        0, // didBecomeUnresponsive
        0, // didBecomeResponsive
        0, // processDidExit
        0 // didChangeBackForwardList
    };
    WKPageSetPageLoaderClient(m_mainWebView->page(), &pageLoaderClient);
}

bool TestController::resetStateToConsistentValues()
{
    m_state = Resetting;

    // FIXME: This function should also ensure that there is only one page open.

    // Reset preferences
    WKPreferencesRef preferences = WKPageGroupGetPreferences(m_pageGroup.get());
    WKPreferencesSetOfflineWebApplicationCacheEnabled(preferences, true);
    WKPreferencesSetFontSmoothingLevel(preferences, kWKFontSmoothingLevelNoSubpixelAntiAliasing);
    WKPreferencesSetXSSAuditorEnabled(preferences, false);
    WKPreferencesSetDeveloperExtrasEnabled(preferences, true);
    WKPreferencesSetJavaScriptCanOpenWindowsAutomatically(preferences, true);

    static WKStringRef standardFontFamily = WKStringCreateWithUTF8CString("Times");
    static WKStringRef cursiveFontFamily = WKStringCreateWithUTF8CString("Apple Chancery");
    static WKStringRef fantasyFontFamily = WKStringCreateWithUTF8CString("Papyrus");
    static WKStringRef fixedFontFamily = WKStringCreateWithUTF8CString("Courier");
    static WKStringRef sansSerifFontFamily = WKStringCreateWithUTF8CString("Helvetica");
    static WKStringRef serifFontFamily = WKStringCreateWithUTF8CString("Times");

    WKPreferencesSetStandardFontFamily(preferences, standardFontFamily);
    WKPreferencesSetCursiveFontFamily(preferences, cursiveFontFamily);
    WKPreferencesSetFantasyFontFamily(preferences, fantasyFontFamily);
    WKPreferencesSetFixedFontFamily(preferences, fixedFontFamily);
    WKPreferencesSetSansSerifFontFamily(preferences, sansSerifFontFamily);
    WKPreferencesSetSerifFontFamily(preferences, serifFontFamily);

    m_mainWebView->focus();

    // Reset main page back to about:blank
    m_doneResetting = false;

    WKPageLoadURL(m_mainWebView->page(), blankURL());
    runUntil(m_doneResetting, ShortTimeout);
    return m_doneResetting;
}

bool TestController::runTest(const char* test)
{
    if (!resetStateToConsistentValues())
        return false;

    m_state = RunningTest;
    m_currentInvocation.set(new TestInvocation(test));
    m_currentInvocation->invoke();
    m_currentInvocation.clear();

    return true;
}

void TestController::runTestingServerLoop()
{
    char filenameBuffer[2048];
    while (fgets(filenameBuffer, sizeof(filenameBuffer), stdin)) {
        char *newLineCharacter = strchr(filenameBuffer, '\n');
        if (newLineCharacter)
            *newLineCharacter = '\0';

        if (strlen(filenameBuffer) == 0)
            continue;

        if (!runTest(filenameBuffer))
            break;
    }
}

void TestController::run()
{
    if (m_usingServerMode)
        runTestingServerLoop();
    else {
        for (size_t i = 0; i < m_paths.size(); ++i) {
            if (!runTest(m_paths[i].c_str()))
                break;
        }
    }
}

void TestController::runUntil(bool& done, TimeoutDuration timeoutDuration)
{
    platformRunUntil(done, timeoutDuration == ShortTimeout ? m_shortTimeout : m_longTimeout);
}

// WKContextInjectedBundleClient

void TestController::didReceiveMessageFromInjectedBundle(WKContextRef context, WKStringRef messageName, WKTypeRef messageBody, const void* clientInfo)
{
    static_cast<TestController*>(const_cast<void*>(clientInfo))->didReceiveMessageFromInjectedBundle(messageName, messageBody);
}

void TestController::didReceiveSynchronousMessageFromInjectedBundle(WKContextRef context, WKStringRef messageName, WKTypeRef messageBody, WKTypeRef* returnData, const void* clientInfo)
{
    *returnData = static_cast<TestController*>(const_cast<void*>(clientInfo))->didReceiveSynchronousMessageFromInjectedBundle(messageName, messageBody).leakRef();
}

void TestController::didReceiveMessageFromInjectedBundle(WKStringRef messageName, WKTypeRef messageBody)
{
    m_currentInvocation->didReceiveMessageFromInjectedBundle(messageName, messageBody);
}

WKRetainPtr<WKTypeRef> TestController::didReceiveSynchronousMessageFromInjectedBundle(WKStringRef messageName, WKTypeRef messageBody)
{
    return m_currentInvocation->didReceiveSynchronousMessageFromInjectedBundle(messageName, messageBody);
}

// WKPageLoaderClient

void TestController::didFinishLoadForFrame(WKPageRef page, WKFrameRef frame, WKTypeRef, const void* clientInfo)
{
    static_cast<TestController*>(const_cast<void*>(clientInfo))->didFinishLoadForFrame(page, frame);
}

void TestController::didFinishLoadForFrame(WKPageRef page, WKFrameRef frame)
{
    if (m_state != Resetting)
        return;

    if (!WKFrameIsMainFrame(frame))
        return;

    WKRetainPtr<WKURLRef> wkURL(AdoptWK, WKFrameCopyURL(frame));
    if (!WKURLIsEqual(wkURL.get(), blankURL()))
        return;

    m_doneResetting = true;
    shared().notifyDone();
}

} // namespace WTR
