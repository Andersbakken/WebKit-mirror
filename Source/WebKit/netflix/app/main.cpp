
#ifndef WTF_PLATFORM_NETFLIX
# define WTF_PLATFORM_NETFLIX 1
#endif
#ifndef WTF_USE_CAIRO
# define WTF_USE_CAIRO 1
#endif

#include "config.h"
#include <ApplicationNetflix.h>
#include <WebViewNetflix.h>
#include <cairo/cairo-directfb.h>

static std::string keyIdentifierForNfKeyCode(int code)
{
    /*switch (code) {
    }*/
    return std::string();
}

static int windowsKeyCodeForNfKeyCode(int code)
{
    return code;
}

static std::string keyTextForNfKeyEvent(const NfKeyEvent& event)
{
    return event.text();
}

class Application;

class WebView : public WebKit::WebViewNetflix
{
public:
    WebView(Application* app);

    void onKeyDown(char const *key, int code, char const *text) { WebViewNetflix::onKeyDown(key, code, text); }
    void onKeyUp(char const *key, int code, char const *text) { WebViewNetflix::onKeyUp(key, code, text); }
    void onMouseMove(float x, float y) { WebViewNetflix::onMouseMove(x, y); }
    void onMousePress(float x, float y) { WebViewNetflix::onMousePress(x, y); }
    void onMouseRelease(float x, float y) { WebViewNetflix::onMouseRelease(x, y); }
    void onWheelScroll(float x, float y, bool up) { WebViewNetflix::onWheelScroll(x, y, up); }

protected:
    void notifyRepaint(const WebCore::IntRect& area);

private:
    Application* m_app;
};

class Application : public WebKit::ApplicationNetflix
{
public:
    Application(int& argc, char**& argv);

    void setWebView(WebView* view);

    void processKeyEvent(const NfKeyEvent& event);
    void processMouseEvent(const NfMouseEvent& event);

private:
    WebView* m_view;
};

WebView::WebView(Application* app)
    : WebViewNetflix(), m_app(app)
{
    setSize(1280, 720);
    setCookieJarFileName("/tmp/cookies.netflix");
}

void WebView::notifyRepaint(const WebCore::IntRect& area)
{
    printf("repaint!\n");
    IDirectFB* dfb = static_cast<IDirectFB*>(m_app->systemHandle());
    IDirectFBSurface* dfbSurface = static_cast<IDirectFBSurface*>(m_app->surfaceHandle());
    cairo_surface_t* cairoSurface = cairo_directfb_surface_create(dfb, dfbSurface);
    WebViewNetflix::onPaint(cairoSurface, area);
    cairo_surface_destroy(cairoSurface);
}

Application::Application(int& argc, char**& argv)
    : ApplicationNetflix(argc, argv), m_view(0)
{
}

void Application::processKeyEvent(const NfKeyEvent& event)
{
    if (!m_view)
        return;

    if (event.isPress())
        m_view->onKeyDown(keyIdentifierForNfKeyCode(event.keyCode()).c_str(),
                          windowsKeyCodeForNfKeyCode(event.keyCode()),
                          keyTextForNfKeyEvent(event).c_str());
    else
        m_view->onKeyUp(keyIdentifierForNfKeyCode(event.keyCode()).c_str(),
                        windowsKeyCodeForNfKeyCode(event.keyCode()),
                        keyTextForNfKeyEvent(event).c_str());
}

void Application::processMouseEvent(const NfMouseEvent& event)
{
    if (m_view)
        m_view->onMouseMove(event.x(), event.y());
}

void Application::setWebView(WebView* view)
{
    m_view = view;
}

int main(int argc, char** argv)
{
    Application app(argc, argv);
    app.createPrimarySurface(1280, 720);
    WebView view(&app);
    app.setWebView(&view);

    std::string uri;
    if (argc == 1)
        uri = "http://tenfootui.netflix.com/htmltvui";
    else
        uri = argv[1];
    view.loadUri(uri.c_str());

    return app.exec();
}
