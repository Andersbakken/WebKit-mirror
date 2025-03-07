/*
 *  Copyright (C) Research In Motion Limited 2010. All rights reserved.
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

#ifndef DumpRenderTreeSupportGtk_h
#define DumpRenderTreeSupportGtk_h


#include "JSStringRef.h"
#include <webkit/webkitdefines.h>

#include <atk/atk.h>
#include <glib.h>
#include <webkit/webkitdefines.h>
#include <webkit/webkitwebframe.h>
#include <wtf/text/CString.h>

class DumpRenderTreeSupportGtk {

public:
    DumpRenderTreeSupportGtk();
    ~DumpRenderTreeSupportGtk();

    static void setDumpRenderTreeModeEnabled(bool);
    static bool dumpRenderTreeModeEnabled();

    static void setLinksIncludedInFocusChain(bool);
    static bool linksIncludedInFocusChain();
    static JSValueRef nodesFromRect(JSContextRef context, JSValueRef value, int x, int y, unsigned top, unsigned right, unsigned bottom, unsigned left, bool ignoreClipping);
    static void dumpConfigurationForViewport(WebKitWebView* webView, gint availableWidth, gint availableHeight);

    // FIXME: Move these to webkitwebframe.h once their API has been discussed.
    static GSList* getFrameChildren(WebKitWebFrame*);
    static WTF::CString getInnerText(WebKitWebFrame*);
    static WTF::CString dumpRenderTree(WebKitWebFrame*);
    static WTF::CString counterValueForElementById(WebKitWebFrame*, const char* id);
    static int pageNumberForElementById(WebKitWebFrame*, const char* id, float pageWidth, float pageHeight);
    static int numberOfPagesForFrame(WebKitWebFrame*, float pageWidth, float pageHeight);
    static WTF::CString pageProperty(WebKitWebFrame*, const char* propertyName, int pageNumber);
    static bool isPageBoxVisible(WebKitWebFrame*, int pageNumber);
    static WTF::CString pageSizeAndMarginsInPixels(WebKitWebFrame*, int pageNumber, int width, int height, int marginTop, int marginRight, int marginBottom, int marginLeft);
    static void addUserStyleSheet(WebKitWebFrame*, const char* sourceCode, bool allFrames);
    static guint getPendingUnloadEventCount(WebKitWebFrame*);
    static bool pauseAnimation(WebKitWebFrame*, const char* name, double time, const char* element);
    static bool pauseTransition(WebKitWebFrame*, const char* name, double time, const char* element);
    static bool pauseSVGAnimation(WebKitWebFrame*, const char* animationId, double time, const char* elementId);
    static WTF::CString markerTextForListItem(WebKitWebFrame*, JSContextRef, JSValueRef nodeObject);
    static unsigned int numberOfActiveAnimations(WebKitWebFrame*);
    static void suspendAnimations(WebKitWebFrame*);
    static void resumeAnimations(WebKitWebFrame*);
    static void clearMainFrameName(WebKitWebFrame*);
    static AtkObject* getFocusedAccessibleElement(WebKitWebFrame*);
    static void layoutFrame(WebKitWebFrame*);

    // WebKitWebView
    static void executeCoreCommandByName(WebKitWebView*, const gchar* name, const gchar* value);
    static bool isCommandEnabled(WebKitWebView*, const gchar* name);

    // GC
    static void gcCollectJavascriptObjects();
    static void gcCollectJavascriptObjectsOnAlternateThread(bool waitUntilDone);
    static unsigned long gcCountJavascriptObjects();

    static void whiteListAccessFromOrigin(const gchar* sourceOrigin, const gchar* destinationProtocol, const gchar* destinationHost, bool allowDestinationSubdomains);
    static void resetOriginAccessWhiteLists();
    static unsigned int workerThreadCount();
    static bool webkitWebFrameSelectionHasSpellingMarker(WebKitWebFrame*, gint from, gint length);

private:
    static bool s_drtRun;
    static bool s_linksIncludedInTabChain;
};

#endif
