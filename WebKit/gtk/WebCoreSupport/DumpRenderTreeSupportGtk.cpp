/*
 *  Copyright (C) Research In Motion Limited 2010. All rights reserved.
 *  Copyright (C) 2010 Joone Hur <joone@kldp.org>
 *  Copyright (C) 2009 Google Inc. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"
#include "DumpRenderTreeSupportGtk.h"

#include "AccessibilityObjectWrapperAtk.h"
#include "AnimationController.h"
#include "APICast.h"
#include "AXObjectCache.h"
#include "Document.h"
#include "FocusController.h"
#include "FrameLoaderClientGtk.h"
#include "FrameView.h"
#include "FrameTree.h"
#include "GCController.h"
#include "GraphicsContext.h"
#include "JSDOMWindow.h"
#include "JSDocument.h"
#include "JSElement.h"
#include "JSLock.h"
#include "JSNodeList.h"
#include "JSValue.h"
#include "NodeList.h"
#include "PageGroup.h"
#include "PlatformString.h"
#include "PrintContext.h"
#include "RenderListItem.h"
#include "RenderView.h"
#include "RenderTreeAsText.h"
#include "SecurityOrigin.h"
#if ENABLE(SVG)
#include "SVGSMILElement.h"
#endif
#include "WorkerThread.h"
#include "webkitprivate.h"
#include "webkitwebframeprivate.h"
#include "webkitwebviewprivate.h"
#include "webkitwebview.h"
#include "webkitwebframe.h"
#include "DOMWrapperWorld.h"
#include <JavaScriptCore/APICast.h>

using namespace JSC;
using namespace WebCore;
using namespace WebKit;

bool DumpRenderTreeSupportGtk::s_drtRun = false;
bool DumpRenderTreeSupportGtk::s_linksIncludedInTabChain = true;

DumpRenderTreeSupportGtk::DumpRenderTreeSupportGtk()
{
}

DumpRenderTreeSupportGtk::~DumpRenderTreeSupportGtk()
{
}

void DumpRenderTreeSupportGtk::setDumpRenderTreeModeEnabled(bool enabled)
{
    s_drtRun = enabled;
}

bool DumpRenderTreeSupportGtk::dumpRenderTreeModeEnabled()
{
    return s_drtRun;
}
void DumpRenderTreeSupportGtk::setLinksIncludedInFocusChain(bool enabled)
{
    s_linksIncludedInTabChain = enabled;
}

bool DumpRenderTreeSupportGtk::linksIncludedInFocusChain()
{
    return s_linksIncludedInTabChain;
}

JSValueRef DumpRenderTreeSupportGtk::nodesFromRect(JSContextRef context, JSValueRef value, int x, int y, unsigned top, unsigned right, unsigned bottom, unsigned left, bool ignoreClipping)
{
    JSLock lock(SilenceAssertionsOnly);
    ExecState* exec = toJS(context);
    if (!value)
        return JSValueMakeUndefined(context);
    JSValue jsValue = toJS(exec, value);
    if (!jsValue.inherits(&JSDocument::s_info))
       return JSValueMakeUndefined(context);

    JSDocument* jsDocument = static_cast<JSDocument*>(asObject(jsValue));
    Document* document = jsDocument->impl();
    RefPtr<NodeList> nodes = document->nodesFromRect(x, y, top, right, bottom, left, ignoreClipping);
    return toRef(exec, toJS(exec, jsDocument->globalObject(), nodes.get()));
}

/**
 * getFrameChildren:
 * @frame: a #WebKitWebFrame
 *
 * Return value: child frames of @frame
 */
GSList* DumpRenderTreeSupportGtk::getFrameChildren(WebKitWebFrame* frame)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_FRAME(frame), 0);

    Frame* coreFrame = core(frame);
    if (!coreFrame)
        return 0;

    GSList* children = 0;
    for (Frame* child = coreFrame->tree()->firstChild(); child; child = child->tree()->nextSibling()) {
        FrameLoader* loader = child->loader();
        WebKit::FrameLoaderClient* client = static_cast<WebKit::FrameLoaderClient*>(loader->client());
        if (client)
          children = g_slist_append(children, client->webFrame());
    }

    return children;
}

/**
 * getInnerText:
 * @frame: a #WebKitWebFrame
 *
 * Return value: inner text of @frame
 */
CString DumpRenderTreeSupportGtk::getInnerText(WebKitWebFrame* frame)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_FRAME(frame), CString(""));

    Frame* coreFrame = core(frame);
    if (!coreFrame)
        return CString("");

    FrameView* view = coreFrame->view();

    if (view && view->layoutPending())
        view->layout();

    Element* documentElement = coreFrame->document()->documentElement();
    return documentElement->innerText().utf8();
}

/**
 * dumpRenderTree:
 * @frame: a #WebKitWebFrame
 *
 * Return value: Non-recursive render tree dump of @frame
 */
CString DumpRenderTreeSupportGtk::dumpRenderTree(WebKitWebFrame* frame)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_FRAME(frame), CString(""));

    Frame* coreFrame = core(frame);
    if (!coreFrame)
        return CString("");

    FrameView* view = coreFrame->view();

    if (view && view->layoutPending())
        view->layout();

    return externalRepresentation(coreFrame).utf8();
}

/**
 * counterValueForElementById:
 * @frame: a #WebKitWebFrame
 * @id: an element ID string
 *
 * Return value: The counter value of element @id in @frame
 */
CString DumpRenderTreeSupportGtk::counterValueForElementById(WebKitWebFrame* frame, const char* id)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_FRAME(frame), CString());

    Frame* coreFrame = core(frame);
    if (!coreFrame)
        return CString();

    Element* coreElement = coreFrame->document()->getElementById(AtomicString(id));
    if (!coreElement)
        return CString();

    return counterValueForElement(coreElement).utf8();
}

/**
 * numberForElementById
 * @frame: a #WebKitWebFrame
 * @id: an element ID string
 * @pageWidth: width of a page
 * @pageHeight: height of a page
 *
 * Return value: The number of page where the specified element will be put
 */
int DumpRenderTreeSupportGtk::pageNumberForElementById(WebKitWebFrame* frame, const char* id, float pageWidth, float pageHeight)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_FRAME(frame), 0);

    Frame* coreFrame = core(frame);
    if (!coreFrame)
        return -1;

    Element* coreElement = coreFrame->document()->getElementById(AtomicString(id));
    if (!coreElement)
        return -1;
    return PrintContext::pageNumberForElement(coreElement, FloatSize(pageWidth, pageHeight));
}

/**
 * numberOfPagesForFrame
 * @frame: a #WebKitWebFrame
 * @pageWidth: width of a page
 * @pageHeight: height of a page
 *
 * Return value: The number of pages to be printed.
 */
int DumpRenderTreeSupportGtk::numberOfPagesForFrame(WebKitWebFrame* frame, float pageWidth, float pageHeight)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_FRAME(frame), 0);

    Frame* coreFrame = core(frame);
    if (!coreFrame)
        return -1;

    return PrintContext::numberOfPages(coreFrame, FloatSize(pageWidth, pageHeight));
}

/**
 * pageProperty
 * @frame: a #WebKitWebFrame
 * @propertyName: name of a property
 * @pageNumber: number of a page 
 *
 * Return value: The value of the given property name.
 */
CString DumpRenderTreeSupportGtk::pageProperty(WebKitWebFrame* frame, const char* propertyName, int pageNumber)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_FRAME(frame), CString());

    Frame* coreFrame = core(frame);
    if (!coreFrame)
        return CString();

    return PrintContext::pageProperty(coreFrame, propertyName, pageNumber).utf8();
}

/**
 * isPageBoxVisible
 * @frame: a #WebKitWebFrame
 * @pageNumber: number of a page 
 *
 * Return value: TRUE if a page box is visible. 
 */
bool DumpRenderTreeSupportGtk::isPageBoxVisible(WebKitWebFrame* frame, int pageNumber)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_FRAME(frame), false);

    Frame* coreFrame = core(frame);
    if (!coreFrame)
        return false;

    return coreFrame->document()->isPageBoxVisible(pageNumber); 
}

/**
 * pageSizeAndMarginsInPixels
 * @frame: a #WebKitWebFrame
 * @pageNumber: number of a page 
 * @width: width of a page
 * @height: height of a page
 * @marginTop: top margin of a page
 * @marginRight: right margin of a page
 * @marginBottom: bottom margin of a page
 * @marginLeft: left margin of a page
 *
 * Return value: The value of page size and margin.
 */
CString DumpRenderTreeSupportGtk::pageSizeAndMarginsInPixels(WebKitWebFrame* frame, int pageNumber, int width, int height, int marginTop, int marginRight, int marginBottom, int marginLeft)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_FRAME(frame), CString());

    Frame* coreFrame = core(frame);
    if (!coreFrame)
        return CString();

    return PrintContext::pageSizeAndMarginsInPixels(coreFrame, pageNumber, width, height, marginTop, marginRight, marginBottom, marginLeft).utf8();
}

/**
 * addUserStyleSheet
 * @frame: a #WebKitWebFrame
 * @sourceCode: code of a user stylesheet
 *
 */
void DumpRenderTreeSupportGtk::addUserStyleSheet(WebKitWebFrame* frame, const char* sourceCode, bool allFrames)
{
    g_return_if_fail(WEBKIT_IS_WEB_FRAME(frame));

    Frame* coreFrame = core(frame);
    if (!coreFrame)
        return;

    WebKitWebView* webView = getViewFromFrame(frame);
    Page* page = core(webView);
    page->group().addUserStyleSheetToWorld(mainThreadNormalWorld(), sourceCode, KURL(), 0, 0, allFrames ? InjectInAllFrames : InjectInTopFrameOnly); 
}

/**
 * getPendingUnloadEventCount:
 * @frame: a #WebKitWebFrame
 *
 * Return value: number of pending unload events
 */
guint DumpRenderTreeSupportGtk::getPendingUnloadEventCount(WebKitWebFrame* frame)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_FRAME(frame), 0);

    return core(frame)->domWindow()->pendingUnloadEventListeners();
}

bool DumpRenderTreeSupportGtk::pauseAnimation(WebKitWebFrame* frame, const char* name, double time, const char* element)
{
    ASSERT(core(frame));
    Element* coreElement = core(frame)->document()->getElementById(AtomicString(element));
    if (!coreElement || !coreElement->renderer())
        return false;
    return core(frame)->animation()->pauseAnimationAtTime(coreElement->renderer(), AtomicString(name), time);
}

bool DumpRenderTreeSupportGtk::pauseTransition(WebKitWebFrame* frame, const char* name, double time, const char* element)
{
    ASSERT(core(frame));
    Element* coreElement = core(frame)->document()->getElementById(AtomicString(element));
    if (!coreElement || !coreElement->renderer())
        return false;
    return core(frame)->animation()->pauseTransitionAtTime(coreElement->renderer(), AtomicString(name), time);
}

bool DumpRenderTreeSupportGtk::pauseSVGAnimation(WebKitWebFrame* frame, const char* animationId, double time, const char* elementId)
{
    ASSERT(core(frame));
#if ENABLE(SVG)
    Document* document = core(frame)->document();
    if (!document || !document->svgExtensions())
        return false;
    Element* coreElement = document->getElementById(AtomicString(animationId));
    if (!coreElement || !SVGSMILElement::isSMILElement(coreElement))
        return false;
    return document->accessSVGExtensions()->sampleAnimationAtTime(elementId, static_cast<SVGSMILElement*>(coreElement), time);
#else
    return false;
#endif
}

CString DumpRenderTreeSupportGtk::markerTextForListItem(WebKitWebFrame* frame, JSContextRef context, JSValueRef nodeObject)
{
    JSC::ExecState* exec = toJS(context);
    Element* element = toElement(toJS(exec, nodeObject));
    if (!element)
        return CString();

    return WebCore::markerTextForListItem(element).utf8();
}

unsigned int DumpRenderTreeSupportGtk::numberOfActiveAnimations(WebKitWebFrame* frame)
{
    Frame* coreFrame = core(frame);
    if (!coreFrame)
        return 0;

    return coreFrame->animation()->numberOfActiveAnimations();
}

void DumpRenderTreeSupportGtk::suspendAnimations(WebKitWebFrame* frame)
{
    Frame* coreFrame = core(frame);
    if (!coreFrame)
        return;

    return coreFrame->animation()->suspendAnimations();
}

void DumpRenderTreeSupportGtk::resumeAnimations(WebKitWebFrame* frame)
{
    Frame* coreFrame = core(frame);
    if (!coreFrame)
        return;

    return coreFrame->animation()->resumeAnimations();
}

void DumpRenderTreeSupportGtk::clearMainFrameName(WebKitWebFrame* frame)
{
    g_return_if_fail(WEBKIT_IS_WEB_FRAME(frame));

    core(frame)->tree()->clearName();
}

AtkObject* DumpRenderTreeSupportGtk::getFocusedAccessibleElement(WebKitWebFrame* frame)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_FRAME(frame), 0);

#if HAVE(ACCESSIBILITY)
    if (!AXObjectCache::accessibilityEnabled())
        AXObjectCache::enableAccessibility();

    WebKitWebFramePrivate* priv = frame->priv;
    if (!priv->coreFrame || !priv->coreFrame->document())
        return 0;

    RenderView* root = toRenderView(priv->coreFrame->document()->renderer());
    if (!root)
        return 0;

    AtkObject* wrapper =  priv->coreFrame->document()->axObjectCache()->getOrCreate(root)->wrapper();
    if (!wrapper)
        return 0;

    return webkit_accessible_get_focused_element(WEBKIT_ACCESSIBLE(wrapper));
#else
    return 0;
#endif
}

void DumpRenderTreeSupportGtk::executeCoreCommandByName(WebKitWebView* webView, const gchar* name, const gchar* value)
{
    g_return_if_fail(WEBKIT_IS_WEB_VIEW(webView));
    g_return_if_fail(name);
    g_return_if_fail(value);

    core(webView)->focusController()->focusedOrMainFrame()->editor()->command(name).execute(value);
}

bool DumpRenderTreeSupportGtk::isCommandEnabled(WebKitWebView* webView, const gchar* name)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_VIEW(webView), FALSE);
    g_return_val_if_fail(name, FALSE);

    return core(webView)->focusController()->focusedOrMainFrame()->editor()->command(name).isEnabled();
}

void DumpRenderTreeSupportGtk::whiteListAccessFromOrigin(const gchar* sourceOrigin, const gchar* destinationProtocol, const gchar* destinationHost, bool allowDestinationSubdomains)
{
    SecurityOrigin::addOriginAccessWhitelistEntry(*SecurityOrigin::createFromString(sourceOrigin), destinationProtocol, destinationHost, allowDestinationSubdomains);
}

void DumpRenderTreeSupportGtk::resetOriginAccessWhiteLists()
{
    SecurityOrigin::resetOriginAccessWhitelists();
}

void DumpRenderTreeSupportGtk::gcCollectJavascriptObjects()
{
    gcController().garbageCollectNow();
}

void DumpRenderTreeSupportGtk::gcCollectJavascriptObjectsOnAlternateThread(bool waitUntilDone)
{
    gcController().garbageCollectOnAlternateThreadForDebugging(waitUntilDone);
}

unsigned long DumpRenderTreeSupportGtk::gcCountJavascriptObjects()
{
    JSC::JSLock lock(JSC::SilenceAssertionsOnly);
    return JSDOMWindow::commonJSGlobalData()->heap.objectCount();
}

void DumpRenderTreeSupportGtk::layoutFrame(WebKitWebFrame* frame)
{
    Frame* coreFrame = core(frame);
    if (!coreFrame)
        return;

    FrameView* view = coreFrame->view();
    if (!view)
        return;

    view->layout();
}

// For testing fast/viewport.
void DumpRenderTreeSupportGtk::dumpConfigurationForViewport(WebKitWebView* webView, gint availableWidth, gint availableHeight)
{
    g_return_if_fail(WEBKIT_IS_WEB_VIEW(webView));

    ViewportArguments arguments = webView->priv->corePage->mainFrame()->document()->viewportArguments();
    // desktopWidth = 980, deviceWidth = 320, deviceHeight = 480, deviceDPI = 160
    ViewportAttributes attrs = computeViewportAttributes(arguments, 980, 320, 480, 160, IntSize(availableWidth, availableHeight));

    fprintf(stdout, "viewport size %dx%d scale %f with limits [%f, %f]\n", attrs.layoutSize.width(), attrs.layoutSize.height(), attrs.initialScale, attrs.minimumScale, attrs.maximumScale);
}

unsigned int DumpRenderTreeSupportGtk::workerThreadCount()
{
#if ENABLE(WORKERS)
    return WebCore::WorkerThread::workerThreadCount();
#else
    return 0;
#endif
}

bool DumpRenderTreeSupportGtk::webkitWebFrameSelectionHasSpellingMarker(WebKitWebFrame *frame, gint from, gint length)
{
    g_return_val_if_fail(WEBKIT_IS_WEB_FRAME(frame), FALSE);

    return core(frame)->editor()->selectionStartHasSpellingMarkerFor(from, length);
}
