#ifndef ChromeClientNetflix_h__
#define ChromeClientNetflix_h__

#include "ChromeClient.h"
#include "WebViewNetflix.h"
#include "Timer.h"
#include "NotImplemented.h"
#include "text/WTFString.h"

#if USE(ACCELERATED_COMPOSITING)
# include "GraphicsLayer.h"
# include "TextureMapper.h"
# include "TextureMapperNode.h"
#endif

namespace WebKit {

#if USE(ACCELERATED_COMPOSITING)
class ChromeClientNetflix;
class TextureMapperNodeClientNetflix {
public:
    TextureMapperNodeClientNetflix(WebViewNetflix *, WebCore::GraphicsLayer*);
    virtual ~TextureMapperNodeClientNetflix();
    void setTextureMapper(const PassOwnPtr<WebCore::TextureMapper>&);
    void syncRootLayer();
    WebCore::TextureMapperNode* rootNode();

private:
    WebViewNetflix *m_webView;
    OwnPtr<WebCore::GraphicsLayer> m_rootGraphicsLayer;
};
#endif

class ChromeClientNetflix : public WebCore::ChromeClient
{
public:
    ChromeClientNetflix(WebViewNetflix *);
    virtual ~ChromeClientNetflix();

    virtual void chromeDestroyed();

    virtual void* webView() const { return m_webView; }

    virtual void setWindowRect(const WebCore::FloatRect&) { breakpoint(); }
    virtual WebCore::FloatRect windowRect() { breakpoint(); return WebCore::FloatRect(); }
    virtual WebCore::FloatRect pageRect() { breakpoint(); return WebCore::FloatRect(); }

    virtual float scaleFactor();

    virtual void focus() { breakpoint(); }
    virtual void unfocus() { breakpoint(); }

    virtual bool canTakeFocus(WebCore::FocusDirection);
    virtual void takeFocus(WebCore::FocusDirection);

    virtual WebCore::KeyboardUIMode keyboardUIMode() { return WebCore::KeyboardAccessDefault; }

    virtual void focusedNodeChanged(WebCore::Node*) { breakpoint(); }

    virtual WebCore::Page* createWindow(WebCore::Frame*, const WebCore::FrameLoadRequest&, const WebCore::WindowFeatures&, const WebCore::NavigationAction&) { breakpoint(); return 0; }
    virtual void show() { breakpoint(); }

    virtual bool canRunModal() { breakpoint(); return false; }
    virtual void runModal() { breakpoint(); }

    virtual void setToolbarsVisible(bool) { breakpoint(); }
    virtual bool toolbarsVisible() { breakpoint(); return false; }

    virtual void setStatusbarVisible(bool) { breakpoint(); }
    virtual bool statusbarVisible() { breakpoint(); return false; }

    virtual void setScrollbarsVisible(bool) { breakpoint(); }
    virtual bool scrollbarsVisible() { breakpoint(); return false; }

    virtual void setMenubarVisible(bool) { breakpoint(); }
    virtual bool menubarVisible() { breakpoint(); return false; }

    virtual void setResizable(bool) { breakpoint(); }

    virtual void addMessageToConsole(WebCore::MessageSource source, WebCore::MessageType type,
                                     WebCore::MessageLevel level, const WTF::String& message,
                                     unsigned int lineNumber, const WTF::String& sourceID);

    virtual bool canRunBeforeUnloadConfirmPanel();
    virtual bool runBeforeUnloadConfirmPanel(const WTF::String&, WebCore::Frame*) { breakpoint(); return true; }

    virtual void closeWindowSoon();

    virtual void runJavaScriptAlert(WebCore::Frame*, const WTF::String&);
    virtual bool runJavaScriptConfirm(WebCore::Frame*, const WTF::String&) { breakpoint(); return true; }
    virtual bool runJavaScriptPrompt(WebCore::Frame*, const WTF::String&, const WTF::String&, WTF::String&) { breakpoint(); return true; }
    virtual void setStatusbarText(const WTF::String&);
    virtual bool shouldInterruptJavaScript();
    virtual bool tabsToLinks() const { breakpoint(); return false; }

    virtual WebCore::IntRect windowResizerRect() const { breakpoint(); return WebCore::IntRect(); }

    virtual void invalidateRootView(const WebCore::IntRect&, bool);
    virtual void invalidateContentsAndRootView(const WebCore::IntRect&, bool);
    virtual void invalidateContentsForSlowScroll(const WebCore::IntRect&, bool);
    virtual void scroll(const WebCore::IntSize&, const WebCore::IntRect&, const WebCore::IntRect&) { breakpoint(); }
    virtual WebCore::IntPoint screenToRootView(const WebCore::IntPoint& p) const { breakpoint(); return p; }
    virtual WebCore::IntRect rootViewToScreen(const WebCore::IntRect& r) const { breakpoint(); return r; }
    virtual PlatformPageClient platformPageClient() const { breakpoint(); return 0; }
    virtual void scrollbarsModeDidChange() const { breakpoint(); }
    virtual void setCursor(const WebCore::Cursor&) { breakpoint(); }

    virtual bool selectItemWritingDirectionIsNatural() { breakpoint(); return false; }
    virtual bool selectItemAlignmentFollowsMenuWritingDirection() { breakpoint(); return true; }
    virtual bool hasOpenedPopup() const { breakpoint(); return false; }
    virtual PassRefPtr<WebCore::PopupMenu> createPopupMenu(WebCore::PopupMenuClient*) const { breakpoint(); return 0; }
    virtual PassRefPtr<WebCore::SearchPopupMenu> createSearchPopupMenu(WebCore::PopupMenuClient*) const { breakpoint(); return 0; }

    virtual void focusedFrameChanged(WebCore::Frame*) { breakpoint(); }
    virtual void contentsSizeChanged(WebCore::Frame*, const WebCore::IntSize&) const;
    virtual void scrollRectIntoView(const WebCore::IntRect&, const WebCore::ScrollView*) const { breakpoint(); }

    virtual bool shouldRubberBandInDirection(WebCore::ScrollDirection) const { breakpoint(); return false; }
    virtual void mouseDidMoveOverElement(const WebCore::HitTestResult&, unsigned) { breakpoint(); }
    virtual void setCursorHiddenUntilMouseMoves(bool) { breakpoint(); }
    virtual void numWheelEventHandlersChanged(unsigned int) { breakpoint(); }

    virtual void setToolTip(const WTF::String&, WebCore::TextDirection) { breakpoint(); }

    virtual void print(WebCore::Frame*) { breakpoint(); }

#if ENABLE(DATABASE)
    virtual void exceededDatabaseQuota(WebCore::Frame*, const WTF::String&) { breakpoint(); }
#endif

    virtual void reachedMaxAppCacheSize(int64_t) { breakpoint(); }
    virtual void reachedApplicationCacheOriginQuota(WebCore::SecurityOrigin*, int64_t) { breakpoint(); }

#if ENABLE(OFFLINE_WEB_APPLICATIONS)
    virtual void reachedMaxAppCacheSize(int64_t) { breakpoint(); }
#endif

#if USE(ACCELERATED_COMPOSITING)
    virtual void attachRootGraphicsLayer(WebCore::Frame*, WebCore::GraphicsLayer*);
    virtual void setNeedsOneShotDrawingSynchronization();
    virtual void scheduleCompositingLayerSync();
    virtual bool allowsAcceleratedCompositing() const;
#endif

#if ENABLE(NOTIFICATIONS)
    virtual WebCore::NotificationPresenter* notificationPresenter() const { breakpoint(); return 0; }
#endif

    virtual void requestGeolocationPermissionForFrame(WebCore::Frame*, WebCore::Geolocation*) { breakpoint(); }
    virtual void cancelGeolocationPermissionRequestForFrame(WebCore::Frame*, WebCore::Geolocation*) { breakpoint(); }

    virtual void runOpenPanel(WebCore::Frame*, PassRefPtr<WebCore::FileChooser>) { breakpoint(); }
    virtual void chooseIconForFiles(const Vector<WTF::String>&, WebCore::FileChooser*) { breakpoint(); }
    virtual void loadIconForFiles(const WTF::Vector<WTF::String, 0u>&, WebCore::FileIconLoader*) { breakpoint(); }

    virtual void formStateDidChange(const WebCore::Node*) { }

private:
    void breakpoint() const { }
    WebViewNetflix *m_webView;

#if USE(ACCELERATED_COMPOSITING)
    friend class PlatformLayerNetflix;
    OwnPtr<TextureMapperNodeClientNetflix> textureMapperNodeClient;
    bool shouldSync;
    void syncLayers(WebCore::Timer<ChromeClientNetflix>*);
    WebCore::Timer<ChromeClientNetflix> syncTimer;
#endif

};

}

#endif // ChromeClientNetflix_h__
