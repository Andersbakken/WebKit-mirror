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

#if ENABLE(PLUGIN_PROCESS)

#include "PluginControllerProxy.h"

#include "BackingStore.h"
#include "DataReference.h"
#include "NPObjectProxy.h"
#include "NPRemoteObjectMap.h"
#include "NPRuntimeUtilities.h"
#include "NPVariantData.h"
#include "NetscapePlugin.h"
#include "PluginProcess.h"
#include "PluginProxyMessages.h"
#include "WebCoreArgumentCoders.h"
#include "WebProcessConnection.h"
#include <WebCore/GraphicsContext.h>
#include <wtf/text/WTFString.h>

using namespace WebCore;

namespace WebKit {

PassOwnPtr<PluginControllerProxy> PluginControllerProxy::create(WebProcessConnection* connection, uint64_t pluginInstanceID, const String& userAgent, bool isPrivateBrowsingEnabled, bool isAcceleratedCompositingEnabled)
{
    return adoptPtr(new PluginControllerProxy(connection, pluginInstanceID, userAgent, isPrivateBrowsingEnabled, isAcceleratedCompositingEnabled));
}

PluginControllerProxy::PluginControllerProxy(WebProcessConnection* connection, uint64_t pluginInstanceID, const String& userAgent, bool isPrivateBrowsingEnabled, bool isAcceleratedCompositingEnabled)
    : m_connection(connection)
    , m_pluginInstanceID(pluginInstanceID)
    , m_userAgent(userAgent)
    , m_isPrivateBrowsingEnabled(isPrivateBrowsingEnabled)
    , m_isAcceleratedCompositingEnabled(isAcceleratedCompositingEnabled)
    , m_paintTimer(RunLoop::main(), this, &PluginControllerProxy::paint)
    , m_waitingForDidUpdate(false)
    , m_pluginCanceledManualStreamLoad(false)
#if PLATFORM(MAC)
    , m_isComplexTextInputEnabled(false)
#endif
{
}

PluginControllerProxy::~PluginControllerProxy()
{
    ASSERT(!m_plugin);
}

bool PluginControllerProxy::initialize(const Plugin::Parameters& parameters)
{
    ASSERT(!m_plugin);

    m_plugin = NetscapePlugin::create(PluginProcess::shared().netscapePluginModule());
    if (!m_plugin)
        return false;

    if (!m_plugin->initialize(this, parameters)) {
        m_plugin = 0;
        return false;
    }

    platformInitialize();

    return true;
}

void PluginControllerProxy::destroy()
{
    ASSERT(m_plugin);

    m_plugin->destroy();
    m_plugin = 0;

    platformDestroy();
}

void PluginControllerProxy::paint()
{
    ASSERT(!m_dirtyRect.isEmpty());
    m_paintTimer.stop();

    if (!m_backingStore)
        return;

    IntRect dirtyRect = m_dirtyRect;
    m_dirtyRect = IntRect();

    // Create a graphics context.
    OwnPtr<GraphicsContext> graphicsContext = m_backingStore->createGraphicsContext();
    graphicsContext->translate(-m_frameRect.x(), -m_frameRect.y());

    ASSERT(m_plugin);
    m_plugin->paint(graphicsContext.get(), dirtyRect);

    m_connection->connection()->send(Messages::PluginProxy::Update(dirtyRect), m_pluginInstanceID);
}

void PluginControllerProxy::startPaintTimer()
{
    // Check if we should start the timer.
    
    if (m_dirtyRect.isEmpty())
        return;

    // FIXME: Check clip rect.
    
    if (m_paintTimer.isActive())
        return;

    if (m_waitingForDidUpdate)
        return;

    // Start the timer.
    m_paintTimer.startOneShot(0);

    m_waitingForDidUpdate = true;
}

void PluginControllerProxy::invalidate(const IntRect& rect)
{
    // Convert the dirty rect to window coordinates.
    IntRect dirtyRect = rect;
    dirtyRect.move(m_frameRect.x(), m_frameRect.y());

    // Make sure that the dirty rect is not greater than the plug-in itself.
    dirtyRect.intersect(m_frameRect);

    m_dirtyRect.unite(dirtyRect);

    startPaintTimer();
}

String PluginControllerProxy::userAgent()
{
    return m_userAgent;
}

void PluginControllerProxy::loadURL(uint64_t requestID, const String& method, const String& urlString, const String& target, const HTTPHeaderMap& headerFields, const Vector<uint8_t>& httpBody, bool allowPopups)
{
    m_connection->connection()->send(Messages::PluginProxy::LoadURL(requestID, method, urlString, target, headerFields, httpBody, allowPopups), m_pluginInstanceID);
}

void PluginControllerProxy::cancelStreamLoad(uint64_t streamID)
{
    m_connection->connection()->send(Messages::PluginProxy::CancelStreamLoad(streamID), m_pluginInstanceID);
}

void PluginControllerProxy::cancelManualStreamLoad()
{
    m_pluginCanceledManualStreamLoad = true;

    m_connection->connection()->send(Messages::PluginProxy::CancelManualStreamLoad(), m_pluginInstanceID);
}

NPObject* PluginControllerProxy::windowScriptNPObject()
{
    uint64_t windowScriptNPObjectID = 0;

    if (!m_connection->connection()->sendSync(Messages::PluginProxy::GetWindowScriptNPObject(), Messages::PluginProxy::GetWindowScriptNPObject::Reply(windowScriptNPObjectID), m_pluginInstanceID))
        return 0;

    if (!windowScriptNPObjectID)
        return 0;

    return m_connection->npRemoteObjectMap()->createNPObjectProxy(windowScriptNPObjectID);
}

NPObject* PluginControllerProxy::pluginElementNPObject()
{
    uint64_t pluginElementNPObjectID = 0;

    if (!m_connection->connection()->sendSync(Messages::PluginProxy::GetPluginElementNPObject(), Messages::PluginProxy::GetPluginElementNPObject::Reply(pluginElementNPObjectID), m_pluginInstanceID))
        return 0;

    if (!pluginElementNPObjectID)
        return 0;

    return m_connection->npRemoteObjectMap()->createNPObjectProxy(pluginElementNPObjectID);
}

bool PluginControllerProxy::evaluate(NPObject* npObject, const String& scriptString, NPVariant* result, bool allowPopups)
{
    NPVariant npObjectAsNPVariant;
    OBJECT_TO_NPVARIANT(npObject, npObjectAsNPVariant);

    // Send the NPObject over as an NPVariantData.
    NPVariantData npObjectAsNPVariantData = m_connection->npRemoteObjectMap()->npVariantToNPVariantData(npObjectAsNPVariant);

    bool returnValue = false;
    NPVariantData resultData;

    if (!m_connection->connection()->sendSync(Messages::PluginProxy::Evaluate(npObjectAsNPVariantData, scriptString, allowPopups), Messages::PluginProxy::Evaluate::Reply(returnValue, resultData), m_pluginInstanceID))
        return false;

    if (!returnValue)
        return false;

    *result = m_connection->npRemoteObjectMap()->npVariantDataToNPVariant(resultData);
    return true;
}

void PluginControllerProxy::setStatusbarText(const String& statusbarText)
{
    m_connection->connection()->send(Messages::PluginProxy::SetStatusbarText(statusbarText), m_pluginInstanceID);
}

bool PluginControllerProxy::isAcceleratedCompositingEnabled()
{
    return m_isAcceleratedCompositingEnabled;
}

void PluginControllerProxy::pluginProcessCrashed()
{
    // This should never be called from here.
    ASSERT_NOT_REACHED();
}

void PluginControllerProxy::setComplexTextInputEnabled(bool complexTextInputEnabled)
{
    if (m_isComplexTextInputEnabled == complexTextInputEnabled)
        return;

    m_isComplexTextInputEnabled = complexTextInputEnabled;

    m_connection->connection()->send(Messages::PluginProxy::SetComplexTextInputEnabled(complexTextInputEnabled), m_pluginInstanceID);
}

String PluginControllerProxy::proxiesForURL(const String& urlString)
{
    String proxyString;
    
    if (!m_connection->connection()->sendSync(Messages::PluginProxy::CookiesForURL(urlString), Messages::PluginProxy::CookiesForURL::Reply(proxyString), m_pluginInstanceID))
        return String();
    
    return proxyString;
}

String PluginControllerProxy::cookiesForURL(const String& urlString)
{
    String cookieString;

    if (!m_connection->connection()->sendSync(Messages::PluginProxy::CookiesForURL(urlString), Messages::PluginProxy::CookiesForURL::Reply(cookieString), m_pluginInstanceID))
        return String();

    return cookieString;
}

void PluginControllerProxy::setCookiesForURL(const String& urlString, const String& cookieString)
{
    m_connection->connection()->send(Messages::PluginProxy::SetCookiesForURL(urlString, cookieString), m_pluginInstanceID);
}

bool PluginControllerProxy::isPrivateBrowsingEnabled()
{
    return m_isPrivateBrowsingEnabled;
}

void PluginControllerProxy::frameDidFinishLoading(uint64_t requestID)
{
    m_plugin->frameDidFinishLoading(requestID);
}

void PluginControllerProxy::frameDidFail(uint64_t requestID, bool wasCancelled)
{
    m_plugin->frameDidFail(requestID, wasCancelled);
}

void PluginControllerProxy::geometryDidChange(const IntRect& frameRect, const IntRect& clipRect, const SharedMemory::Handle& backingStoreHandle)
{
    m_frameRect = frameRect;
    m_clipRect = clipRect;

    ASSERT(m_plugin);

    if (!backingStoreHandle.isNull()) {
        // Create a new backing store.
        m_backingStore = BackingStore::create(frameRect.size(), backingStoreHandle);
    }

    m_plugin->geometryDidChange(frameRect, clipRect);

    platformGeometryDidChange(frameRect, clipRect);
}

void PluginControllerProxy::didEvaluateJavaScript(uint64_t requestID, const String& requestURLString, const String& result)
{
    m_plugin->didEvaluateJavaScript(requestID, requestURLString, result);
}

void PluginControllerProxy::streamDidReceiveResponse(uint64_t streamID, const String& responseURLString, uint32_t streamLength, uint32_t lastModifiedTime, const String& mimeType, const String& headers)
{
    m_plugin->streamDidReceiveResponse(streamID, KURL(ParsedURLString, responseURLString), streamLength, lastModifiedTime, mimeType, headers);
}

void PluginControllerProxy::streamDidReceiveData(uint64_t streamID, const CoreIPC::DataReference& data)
{
    m_plugin->streamDidReceiveData(streamID, reinterpret_cast<const char*>(data.data()), data.size());
}

void PluginControllerProxy::streamDidFinishLoading(uint64_t streamID)
{
    m_plugin->streamDidFinishLoading(streamID);
}

void PluginControllerProxy::streamDidFail(uint64_t streamID, bool wasCancelled)
{
    m_plugin->streamDidFail(streamID, wasCancelled);
}

void PluginControllerProxy::manualStreamDidReceiveResponse(const String& responseURLString, uint32_t streamLength, uint32_t lastModifiedTime, const String& mimeType, const String& headers)
{
    if (m_pluginCanceledManualStreamLoad)
        return;

    m_plugin->manualStreamDidReceiveResponse(KURL(ParsedURLString, responseURLString), streamLength, lastModifiedTime, mimeType, headers);
}

void PluginControllerProxy::manualStreamDidReceiveData(const CoreIPC::DataReference& data)
{
    if (m_pluginCanceledManualStreamLoad)
        return;

    m_plugin->manualStreamDidReceiveData(reinterpret_cast<const char*>(data.data()), data.size());
}

void PluginControllerProxy::manualStreamDidFinishLoading()
{
    if (m_pluginCanceledManualStreamLoad)
        return;
    
    m_plugin->manualStreamDidFinishLoading();
}

void PluginControllerProxy::manualStreamDidFail(bool wasCancelled)
{
    if (m_pluginCanceledManualStreamLoad)
        return;
    
    m_plugin->manualStreamDidFail(wasCancelled);
}
    
void PluginControllerProxy::handleMouseEvent(const WebMouseEvent& mouseEvent, bool& handled)
{
    handled = m_plugin->handleMouseEvent(mouseEvent);
}

void PluginControllerProxy::handleWheelEvent(const WebWheelEvent& wheelEvent, bool& handled)
{
    handled = m_plugin->handleWheelEvent(wheelEvent);
}

void PluginControllerProxy::handleMouseEnterEvent(const WebMouseEvent& mouseEnterEvent, bool& handled)
{
    handled = m_plugin->handleMouseEnterEvent(mouseEnterEvent);
}

void PluginControllerProxy::handleMouseLeaveEvent(const WebMouseEvent& mouseLeaveEvent, bool& handled)
{
    handled = m_plugin->handleMouseLeaveEvent(mouseLeaveEvent);
}

void PluginControllerProxy::handleKeyboardEvent(const WebKeyboardEvent& keyboardEvent, bool& handled)
{
    handled = m_plugin->handleKeyboardEvent(keyboardEvent);
}

void PluginControllerProxy::paintEntirePlugin()
{
    m_dirtyRect = m_frameRect;
    paint();
}

void PluginControllerProxy::setFocus(bool hasFocus)
{
    m_plugin->setFocus(hasFocus);
}

void PluginControllerProxy::didUpdate()
{
    m_waitingForDidUpdate = false;
    startPaintTimer();
}

void PluginControllerProxy::getPluginScriptableNPObject(uint64_t& pluginScriptableNPObjectID)
{
    NPObject* pluginScriptableNPObject = m_plugin->pluginScriptableNPObject();
    if (!pluginScriptableNPObject) {
        pluginScriptableNPObjectID = 0;
        return;
    }
    
    pluginScriptableNPObjectID = m_connection->npRemoteObjectMap()->registerNPObject(pluginScriptableNPObject);
    releaseNPObject(pluginScriptableNPObject);
}

#if PLATFORM(MAC)
void PluginControllerProxy::windowFocusChanged(bool hasFocus)
{
    m_plugin->windowFocusChanged(hasFocus);
}

void PluginControllerProxy::windowAndViewFramesChanged(const IntRect& windowFrameInScreenCoordinates, const IntRect& viewFrameInWindowCoordinates)
{
    m_plugin->windowAndViewFramesChanged(windowFrameInScreenCoordinates, viewFrameInWindowCoordinates);
}

void PluginControllerProxy::windowVisibilityChanged(bool isVisible)
{
    m_plugin->windowVisibilityChanged(isVisible);
}

void PluginControllerProxy::sendComplexTextInput(const String& textInput)
{
    m_plugin->sendComplexTextInput(textInput);
}

#endif

void PluginControllerProxy::privateBrowsingStateChanged(bool isPrivateBrowsingEnabled)
{
    m_plugin->privateBrowsingStateChanged(isPrivateBrowsingEnabled);
}

} // namespace WebKit

#endif // ENABLE(PLUGIN_PROCESS)
