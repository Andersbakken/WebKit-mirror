#include "config.h"
#include "ChromeClientNetflix.h"
#include "WebViewNetflix.h"
#include "NotImplemented.h"
#include "text/CString.h"
#include <stdio.h>

#if USE(ACCELERATED_COMPOSITING)
#include "texmap/GraphicsLayerTextureMapper.h"
#include "texmap/TextureMapperPlatformLayer.h"
#endif

using namespace WebCore;

namespace WebKit {

ChromeClientNetflix::ChromeClientNetflix(WebViewNetflix *webview) :
  m_webView(webview)
#if USE(ACCELERATED_COMPOSITING)
  , rootGraphicsLayer(0)
  , syncTimer(this, &ChromeClientNetflix::syncLayers)
#endif

{
}

ChromeClientNetflix::~ChromeClientNetflix()
{
}

void ChromeClientNetflix::chromeDestroyed()
{
    delete this;
}

float ChromeClientNetflix::scaleFactor()
{
    return 1.0f;
}

bool ChromeClientNetflix::canTakeFocus(WebCore::FocusDirection)
{
    return false;
}

void ChromeClientNetflix::takeFocus(WebCore::FocusDirection)
{
}

void ChromeClientNetflix::closeWindowSoon()
{
     if (m_webView)
         m_webView->notify(new WebKit::EventNetflix(EventNetflix::WindowClose));
}

bool ChromeClientNetflix::canRunBeforeUnloadConfirmPanel()
{
    return true;
}

bool ChromeClientNetflix::shouldInterruptJavaScript()
{
    return false;
}

void ChromeClientNetflix::contentsSizeChanged(WebCore::Frame* frame, const WebCore::IntSize& size) const
{
//  DEBUG_PRINTF("ChromeClientNetflix::contentsSizeChanged() %d x %d\n", size.width(), size.height());
    notImplemented();
}

void ChromeClientNetflix::runJavaScriptAlert(WebCore::Frame* frame, const WTF::String& str)
{
    printf("Alert: %s\n", str.latin1().data());
    notImplemented();
}

void ChromeClientNetflix::setStatusbarText(const WTF::String& str)
{
    notImplemented();
}

void ChromeClientNetflix::addMessageToConsole(WebCore::MessageSource source, WebCore::MessageType type,
                                          WebCore::MessageLevel level, const WTF::String& message,
                                          unsigned int lineNumber, const WTF::String& sourceID)
{
    m_webView->javaScriptConsoleMessage(sourceID.latin1().data(), lineNumber, message.latin1().data());
}

void ChromeClientNetflix::invalidateWindow(const WebCore::IntRect& rect, bool)
{
    m_webView->notifyRepaint(rect);
}

void ChromeClientNetflix::invalidateContentsAndWindow(const WebCore::IntRect& rect, bool)
{
    m_webView->notifyRepaint(rect);
}

void ChromeClientNetflix::invalidateContentsForSlowScroll(const WebCore::IntRect& rect, bool)
{
    m_webView->notifyRepaint(rect);
}

#if USE(ACCELERATED_COMPOSITING)

class PlatformLayerNetflix : public WebCore::TextureMapperLayerClient {
public:
    PlatformLayerNetflix(ChromeClientNetflix *client, WebCore::TextureMapperContentLayer *layer)
        : m_client(client)
        , m_layer(layer)
    {
        if (m_layer)
            m_layer->setPlatformLayerClient(this);
        m_client->m_webView->rootGraphicsLayer = m_layer;
    }
    ~PlatformLayerNetflix()
    {
        if (m_layer)
            m_layer->setPlatformLayerClient(0);
        if (m_client) {
            if( m_client->m_webView )
                m_client->m_webView->rootGraphicsLayer = 0;
            m_client->rootGraphicsLayer = 0;
        }
    }

    void setNeedsDisplay()
    {
        WebCore::IntRect bounds(0, 0, m_client->m_webView->getWidth(),
                                m_client->m_webView->getHeight());
        m_client->m_webView->notifyRepaint(bounds);
    }

    void setNeedsDisplayInRect(const WebCore::IntRect& rect)
    {
        m_client->m_webView->notifyRepaint(rect);
    }

    void setSizeChanged(const WebCore::IntSize& newSize)
    {
    }

    ChromeClientNetflix *m_client;
    WebCore::TextureMapperContentLayer* m_layer;
};

void
ChromeClientNetflix::attachRootGraphicsLayer(WebCore::Frame*, WebCore::GraphicsLayer *layer)
{
    if (layer) {
        rootGraphicsLayer = new PlatformLayerNetflix(this, static_cast<WebCore::TextureMapperContentLayer*>(layer->platformLayer()));
    } else {
        delete rootGraphicsLayer;
        rootGraphicsLayer = 0;
    }
}

void
ChromeClientNetflix::setNeedsOneShotDrawingSynchronization()
{
    syncTimer.startOneShot(0);
}

void
ChromeClientNetflix::scheduleCompositingLayerSync()
{
    syncTimer.startOneShot(0);
}

bool
ChromeClientNetflix::allowsAcceleratedCompositing() const
{
    return true;
}

void
ChromeClientNetflix::syncLayers(WebCore::Timer<ChromeClientNetflix>*)
{
    m_webView->syncCompositingState();
}

#endif


}
