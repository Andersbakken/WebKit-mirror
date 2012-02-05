
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

class PaintEvent : public WebKit::EventNetflix
{
public:
    PaintEvent(const WebCore::IntRect& area)
        : WebKit::EventNetflix(WebKit::EventNetflix::Repaint), m_area(area)
    {
    }

    inline const WebCore::IntRect& area() const { return m_area; }

private:
    WebCore::IntRect m_area;
};

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
#if USE(CAIRO)
    void onPaint(cairo_surface_t *surface, WebCore::IntRect area) { WebViewNetflix::onPaint(surface, area); }
#endif

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

protected:
    void processKeyEvent(const NfKeyEvent* event);
    void processMouseEvent(const NfMouseEvent* event);

    void processEvent(const NfEvent* event);

private:
    WebView* m_view;
};

WebView::WebView(Application* app)
    : WebViewNetflix(), m_app(app)
{
    setSize(app->width(), app->height());
    setCookieJarFileName("/tmp/cookies.netflix");
}

void WebView::notifyRepaint(const WebCore::IntRect& area)
{
    m_app->postEvent(new PaintEvent(area));
}

Application::Application(int& argc, char**& argv)
    : ApplicationNetflix(argc, argv), m_view(0)
{
}

void Application::processKeyEvent(const NfKeyEvent* event)
{
    if (!m_view)
        return;

    if (event->type() == NfEvent::KeyPress)
        m_view->onKeyDown(keyIdentifierForNfKeyCode(event->keyCode()).c_str(),
                          windowsKeyCodeForNfKeyCode(event->keyCode()),
                          keyTextForNfKeyEvent(*event).c_str());
    else
        m_view->onKeyUp(keyIdentifierForNfKeyCode(event->keyCode()).c_str(),
                        windowsKeyCodeForNfKeyCode(event->keyCode()),
                        keyTextForNfKeyEvent(*event).c_str());
}

void Application::processMouseEvent(const NfMouseEvent* event)
{
    if (!m_view)
        return;

    if (event->type() == NfEvent::MouseMove)
        m_view->onMouseMove(event->x(), event->y());
    else if (event->button() == NfMouseEvent::Left) {
        if (event->type() == NfEvent::MousePress)
            m_view->onMousePress(event->x(), event->y());
        else if (event->type() == NfEvent::MouseRelease)
            m_view->onMouseRelease(event->x(), event->y());
    }
}

void Application::processEvent(const NfEvent* event)
{
    switch (event->type()) {
    case NfEvent::KeyPress:
    case NfEvent::KeyRelease:
        processKeyEvent(static_cast<const NfKeyEvent*>(event));
        break;
    case NfEvent::MousePress:
    case NfEvent::MouseRelease:
    case NfEvent::MouseMove:
        processMouseEvent(static_cast<const NfMouseEvent*>(event));
        break;
    case WebKit::EventNetflix::Repaint: {
        if (!m_view)
            break;
        const WebCore::IntRect& area = static_cast<const PaintEvent*>(event)->area();

        cairo_surface_t* cairoSurface = surface();
        m_view->onPaint(cairoSurface, area);
        cairo_surface_destroy(cairoSurface);

        flush(area.x(), area.y(), area.x() + area.width(), area.y() + area.height());

        break; }
    default:
        ApplicationNetflix::processEvent(event);
        break;
    }
}

void Application::setWebView(WebView* view)
{
    m_view = view;
}

int main(int argc, char** argv)
{
    Application app(argc, argv);
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
