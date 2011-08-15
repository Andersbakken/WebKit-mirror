#include "config.h"
#include "WebViewNetflix.h"

#include "ChromeClientNetflix.h"
#include "EditorClientNetflix.h"
#include "FrameLoaderClientNetflix.h"
#if ENABLE(INSPECTOR)
#include "InspectorClientNetflix.h"
#endif
#include "RenderTreeAsText.h"

#include <CurrentTime.h>
#include <FrameView.h>
#include <GCController.h>
#include <InitializeThreading.h>
#include <PageCache.h>
#include <MemoryCache.h>
#include <PlatformKeyboardEvent.h>
#include <PlatformMouseEvent.h>
#include <PlatformWheelEvent.h>
#include <ResourceHandle.h>
#include <ResourceHandleManager.h>
#include <ResourceRequest.h>
#include <Settings.h>
#include <SubstituteData.h>
#include <SharedBuffer.h>
#include <Threading.h>
#include <Page.h>
#include <image-decoders/jpeg/JPEGImageDecoder.h>

#if ENABLE(NETSCAPE_PLUGIN_API)
#include <JSDOMWindowBase.h>
#include <JSLock.h>
#include <ScriptSourceCode.h>
#include "npruntime.h"
#include "npruntime_impl.h"
#include "BridgeJSC.h"
#include "runtime_object.h"
#include "c_instance.h"
#include "c_utility.h"
#endif

#include "texmap/TextureMapperPlatformLayer.h"

using namespace WebCore;

#define FRAME_BUFFER_WIDTH 1280
#define FRAME_BUFFER_HEIGHT 720


namespace WebKit {

static bool init = false;
void initWebKitNetflix()
{
    if (!init) {
        JSC::initializeThreading();
        WTF::initializeMainThread();
        memoryCache()->setDisabled(false);
        memoryCache()->setCapacities(0, 5*1024*1024, 5*1024*1024);
        //achim:cache()->setCapacities(0, 8388608>>1, 8388608>>1);
        pageCache()->setCapacity(0);

        init = true;
    }
}


WebViewNetflix::WebViewNetflix() :
    m_page(0)
#if USE(ACCELERATED_COMPOSITING)
    ,rootTextureMapperNode(0)
#endif
{
    WebKit::initWebKitNetflix();

    WebCore::Page::PageClients clients;
    memset(&clients, 0, sizeof(clients));
    clients.chromeClient = new ChromeClientNetflix(this);
    clients.editorClient = new EditorClientNetflix(this);
#if ENABLE(INSPECTOR)
    clients.inspectorClient = new InspectorClientNetflix;
#endif
    m_page = new Page(clients);

    m_page->settings()->setFixedFontFamily("Courier New");
    m_page->settings()->setSerifFontFamily("Times New Roman");
    m_page->settings()->setCursiveFontFamily("Comic Sans MS");
    m_page->settings()->setFantasyFontFamily("Impact");
    m_page->settings()->setStandardFontFamily("Arial");
    m_page->settings()->setSansSerifFontFamily("Arial");

    // All these sizes are in pixels
    m_page->settings()->setMinimumFontSize(1);
    m_page->settings()->setDefaultFontSize(16);
    m_page->settings()->setDefaultFixedFontSize(13);
    m_page->settings()->setMinimumLogicalFontSize(9);

    m_page->settings()->setJavaEnabled(false);
    m_page->settings()->setPluginsEnabled(false);
    m_page->settings()->setLoadsImagesAutomatically(true);
    m_page->settings()->setJavaScriptEnabled(true);
    m_page->settings()->setDeveloperExtrasEnabled(false);
    m_page->settings()->setPrivateBrowsingEnabled(false);
    m_page->settings()->setJavaScriptCanOpenWindowsAutomatically(false);

#if defined(DBG_THOMAS) || defined(DBG_ERIC) || defined(DBG_HOLGER)     || defined(DBG_JENS) || defined(DBG_ACHIM) || 1
    m_page->settings()->setWebSecurityEnabled(false);
#endif

#if USE(ACCELERATED_COMPOSITING)
    const bool accelerated = getenv("NF_NO_ACCEL") == 0;
    m_page->settings()->setAcceleratedCompositingEnabled(accelerated);
    m_page->settings()->setAcceleratedCompositingFor3DTransformsEnabled(accelerated);
    m_page->settings()->setAcceleratedCompositingForAnimationEnabled(accelerated);
    //m_page->settings()->setAcceleratedCompositingForCanvasEnabled(value);
    m_page->settings()->setAcceleratedCompositingForVideoEnabled(false);
    m_page->settings()->setAcceleratedCompositingForPluginsEnabled(false);
#endif
    m_page->settings()->setShouldPrintBackgrounds(false);

    m_page->settings()->setLocalStorageEnabled(true);
#if ENABLE(DOM_STORAGE)
    m_page->settings()->setSessionStorageQuota(256 * 1024);
#endif
    m_page->settings()->setLocalStorageDatabasePath("LS");      // Name is arbitrary
    m_page->settings()->setAllowUniversalAccessFromFileURLs(true);
    m_page->settings()->setOfflineWebApplicationCacheEnabled(true);
    m_page->settings()->setUsesPageCache(false);

    m_page->settings()->setDefaultTextEncodingName("iso-8859-1");

    FrameLoaderClientNetflix *loader = new FrameLoaderClientNetflix;
    m_frame = Frame::create(m_page, NULL, loader);
    loader->setFrame(m_frame.get());
    loader->setWebView(this);

    m_frame->init();

    m_frame.get()->selection()->setCaretVisible(true);
}

WebViewNetflix::~WebViewNetflix()
{
    delete m_page;
}

void
WebViewNetflix::setSize(int w, int h)
{
    m_size = IntSize(w, h);
}

void
WebViewNetflix::loadUri(char const *url)
{
    m_frame->loader()->load(KURL(ParsedURLString, url), false);
}

void
WebViewNetflix::loadHtml(char const *html)
{
    const WebCore::ResourceRequest request;
    // TODO: This is not likely to be internationalized. It assumes the string parameter is UTF-8.
    WTF::RefPtr<WebCore::SharedBuffer> data( SharedBuffer::create(html, strlen(html)) );
    SubstituteData substituteData(data, String("text/html"), String("utf-8"), KURL());
    m_frame->loader()->load(request, substituteData, false);
}

const char*
WebViewNetflix::currentUrl()
{
#if 0
    return m_frame->loader()->url().string().latin1().data();
#else
    return 0;
#endif
}

#if USE(ACCELERATED_COMPOSITING)
void
WebViewNetflix::renderCompositedLayers(GraphicsContext* context, const IntRect& clip)
{
    if (!rootTextureMapperNode || !textureMapper)
        return;

    textureMapper->setGraphicsContext(context);
    textureMapper->setImageInterpolationQuality(context->imageInterpolationQuality());
    textureMapper->setTextDrawingMode(context->textDrawingMode());
    textureMapper->setViewportSize(m_frame->view()->frameRect().size());
    textureMapper->beginPainting();
    {
        textureMapper->beginClip(context->getCTM().toTransformationMatrix(), clip);
        rootTextureMapperNode->paint();
        textureMapper->endClip();
    }
    textureMapper->endPainting();
}
#endif

#if USE(CAIRO)
void
WebViewNetflix::onPaint(cairo_surface_t *surface, WebCore::IntRect rect)
{
    if(rect.isEmpty())
        rect = WebCore::IntRect(0, 0, m_size.width(), m_size.height());
    m_frame->view()->updateLayoutAndStyleIfNeededRecursive();

    cairo_t *cairo = cairo_create(surface);
    ASSERT(cairo);
    if(cairo) {
        GraphicsContext gc(cairo);
#if 0
        fprintf(stderr, "Render!! %d %d\n", m_size.width(), m_size.height());
        fprintf(stderr, "%s\n", WebCore::externalRepresentation(m_frame.get()).latin1().data());
#endif
        if(m_size.width() != FRAME_BUFFER_WIDTH || m_size.height() != FRAME_BUFFER_HEIGHT)
            gc.scale(FloatSize((float)FRAME_BUFFER_WIDTH / m_size.width(), (float)FRAME_BUFFER_HEIGHT / m_size.height()));
        m_frame->view()->paint(&gc, rect);
#if USE(ACCELERATED_COMPOSITING)
        renderCompositedLayers(&gc, rect);
#endif
#if 0
        frame->page()->inspectorController()->drawNodeHighlight(context);
#endif
    }
    cairo_destroy(cairo);
}
#endif

void
WebViewNetflix::onKeyDown(char const *key, int code, char const *text)
{
    PlatformKeyboardEvent keyPressEvent(PlatformKeyboardEvent::KeyDown, text, key, code);
    m_page->mainFrame()->eventHandler()->keyEvent(keyPressEvent);
}

void
WebViewNetflix::onKeyUp(char const *key, int code, char const *text)
{
    PlatformKeyboardEvent keyPressEvent(PlatformKeyboardEvent::KeyUp, text, key, code);
    m_page->mainFrame()->eventHandler()->keyEvent(keyPressEvent);
}

void
WebViewNetflix::onMouseMove(float x, float y)
{
    const IntPoint pos(x, y);
    PlatformMouseEvent mouseEvent(pos, pos,
                                  NoButton,
                                  MouseEventMoved,
                                  0,
                                  false, false, false, false,
                                  WTF::currentTime());
    m_page->mainFrame()->eventHandler()->mouseMoved(mouseEvent);
}

void
WebViewNetflix::onMousePress(float x, float y)
{
    const IntPoint pos(x, y);
    PlatformMouseEvent mouseEvent(pos, pos,
                                  LeftButton,
                                  MouseEventPressed,
                                  1,
                                  false, false, false, false,
                                  WTF::currentTime());
    m_page->mainFrame()->eventHandler()->handleMousePressEvent(mouseEvent);
}

void
WebViewNetflix::onMouseRelease(float x, float y)
{
    const IntPoint pos(x, y);
    PlatformMouseEvent mouseEvent(pos, pos,
                                  LeftButton,
                                  MouseEventReleased,
                                  0,
                                  false, false, false, false,
                                  WTF::currentTime());
    m_page->mainFrame()->eventHandler()->handleMouseReleaseEvent(mouseEvent);
}

void
WebViewNetflix::onWheelScroll(float x, float y, bool up)
{
    IntPoint pos(x, y);
    PlatformWheelEvent wheelEvent(pos, pos, up ? 1.0f : -1.0f, 0.0f, 0.0f, 0.0f, ScrollByPixelWheelEvent, false, false, false, false, false);
    m_page->mainFrame()->eventHandler()->handleWheelEvent(wheelEvent);
}

void
WebViewNetflix::syncCompositingState()
{
    m_page->mainFrame()->view()->syncCompositingStateIncludingSubframes();
}

void
WebViewNetflix::javaScriptConsoleMessage(const char* source, unsigned int line, const char* message)
{
    fprintf(stderr, "%s:%d: [%s]\n", source, line, message);
}

void
WebViewNetflix::setCookieJarFileName(const char *cookieJarFileName)
{
    ResourceHandleManager::sharedInstance()->setCookieJarFileName(cookieJarFileName);
}

void
WebViewNetflix::addToJavaScriptWindowObject(Frame *frame, const char *name, NPObject *object)
{
#if ENABLE(NETSCAPE_PLUGIN_API)
    JSC::JSLock lock(JSC::SilenceAssertionsOnly);
    JSDOMWindow* window = toJSDOMWindow(frame, mainThreadNormalWorld());
    if (!window)
        return;

    JSC::ExecState* exec = window->globalExec();

    JSC::Bindings::RootObject* root = frame->script()->bindingRootObject();
    if (!root)
        return;
    JSC::JSObject *runtimeObject =
        JSC::Bindings::CInstance::create(object, root)->createRuntimeObject(exec);

    JSC::PutPropertySlot slot;
    window->put(exec, JSC::Identifier(exec, name), runtimeObject, slot);
#endif
}

#if ENABLE(NETSCAPE_PLUGIN_API)
NPVariant
WebViewNetflix::evaluateJavaScript(Frame *frame, const char *scriptSource)
{
    if (!frame)
        frame = m_frame.get();
    ScriptController *proxy = frame->script();
    NPVariant rc;
    VOID_TO_NPVARIANT(rc);
    if (proxy) {
        JSC::JSValue v = frame->script()->executeScript(ScriptSourceCode(scriptSource)).jsValue();
        JSC::Bindings::convertValueToNPVariant(proxy->globalObject(mainThreadNormalWorld())->globalExec(), v, &rc);
    }
    return rc;
}
#endif

void
WebViewNetflix::disableWebSecurity()
{
    m_page->settings()->setWebSecurityEnabled(false);
}

void
WebViewNetflix::collectJavascriptGarbageNow()
{
    WebCore::gcController().garbageCollectNow();
}

void
WebViewNetflix::collectJavascriptGarbageSoon()
{
    WebCore::gcController().garbageCollectSoon();
}

void* WebViewNetflix::operator new(size_t size)
{
    return malloc(size);
}

void WebViewNetflix::operator delete(void *ptr)
{
    free(ptr);
}

}

