#include "config.h"
#include "ChromeClientNetflix.h"
#include "WebViewNetflix.h"
#include "NotImplemented.h"
#include "text/CString.h"
#include <stdio.h>

#include "Frame.h"
#include "FrameView.h"

#if USE(ACCELERATED_COMPOSITING)
# include "texmap/GraphicsLayerTextureMapper.h"
# include "texmap/TextureMapperPlatformLayer.h"
# include "texmap/TextureMapperGeneric.h"
#endif

using namespace WebCore;

namespace WebKit {

ChromeClientNetflix::ChromeClientNetflix(WebViewNetflix *webview) :
  m_webView(webview)
#if USE(ACCELERATED_COMPOSITING)
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

TextureMapperNodeClientNetflix::TextureMapperNodeClientNetflix(WebViewNetflix* view, GraphicsLayer* layer)
    : m_webView(view)
    , m_rootGraphicsLayer(GraphicsLayer::create(0))
{
    m_webView->rootTextureMapperNode = rootNode();
    m_rootGraphicsLayer->addChild(layer);
    m_rootGraphicsLayer->setDrawsContent(false);
    m_rootGraphicsLayer->setMasksToBounds(false);
    m_rootGraphicsLayer->setSize(IntSize(1000, 1000));
}

void TextureMapperNodeClientNetflix::setTextureMapper(const PassOwnPtr<TextureMapper>& textureMapper)
{
    m_webView->textureMapper = textureMapper;
    m_webView->rootTextureMapperNode->setTextureMapper(m_webView->textureMapper.get());
}

TextureMapperNodeClientNetflix::~TextureMapperNodeClientNetflix()
{
    m_webView->rootTextureMapperNode = 0;
}

void TextureMapperNodeClientNetflix::syncRootLayer()
{
    m_rootGraphicsLayer->syncCompositingStateForThisLayerOnly();
}

TextureMapperNode* TextureMapperNodeClientNetflix::rootNode()
{
    return toTextureMapperNode(m_rootGraphicsLayer.get());
}

void
ChromeClientNetflix::attachRootGraphicsLayer(WebCore::Frame*, WebCore::GraphicsLayer *layer)
{
    if (layer) {
        textureMapperNodeClient = adoptPtr(new TextureMapperNodeClientNetflix(m_webView, layer));
        textureMapperNodeClient->setTextureMapper(adoptPtr(new TextureMapperGeneric));
        textureMapperNodeClient->syncRootLayer();
        return;
    }
    textureMapperNodeClient.clear();
}

void
ChromeClientNetflix::setNeedsOneShotDrawingSynchronization()
{
    syncTimer.startOneShot(0);
}

void
ChromeClientNetflix::scheduleCompositingLayerSync()
{
    shouldSync = true;
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
    if (textureMapperNodeClient)
        textureMapperNodeClient->syncRootLayer();
    m_webView->syncCompositingState();
    if (!textureMapperNodeClient)
        return;
    if (textureMapperNodeClient->rootNode()->descendantsOrSelfHaveRunningAnimations())
        syncTimer.startOneShot(1.0 / 60.0);
    m_webView->notifyRepaint(IntRect(0, 0, m_webView->getWidth(), m_webView->getHeight()));
    if (!shouldSync)
        return;
    shouldSync = false;
}

#endif


}
