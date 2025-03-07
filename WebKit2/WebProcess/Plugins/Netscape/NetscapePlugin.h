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

#ifndef NetscapePlugin_h
#define NetscapePlugin_h

#include "NetscapePluginModule.h"
#include "Plugin.h"
#include "RunLoop.h"
#include <WebCore/GraphicsLayer.h>
#include <WebCore/IntRect.h>
#include <wtf/HashMap.h>
#include <wtf/text/CString.h>
#include <wtf/text/StringHash.h>

namespace WebCore {
    class HTTPHeaderMap;
}

namespace WebKit {

class NetscapePluginStream;
    
class NetscapePlugin : public Plugin {
public:
    static PassRefPtr<NetscapePlugin> create(PassRefPtr<NetscapePluginModule> pluginModule);
    virtual ~NetscapePlugin();

    static PassRefPtr<NetscapePlugin> fromNPP(NPP);

#if PLATFORM(MAC)
    NPError setDrawingModel(NPDrawingModel);
    NPError setEventModel(NPEventModel);
    NPBool convertPoint(double sourceX, double sourceY, NPCoordinateSpace sourceSpace, double& destX, double& destY, NPCoordinateSpace destSpace);

#ifndef NP_NO_CARBON
    WindowRef windowRef() const;
    bool isWindowActive() const { return m_windowHasFocus; }

    static NetscapePlugin* netscapePluginFromWindow(WindowRef);
    static unsigned buttonState();
#endif

#elif PLATFORM(WIN)
    HWND containingWindow() const;
#endif

    void invalidate(const NPRect*);
    static const char* userAgent(NPP);
    void loadURL(const String& method, const String& urlString, const String& target, const WebCore::HTTPHeaderMap& headerFields,
                 const Vector<uint8_t>& httpBody, bool sendNotification, void* notificationData);
    NPError destroyStream(NPStream*, NPReason);
    void setIsWindowed(bool);
    void setStatusbarText(const String&);
    static void setException(const String&);
    bool evaluate(NPObject*, const String&scriptString, NPVariant* result);
    bool isPrivateBrowsingEnabled();

    // These return retained objects.
    NPObject* windowScriptNPObject();
    NPObject* pluginElementNPObject();

    void cancelStreamLoad(NetscapePluginStream*);
    void removePluginStream(NetscapePluginStream*);

    bool isAcceleratedCompositingEnabled();

    void pushPopupsEnabledState(bool enabled);
    void popPopupsEnabledState();

    String proxiesForURL(const String& urlString);
    String cookiesForURL(const String& urlString);
    void setCookiesForURL(const String& urlString, const String& cookieString);

    // Member functions for calling into the plug-in.
    NPError NPP_New(NPMIMEType pluginType, uint16_t mode, int16_t argc, char* argn[], char* argv[], NPSavedData*);
    NPError NPP_Destroy(NPSavedData**);
    NPError NPP_SetWindow(NPWindow*);
    NPError NPP_NewStream(NPMIMEType, NPStream*, NPBool seekable, uint16_t* stype);
    NPError NPP_DestroyStream(NPStream*, NPReason);
    void NPP_StreamAsFile(NPStream*, const char* filename);
    int32_t NPP_WriteReady(NPStream*);
    int32_t NPP_Write(NPStream*, int32_t offset, int32_t len, void* buffer);
    int16_t NPP_HandleEvent(void* event);
    void NPP_URLNotify(const char* url, NPReason, void* notifyData);
    NPError NPP_GetValue(NPPVariable, void *value);
    NPError NPP_SetValue(NPNVariable, void *value);

private:
    NetscapePlugin(PassRefPtr<NetscapePluginModule> pluginModule);

    void callSetWindow();
    bool shouldLoadSrcURL();
    NetscapePluginStream* streamFromID(uint64_t streamID);
    void stopAllStreams();
    bool allowPopups() const;

    const char* userAgent();

    bool platformPostInitialize();
    void platformDestroy();
    bool platformInvalidate(const WebCore::IntRect&);
    void platformGeometryDidChange();
    void platformPaint(WebCore::GraphicsContext*, const WebCore::IntRect& dirtyRect);

    bool platformHandleMouseEvent(const WebMouseEvent&);
    bool platformHandleWheelEvent(const WebWheelEvent&);
    bool platformHandleMouseEnterEvent(const WebMouseEvent&);
    bool platformHandleMouseLeaveEvent(const WebMouseEvent&);
    bool platformHandleKeyboardEvent(const WebKeyboardEvent&);
    void platformSetFocus(bool);

    // Plugin
    virtual bool initialize(PluginController*, const Parameters&);
    virtual void destroy();
    virtual void paint(WebCore::GraphicsContext*, const WebCore::IntRect& dirtyRect);
#if PLATFORM(MAC)
    virtual PlatformLayer* pluginLayer();
#endif
    virtual void geometryDidChange(const WebCore::IntRect& frameRect, const WebCore::IntRect& clipRect);
    virtual void frameDidFinishLoading(uint64_t requestID);
    virtual void frameDidFail(uint64_t requestID, bool wasCancelled);
    virtual void didEvaluateJavaScript(uint64_t requestID, const String& requestURLString, const String& result);
    virtual void streamDidReceiveResponse(uint64_t streamID, const WebCore::KURL& responseURL, uint32_t streamLength, 
                                          uint32_t lastModifiedTime, const String& mimeType, const String& headers);
    virtual void streamDidReceiveData(uint64_t streamID, const char* bytes, int length);
    virtual void streamDidFinishLoading(uint64_t streamID);
    virtual void streamDidFail(uint64_t streamID, bool wasCancelled);
    virtual void manualStreamDidReceiveResponse(const WebCore::KURL& responseURL, uint32_t streamLength, 
                                                uint32_t lastModifiedTime, const String& mimeType, const String& headers);
    virtual void manualStreamDidReceiveData(const char* bytes, int length);
    virtual void manualStreamDidFinishLoading();
    virtual void manualStreamDidFail(bool wasCancelled);
    
    virtual bool handleMouseEvent(const WebMouseEvent&);
    virtual bool handleWheelEvent(const WebWheelEvent&);
    virtual bool handleMouseEnterEvent(const WebMouseEvent&);
    virtual bool handleMouseLeaveEvent(const WebMouseEvent&);
    virtual bool handleKeyboardEvent(const WebKeyboardEvent&);
    virtual void setFocus(bool);
    virtual NPObject* pluginScriptableNPObject();

#if PLATFORM(MAC)
    virtual void windowFocusChanged(bool);
    virtual void windowAndViewFramesChanged(const WebCore::IntRect& windowFrameInScreenCoordinates, const WebCore::IntRect& viewFrameInWindowCoordinates);
    virtual void windowVisibilityChanged(bool);

    virtual uint64_t pluginComplexTextInputIdentifier() const;
    virtual void sendComplexTextInput(const String& textInput);
#endif

    virtual void privateBrowsingStateChanged(bool);

    virtual PluginController* controller();

    PluginController* m_pluginController;
    uint64_t m_nextRequestID;

    typedef HashMap<uint64_t, std::pair<String, void*> > PendingURLNotifyMap;
    PendingURLNotifyMap m_pendingURLNotifications;

    typedef HashMap<uint64_t, RefPtr<NetscapePluginStream> > StreamsMap;
    StreamsMap m_streams;

    RefPtr<NetscapePluginModule> m_pluginModule;
    NPP_t m_npp;
    NPWindow m_npWindow;

    WebCore::IntRect m_frameRect;
    WebCore::IntRect m_clipRect;

    CString m_userAgent;

    bool m_isStarted;
    bool m_isWindowed;
    bool m_inNPPNew;
    bool m_loadManually;
    RefPtr<NetscapePluginStream> m_manualStream;
    Vector<bool, 8> m_popupEnabledStates;

#if PLATFORM(MAC)
    NPDrawingModel m_drawingModel;
    NPEventModel m_eventModel;
    RetainPtr<PlatformLayer> m_pluginLayer;

    bool m_pluginHasFocus;
    bool m_windowHasFocus;

    WebCore::IntRect m_windowFrameInScreenCoordinates;
    WebCore::IntRect m_viewFrameInWindowCoordinates;

#ifndef NP_NO_CARBON
    void nullEventTimerFired();

    // FIXME: It's a bit wasteful to have one null event timer per plug-in.
    // We should investigate having one per window.
    RunLoop::Timer<NetscapePlugin> m_nullEventTimer;
    NP_CGContext m_npCGContext;
#endif
#elif PLATFORM(WIN)
    HWND m_window;
#endif
};

} // namespace WebKit

#endif // NetscapePlugin_h
