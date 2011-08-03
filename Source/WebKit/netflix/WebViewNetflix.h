#ifndef WebViewNetflix_H
#define WebViewNetflix_H

#include "PassRefPtr.h"
#include "KURL.h"
#include "IntRect.h"
#include "Color.h"
#include "WindowsKeyboardCodes.h"

#ifndef WTF_PLATFORM_NETFLIX
# define WTF_PLATFORM_NETFLIX 1
#endif
#ifndef WTF_USE_CAIRO
# define WTF_USE_CAIRO 1
#endif

#include <vector>

extern "C" {
#if USE(CAIRO)
    typedef struct _cairo_surface cairo_surface_t;
#endif
    typedef struct NPObject NPObject;
    typedef struct _NPVariant NPVariant;
}

namespace WebCore {
bool getSharedTimerFireDelay(long&);
void checkSharedTimer();

class Frame;
class Page;
class GraphicsContext;
class TextureMapperContentLayer;
}

namespace WebKit {

enum NetflixEvent {
    NetflixEventLoadFail,
    NetflixEventWindowClose,
    NetflixEventWindowClear,
    NetflixEventFrameCreate, /* data: WebCore::Frame created */
};

class ChromeClientNetflix;
class FrameLoaderClientNetflix;

#define WEBKIT_API __attribute__((visibility("default")))
#define WEBKIT_OBSOLETE_API WEBKIT_API __attribute__((deprecated))

class WEBKIT_API WebViewNetflix
{
public:
    WebViewNetflix();
    ~WebViewNetflix();

    void loadUri(char const *);
    void loadHtml(char const *);
    const char *currentUrl();

    void setSize(int, int);
    inline int getWidth() const { return m_size.width(); }
    inline int getHeight() const { return m_size.height(); }

    void checkTimers();

    inline WebCore::Color backgroundColor() const { return m_backgroundColor; }
    inline void setBackgroundColor(const WebCore::Color &color) { m_backgroundColor = color; }

    void setCookieJarFileName(const char* cookieJarFileName);

    void addToJavaScriptWindowObject(WebCore::Frame*, const char *, NPObject*);
#if ENABLE(NETSCAPE_PLUGIN_API)
    NPVariant evaluateJavaScript(WebCore::Frame*, const char *);
#endif

    void disableWebSecurity();
    
    void collectJavascriptGarbageNow();
    void collectJavascriptGarbageSoon();

    void* operator new(size_t);
    void operator delete(void*);

protected:
    void onKeyDown(char const *, int);
    void onKeyUp(char const *, int);
    void onMouseMove(float, float);
    void onMousePress(float, float);
    void onMouseRelease(float, float);
    void onWheelScroll(float, float, bool up);
#if USE(CAIRO)
    void onPaint(cairo_surface_t *, WebCore::IntRect);
#endif

    virtual void notify(const NetflixEvent &event, WebCore::Frame *frame = NULL) { }
    virtual void notifyRepaint(const WebCore::IntRect &) { }
    virtual WebCore::KURL notifyRequest(const WebCore::KURL &url) { return url; }
    virtual void javaScriptConsoleMessage(const char*, unsigned int, const char*);
    virtual bool canHandleRequest(const WebCore::KURL &) const { return true; }

private:
    friend class ChromeClientNetflix;
    friend class PlatformLayerNetflix;
    friend class FrameLoaderClientNetflix;

    void syncCompositingState();

    WebCore::IntSize m_size;
    WTF::RefPtr<WebCore::Frame> m_frame;

    WebCore::Page *m_page;
#if USE(ACCELERATED_COMPOSITING)
    WebCore::TextureMapperContentLayer* rootGraphicsLayer;
#endif
    WebCore::Color m_backgroundColor;
};

}

#endif
