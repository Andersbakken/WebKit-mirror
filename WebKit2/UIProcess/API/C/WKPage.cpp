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

#include "WKPage.h"
#include "WKPagePrivate.h"

#include "WKAPICast.h"
#include "WebBackForwardList.h"
#include "WebData.h"
#include "WebPageProxy.h"

#ifdef __BLOCKS__
#include <Block.h>
#endif

using namespace WebKit;

WKTypeID WKPageGetTypeID()
{
    return toAPI(WebPageProxy::APIType);
}

WKContextRef WKPageGetContext(WKPageRef pageRef)
{
    return toAPI(toImpl(pageRef)->context());
}

WKPageGroupRef WKPageGetPageGroup(WKPageRef pageRef)
{
    return toAPI(toImpl(pageRef)->pageGroup());
}

void WKPageLoadURL(WKPageRef pageRef, WKURLRef URLRef)
{
    toImpl(pageRef)->loadURL(toImpl(URLRef)->string());
}

void WKPageLoadURLRequest(WKPageRef pageRef, WKURLRequestRef urlRequestRef)
{
    toImpl(pageRef)->loadURLRequest(toImpl(urlRequestRef));    
}

void WKPageLoadHTMLString(WKPageRef pageRef, WKStringRef htmlStringRef, WKURLRef baseURLRef)
{
    toImpl(pageRef)->loadHTMLString(toWTFString(htmlStringRef), toWTFString(baseURLRef));
}

void WKPageLoadAlternateHTMLString(WKPageRef pageRef, WKStringRef htmlStringRef, WKURLRef baseURLRef, WKURLRef unreachableURLRef)
{
    toImpl(pageRef)->loadAlternateHTMLString(toWTFString(htmlStringRef), toWTFString(baseURLRef), toWTFString(unreachableURLRef));
}

void WKPageLoadPlainTextString(WKPageRef pageRef, WKStringRef plainTextStringRef)
{
    toImpl(pageRef)->loadPlainTextString(toWTFString(plainTextStringRef));    
}

void WKPageStopLoading(WKPageRef pageRef)
{
    toImpl(pageRef)->stopLoading();
}

void WKPageReload(WKPageRef pageRef)
{
    toImpl(pageRef)->reload(false);
}

void WKPageReloadFromOrigin(WKPageRef pageRef)
{
    toImpl(pageRef)->reload(true);
}

bool WKPageTryClose(WKPageRef pageRef)
{
    return toImpl(pageRef)->tryClose();
}

void WKPageClose(WKPageRef pageRef)
{
    toImpl(pageRef)->close();
}

bool WKPageIsClosed(WKPageRef pageRef)
{
    return toImpl(pageRef)->isClosed();
}

void WKPageGoForward(WKPageRef pageRef)
{
    toImpl(pageRef)->goForward();
}

bool WKPageCanGoForward(WKPageRef pageRef)
{
    return toImpl(pageRef)->canGoForward();
}

void WKPageGoBack(WKPageRef pageRef)
{
    toImpl(pageRef)->goBack();
}

bool WKPageCanGoBack(WKPageRef pageRef)
{
    return toImpl(pageRef)->canGoBack();
}

void WKPageGoToBackForwardListItem(WKPageRef pageRef, WKBackForwardListItemRef itemRef)
{
    toImpl(pageRef)->goToBackForwardItem(toImpl(itemRef));
}

WKBackForwardListRef WKPageGetBackForwardList(WKPageRef pageRef)
{
    return toAPI(toImpl(pageRef)->backForwardList());
}

WKStringRef WKPageCopyTitle(WKPageRef pageRef)
{
    return toCopiedAPI(toImpl(pageRef)->pageTitle());
}

WKFrameRef WKPageGetMainFrame(WKPageRef pageRef)
{
    return toAPI(toImpl(pageRef)->mainFrame());
}

WKFrameRef WKPageGetFocusedFrame(WKPageRef pageRef)
{
    return toAPI(toImpl(pageRef)->focusedFrame());
}

#if defined(ENABLE_INSPECTOR) && ENABLE_INSPECTOR
WKInspectorRef WKPageGetInspector(WKPageRef pageRef)
{
    return toAPI(toImpl(pageRef)->inspector());
}
#endif

double WKPageGetEstimatedProgress(WKPageRef pageRef)
{
    return toImpl(pageRef)->estimatedProgress();
}

WKStringRef WKPageCopyUserAgent(WKPageRef pageRef)
{
    return toCopiedAPI(toImpl(pageRef)->userAgent());
}

WKStringRef WKPageCopyApplicationNameForUserAgent(WKPageRef pageRef)
{
    return toCopiedAPI(toImpl(pageRef)->applicationNameForUserAgent());
}

void WKPageSetApplicationNameForUserAgent(WKPageRef pageRef, WKStringRef applicationNameRef)
{
    toImpl(pageRef)->setApplicationNameForUserAgent(toWTFString(applicationNameRef));
}

WKStringRef WKPageCopyCustomUserAgent(WKPageRef pageRef)
{
    return toCopiedAPI(toImpl(pageRef)->customUserAgent());
}

void WKPageSetCustomUserAgent(WKPageRef pageRef, WKStringRef userAgentRef)
{
    toImpl(pageRef)->setCustomUserAgent(toWTFString(userAgentRef));
}

bool WKPageSupportsTextEncoding(WKPageRef pageRef)
{
    return toImpl(pageRef)->supportsTextEncoding();
}

WKStringRef WKPageCopyCustomTextEncodingName(WKPageRef pageRef)
{
    return toCopiedAPI(toImpl(pageRef)->customTextEncodingName());
}

void WKPageSetCustomTextEncodingName(WKPageRef pageRef, WKStringRef encodingNameRef)
{
    toImpl(pageRef)->setCustomTextEncodingName(toWTFString(encodingNameRef));
}

void WKPageTerminate(WKPageRef pageRef)
{
    toImpl(pageRef)->terminateProcess();
}

WKStringRef WKPageGetSessionHistoryURLValueType()
{
    static WebString* sessionHistoryURLValueType = WebString::create("SessionHistoryURL").releaseRef();
    return toAPI(sessionHistoryURLValueType);
}

WKDataRef WKPageCopySessionState(WKPageRef pageRef, void *context, WKPageSessionStateFilterCallback filter)
{
    return toAPI(toImpl(pageRef)->sessionStateData(filter, context).releaseRef());
}

void WKPageRestoreFromSessionState(WKPageRef pageRef, WKDataRef sessionStateData)
{
    toImpl(pageRef)->restoreFromSessionStateData(toImpl(sessionStateData));
}

double WKPageGetTextZoomFactor(WKPageRef pageRef)
{
    return toImpl(pageRef)->textZoomFactor();
}

void WKPageSetTextZoomFactor(WKPageRef pageRef, double zoomFactor)
{
    toImpl(pageRef)->setTextZoomFactor(zoomFactor);
}

double WKPageGetPageZoomFactor(WKPageRef pageRef)
{
    return toImpl(pageRef)->pageZoomFactor();
}

void WKPageSetPageZoomFactor(WKPageRef pageRef, double zoomFactor)
{
    toImpl(pageRef)->setPageZoomFactor(zoomFactor);
}

void WKPageSetPageAndTextZoomFactors(WKPageRef pageRef, double pageZoomFactor, double textZoomFactor)
{
    toImpl(pageRef)->setPageAndTextZoomFactors(pageZoomFactor, textZoomFactor);
}

void WKPageScaleWebView(WKPageRef pageRef, double scale, WKPoint origin)
{
    toImpl(pageRef)->scaleWebView(scale, toIntPoint(origin));
}

double WKPageGetViewScaleFactor(WKPageRef pageRef)
{
    return toImpl(pageRef)->viewScaleFactor();
}

void WKPageFindString(WKPageRef pageRef, WKStringRef string, WKFindOptions options, unsigned maxMatchCount)
{
    toImpl(pageRef)->findString(toImpl(string)->string(), toFindOptions(options), maxMatchCount);
}

void WKPageHideFindUI(WKPageRef pageRef)
{
    toImpl(pageRef)->hideFindUI();
}

void WKPageCountStringMatches(WKPageRef pageRef, WKStringRef string, WKFindOptions options, unsigned maxMatchCount)
{
    toImpl(pageRef)->countStringMatches(toImpl(string)->string(), toFindOptions(options), maxMatchCount);
}

void WKPageSetPageContextMenuClient(WKPageRef pageRef, const WKPageContextMenuClient* wkClient)
{
    if (wkClient && wkClient->version)
        return;
    toImpl(pageRef)->initializeContextMenuClient(wkClient);
}

void WKPageSetPageFindClient(WKPageRef pageRef, const WKPageFindClient* wkClient)
{
    if (wkClient && wkClient->version)
        return;
    toImpl(pageRef)->initializeFindClient(wkClient);
}

void WKPageSetPageFormClient(WKPageRef pageRef, const WKPageFormClient* wkClient)
{
    if (wkClient && wkClient->version)
        return;
    toImpl(pageRef)->initializeFormClient(wkClient);
}

void WKPageSetPageLoaderClient(WKPageRef pageRef, const WKPageLoaderClient* wkClient)
{
    if (wkClient && wkClient->version)
        return;
    toImpl(pageRef)->initializeLoaderClient(wkClient);
}

void WKPageSetPagePolicyClient(WKPageRef pageRef, const WKPagePolicyClient* wkClient)
{
    if (wkClient && wkClient->version)
        return;
    toImpl(pageRef)->initializePolicyClient(wkClient);
}

void WKPageSetPageResourceLoadClient(WKPageRef pageRef, const WKPageResourceLoadClient* wkClient)
{
    if (wkClient && wkClient->version)
        return;
    toImpl(pageRef)->initializeResourceLoadClient(wkClient);
}

void WKPageSetPageUIClient(WKPageRef pageRef, const WKPageUIClient* wkClient)
{
    if (wkClient && wkClient->version)
        return;
    toImpl(pageRef)->initializeUIClient(wkClient);
}

void WKPageRunJavaScriptInMainFrame(WKPageRef pageRef, WKStringRef scriptRef, void* context, WKPageRunJavaScriptFunction callback)
{
    toImpl(pageRef)->runJavaScriptInMainFrame(toImpl(scriptRef)->string(), StringCallback::create(context, callback));
}

#ifdef __BLOCKS__
static void callRunJavaScriptBlockAndRelease(WKStringRef resultValue, WKErrorRef error, void* context)
{
    WKPageRunJavaScriptBlock block = (WKPageRunJavaScriptBlock)context;
    block(resultValue, error);
    Block_release(block);
}

void WKPageRunJavaScriptInMainFrame_b(WKPageRef pageRef, WKStringRef scriptRef, WKPageRunJavaScriptBlock block)
{
    WKPageRunJavaScriptInMainFrame(pageRef, scriptRef, Block_copy(block), callRunJavaScriptBlockAndRelease);
}
#endif

void WKPageRenderTreeExternalRepresentation(WKPageRef pageRef, void* context, WKPageRenderTreeExternalRepresentationFunction callback)
{
    toImpl(pageRef)->getRenderTreeExternalRepresentation(StringCallback::create(context, callback));
}

#ifdef __BLOCKS__
static void callRenderTreeExternalRepresentationBlockAndDispose(WKStringRef resultValue, WKErrorRef error, void* context)
{
    WKPageRenderTreeExternalRepresentationBlock block = (WKPageRenderTreeExternalRepresentationBlock)context;
    block(resultValue, error);
    Block_release(block);
}

void WKPageRenderTreeExternalRepresentation_b(WKPageRef pageRef, WKPageRenderTreeExternalRepresentationBlock block)
{
    WKPageRenderTreeExternalRepresentation(pageRef, Block_copy(block), callRenderTreeExternalRepresentationBlockAndDispose);
}
#endif

void WKPageGetSourceForFrame(WKPageRef pageRef, WKFrameRef frameRef, void* context, WKPageGetSourceForFrameFunction callback)
{
    toImpl(pageRef)->getSourceForFrame(toImpl(frameRef), StringCallback::create(context, callback));
}

#ifdef __BLOCKS__
static void callGetSourceForFrameBlockBlockAndDispose(WKStringRef resultValue, WKErrorRef error, void* context)
{
    WKPageGetSourceForFrameBlock block = (WKPageGetSourceForFrameBlock)context;
    block(resultValue, error);
    Block_release(block);
}

void WKPageGetSourceForFrame_b(WKPageRef pageRef, WKFrameRef frameRef, WKPageGetSourceForFrameBlock block)
{
    WKPageGetSourceForFrame(pageRef, frameRef, Block_copy(block), callGetSourceForFrameBlockBlockAndDispose);
}
#endif

void WKPageGetContentsAsString(WKPageRef pageRef, void* context, WKPageGetContentsAsStringFunction callback)
{
    toImpl(pageRef)->getContentsAsString(StringCallback::create(context, callback));
}

#ifdef __BLOCKS__
static void callContentsAsStringBlockBlockAndDispose(WKStringRef resultValue, WKErrorRef error, void* context)
{
    WKPageGetContentsAsStringBlock block = (WKPageGetContentsAsStringBlock)context;
    block(resultValue, error);
    Block_release(block);
}

void WKPageGetContentsAsString_b(WKPageRef pageRef, WKPageGetSourceForFrameBlock block)
{
    WKPageGetContentsAsString(pageRef, Block_copy(block), callContentsAsStringBlockBlockAndDispose);
}
#endif
