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

#include "WebUIClient.h"

#include "ImmutableDictionary.h"
#include "NativeWebKeyboardEvent.h"
#include "WKAPICast.h"
#include "WebNumber.h"
#include "WebOpenPanelResultListenerProxy.h"
#include "WebPageProxy.h"
#include <WebCore/FloatRect.h>
#include <WebCore/IntSize.h>
#include <WebCore/WindowFeatures.h>
#include <string.h>
#include <wtf/text/WTFString.h>

using namespace WebCore;

namespace WebKit {

PassRefPtr<WebPageProxy> WebUIClient::createNewPage(WebPageProxy* page, const WindowFeatures& windowFeatures, WebEvent::Modifiers modifiers, WebMouseEvent::Button button)
{
    if (!m_client.createNewPage)
        return 0;

    ImmutableDictionary::MapType map;
    if (windowFeatures.xSet)
        map.set("x", WebDouble::create(windowFeatures.x));
    if (windowFeatures.ySet)
        map.set("y", WebDouble::create(windowFeatures.y));
    if (windowFeatures.widthSet)
        map.set("width", WebDouble::create(windowFeatures.width));
    if (windowFeatures.heightSet)
        map.set("height", WebDouble::create(windowFeatures.height));
    map.set("menuBarVisible", WebBoolean::create(windowFeatures.menuBarVisible));
    map.set("statusBarVisible", WebBoolean::create(windowFeatures.statusBarVisible));
    map.set("toolBarVisible", WebBoolean::create(windowFeatures.toolBarVisible));
    map.set("scrollbarsVisible", WebBoolean::create(windowFeatures.scrollbarsVisible));
    map.set("resizable", WebBoolean::create(windowFeatures.resizable));
    map.set("fullscreen", WebBoolean::create(windowFeatures.fullscreen));
    map.set("dialog", WebBoolean::create(windowFeatures.dialog));
    RefPtr<ImmutableDictionary> featuresMap = ImmutableDictionary::adopt(map);

    return adoptRef(toImpl(m_client.createNewPage(toAPI(page), toAPI(featuresMap.get()), toAPI(modifiers), toAPI(button), m_client.clientInfo)));
} 

void WebUIClient::showPage(WebPageProxy* page)
{
    if (!m_client.showPage)
        return;
    
    m_client.showPage(toAPI(page), m_client.clientInfo);
}

void WebUIClient::close(WebPageProxy* page)
{
    if (!m_client.close)
        return;
    
    m_client.close(toAPI(page), m_client.clientInfo);
}

void WebUIClient::runJavaScriptAlert(WebPageProxy* page, const String& message, WebFrameProxy* frame)
{
    if (!m_client.runJavaScriptAlert)
        return;
    
    m_client.runJavaScriptAlert(toAPI(page), toAPI(message.impl()), toAPI(frame), m_client.clientInfo);
}

bool WebUIClient::runJavaScriptConfirm(WebPageProxy* page, const String& message, WebFrameProxy* frame)
{
    if (!m_client.runJavaScriptConfirm)
        return false;

    return m_client.runJavaScriptConfirm(toAPI(page), toAPI(message.impl()), toAPI(frame), m_client.clientInfo);
}

String WebUIClient::runJavaScriptPrompt(WebPageProxy* page, const String& message, const String& defaultValue, WebFrameProxy* frame)
{
    if (!m_client.runJavaScriptPrompt)
        return String();

    WebString* string = toImpl(m_client.runJavaScriptPrompt(toAPI(page), toAPI(message.impl()), toAPI(defaultValue.impl()), toAPI(frame), m_client.clientInfo));
    if (!string)
        return String();

    String result = string->string();
    string->deref();

    return result;
}

void WebUIClient::setStatusText(WebPageProxy* page, const String& text)
{
    if (!m_client.setStatusText)
        return;

    m_client.setStatusText(toAPI(page), toAPI(text.impl()), m_client.clientInfo);
}

void WebUIClient::mouseDidMoveOverElement(WebPageProxy* page, WebEvent::Modifiers modifiers, APIObject* userData)
{
    if (!m_client.mouseDidMoveOverElement)
        return;

    m_client.mouseDidMoveOverElement(toAPI(page), toAPI(modifiers), toAPI(userData), m_client.clientInfo);
}

void WebUIClient::missingPluginButtonClicked(WebPageProxy* page, const String& mimeType, const String& url)
{
    if (!m_client.missingPluginButtonClicked)
        return;

    m_client.missingPluginButtonClicked(toAPI(page), toAPI(mimeType.impl()), toAPI(url.impl()), m_client.clientInfo);
}

void WebUIClient::didNotHandleKeyEvent(WebPageProxy* page, const NativeWebKeyboardEvent& event)
{
    if (!m_client.didNotHandleKeyEvent)
        return;
    m_client.didNotHandleKeyEvent(toAPI(page), event.nativeEvent(), m_client.clientInfo);
}

bool WebUIClient::toolbarsAreVisible(WebPageProxy* page)
{
    if (!m_client.toolbarsAreVisible)
        return true;
    return m_client.toolbarsAreVisible(toAPI(page), m_client.clientInfo);

}
void WebUIClient::setToolbarsAreVisible(WebPageProxy* page, bool visible)
{
    if (!m_client.setToolbarsAreVisible)
        return;
    m_client.setToolbarsAreVisible(toAPI(page), visible, m_client.clientInfo);
}

bool WebUIClient::menuBarIsVisible(WebPageProxy* page)
{
    if (!m_client.menuBarIsVisible)
        return true;
    return m_client.menuBarIsVisible(toAPI(page), m_client.clientInfo);
}

void WebUIClient::setMenuBarIsVisible(WebPageProxy* page, bool visible)
{
    if (!m_client.setMenuBarIsVisible)
        return;
    m_client.setMenuBarIsVisible(toAPI(page), visible, m_client.clientInfo);
}

bool WebUIClient::statusBarIsVisible(WebPageProxy* page)
{
    if (!m_client.statusBarIsVisible)
        return true;
    return m_client.statusBarIsVisible(toAPI(page), m_client.clientInfo);
}

void WebUIClient::setStatusBarIsVisible(WebPageProxy* page, bool visible)
{
    if (!m_client.setStatusBarIsVisible)
        return;
    m_client.setStatusBarIsVisible(toAPI(page), visible, m_client.clientInfo);
}

bool WebUIClient::isResizable(WebPageProxy* page)
{
    if (!m_client.isResizable)
        return true;
    return m_client.isResizable(toAPI(page), m_client.clientInfo);
}

void WebUIClient::setIsResizable(WebPageProxy* page, bool resizable)
{
    if (!m_client.setIsResizable)
        return;
    m_client.setIsResizable(toAPI(page), resizable, m_client.clientInfo);
}

void WebUIClient::setWindowFrame(WebPageProxy* page, const FloatRect& frame)
{
    if (!m_client.setWindowFrame)
        return;

    m_client.setWindowFrame(toAPI(page), toAPI(frame), m_client.clientInfo);
}

FloatRect WebUIClient::windowFrame(WebPageProxy* page)
{
    if (!m_client.getWindowFrame)
        return FloatRect();

    return toFloatRect(m_client.getWindowFrame(toAPI(page), m_client.clientInfo));
}

bool WebUIClient::canRunBeforeUnloadConfirmPanel()
{
    return m_client.runBeforeUnloadConfirmPanel;
}

bool WebUIClient::runBeforeUnloadConfirmPanel(WebPageProxy* page, const String& message, WebFrameProxy* frame)
{
    if (!m_client.runBeforeUnloadConfirmPanel)
        return true;

    return m_client.runBeforeUnloadConfirmPanel(toAPI(page), toAPI(message.impl()), toAPI(frame), m_client.clientInfo);
}

void WebUIClient::didDraw(WebPageProxy* page)
{
    if (!m_client.didDraw)
        return;

    m_client.didDraw(toAPI(page), m_client.clientInfo);
}

void WebUIClient::pageDidScroll(WebPageProxy* page)
{
    if (!m_client.pageDidScroll)
        return;

    m_client.pageDidScroll(toAPI(page), m_client.clientInfo);
}

unsigned long long WebUIClient::exceededDatabaseQuota(WebPageProxy* page, WebFrameProxy* frame, WebSecurityOrigin* origin, const String& databaseName, const String& databaseDisplayName, unsigned long long currentQuota, unsigned long long currentUsage, unsigned long long expectedUsage)
{
    if (!m_client.exceededDatabaseQuota)
        return currentQuota;

    return m_client.exceededDatabaseQuota(toAPI(page), toAPI(frame), toAPI(origin), toAPI(databaseName.impl()), toAPI(databaseDisplayName.impl()), currentQuota, currentUsage, expectedUsage, m_client.clientInfo);
}

bool WebUIClient::runOpenPanel(WebPageProxy* page, WebFrameProxy* frame, const WebOpenPanelParameters::Data& parameterData, WebOpenPanelResultListenerProxy* listener)
{
    if (!m_client.runOpenPanel)
        return false;

    RefPtr<WebOpenPanelParameters> parameters = WebOpenPanelParameters::create(parameterData);
    m_client.runOpenPanel(toAPI(page), toAPI(frame), toAPI(parameters.get()), toAPI(listener), m_client.clientInfo);
    return true;
}

} // namespace WebKit
