/*
 * Copyright (C) 2007, 2008 Holger Hans Peter Freyther
 * Copyright (C) 2007, 2008 Christian Dywan <christian@imendio.com>
 * Copyright (C) 2008 Nuanti Ltd.
 * Copyright (C) 2008 Alp Toker <alp@atoker.com>
 * Copyright (C) 2008 Gustavo Noronha Silva <gns@gnome.org>
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"
#include "ChromeClientGtk.h"

#include "Chrome.h"
#include "Console.h"
#include "DumpRenderTreeSupportGtk.h"
#include "Element.h"
#include "FileChooser.h"
#include "FileSystem.h"
#include "FloatRect.h"
#include "FrameLoadRequest.h"
#include "FrameView.h"
#include "GtkVersioning.h"
#include "HTMLNames.h"
#include "HitTestResult.h"
#include "Icon.h"
#include "IntRect.h"
#include "KURL.h"
#include "NavigationAction.h"
#include "NotImplemented.h"
#include "PlatformString.h"
#include "PopupMenuClient.h"
#include "PopupMenuGtk.h"
#include "SearchPopupMenuGtk.h"
#include "SecurityOrigin.h"
#include "WindowFeatures.h"
#include "webkitgeolocationpolicydecision.h"
#include "webkitnetworkrequest.h"
#include "webkitprivate.h"
#include "webkitsecurityoriginprivate.h"
#include "webkitviewportattributesprivate.h"
#include "webkitwebframeprivate.h"
#include "webkitwebview.h"
#include "webkitwebviewprivate.h"
#include <glib.h>
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include <wtf/text/CString.h>

#if ENABLE(DATABASE)
#include "DatabaseTracker.h"
#endif

using namespace WebCore;

namespace WebKit {

ChromeClient::ChromeClient(WebKitWebView* webView)
    : m_webView(webView)
    , m_closeSoonTimer(0)
{
    ASSERT(m_webView);
}

void ChromeClient::chromeDestroyed()
{
    delete this;
}

FloatRect ChromeClient::windowRect()
{
    GtkWidget* window = gtk_widget_get_toplevel(GTK_WIDGET(m_webView));
    if (gtk_widget_is_toplevel(window)) {
        gint left, top, width, height;
        gtk_window_get_position(GTK_WINDOW(window), &left, &top);
        gtk_window_get_size(GTK_WINDOW(window), &width, &height);
        return IntRect(left, top, width, height);
    }
    return FloatRect();
}

void ChromeClient::setWindowRect(const FloatRect& rect)
{
    IntRect intrect = IntRect(rect);
    WebKitWebWindowFeatures* webWindowFeatures = webkit_web_view_get_window_features(m_webView);

    g_object_set(webWindowFeatures,
                 "x", intrect.x(),
                 "y", intrect.y(),
                 "width", intrect.width(),
                 "height", intrect.height(),
                 NULL);

    gboolean autoResizeWindow;
    WebKitWebSettings* settings = webkit_web_view_get_settings(m_webView);
    g_object_get(settings, "auto-resize-window", &autoResizeWindow, NULL);

    if (!autoResizeWindow)
        return;

    GtkWidget* window = gtk_widget_get_toplevel(GTK_WIDGET(m_webView));
    if (gtk_widget_is_toplevel(window)) {
        gtk_window_move(GTK_WINDOW(window), intrect.x(), intrect.y());
        gtk_window_resize(GTK_WINDOW(window), intrect.width(), intrect.height());
    }
}

FloatRect ChromeClient::pageRect()
{
    GtkAllocation allocation;
#if GTK_CHECK_VERSION(2, 18, 0)
    gtk_widget_get_allocation(GTK_WIDGET(m_webView), &allocation);
#else
    allocation = GTK_WIDGET(m_webView)->allocation;
#endif
    return IntRect(allocation.x, allocation.y, allocation.width, allocation.height);
}

float ChromeClient::scaleFactor()
{
    // Not implementable
    return 1.0;
}

void ChromeClient::focus()
{
    gtk_widget_grab_focus(GTK_WIDGET(m_webView));
}

void ChromeClient::unfocus()
{
    GtkWidget* window = gtk_widget_get_toplevel(GTK_WIDGET(m_webView));
    if (gtk_widget_is_toplevel(window))
        gtk_window_set_focus(GTK_WINDOW(window), NULL);
}

Page* ChromeClient::createWindow(Frame* frame, const FrameLoadRequest& frameLoadRequest, const WindowFeatures& coreFeatures, const NavigationAction&)
{
    WebKitWebView* webView = 0;

    g_signal_emit_by_name(m_webView, "create-web-view", kit(frame), &webView);

    if (!webView)
        return 0;

    WebKitWebWindowFeatures* webWindowFeatures = webkit_web_window_features_new_from_core_features(coreFeatures);
    g_object_set(webView, "window-features", webWindowFeatures, NULL);
    g_object_unref(webWindowFeatures);

    if (!frameLoadRequest.isEmpty())
        webkit_web_view_open(webView, frameLoadRequest.resourceRequest().url().string().utf8().data());

    return core(webView);
}

void ChromeClient::show()
{
    webkit_web_view_notify_ready(m_webView);
}

bool ChromeClient::canRunModal()
{
    notImplemented();
    return false;
}

void ChromeClient::runModal()
{
    notImplemented();
}

void ChromeClient::setToolbarsVisible(bool visible)
{
    WebKitWebWindowFeatures* webWindowFeatures = webkit_web_view_get_window_features(m_webView);

    g_object_set(webWindowFeatures, "toolbar-visible", visible, NULL);
}

bool ChromeClient::toolbarsVisible()
{
    WebKitWebWindowFeatures* webWindowFeatures = webkit_web_view_get_window_features(m_webView);
    gboolean visible;

    g_object_get(webWindowFeatures, "toolbar-visible", &visible, NULL);
    return visible;
}

void ChromeClient::setStatusbarVisible(bool visible)
{
    WebKitWebWindowFeatures* webWindowFeatures = webkit_web_view_get_window_features(m_webView);

    g_object_set(webWindowFeatures, "statusbar-visible", visible, NULL);
}

bool ChromeClient::statusbarVisible()
{
    WebKitWebWindowFeatures* webWindowFeatures = webkit_web_view_get_window_features(m_webView);
    gboolean visible;

    g_object_get(webWindowFeatures, "statusbar-visible", &visible, NULL);
    return visible;
}

void ChromeClient::setScrollbarsVisible(bool visible)
{
    WebKitWebWindowFeatures* webWindowFeatures = webkit_web_view_get_window_features(m_webView);

    g_object_set(webWindowFeatures, "scrollbar-visible", visible, NULL);
}

bool ChromeClient::scrollbarsVisible() {
    WebKitWebWindowFeatures* webWindowFeatures = webkit_web_view_get_window_features(m_webView);
    gboolean visible;

    g_object_get(webWindowFeatures, "scrollbar-visible", &visible, NULL);
    return visible;
}

void ChromeClient::setMenubarVisible(bool visible)
{
    WebKitWebWindowFeatures* webWindowFeatures = webkit_web_view_get_window_features(m_webView);

    g_object_set(webWindowFeatures, "menubar-visible", visible, NULL);
}

bool ChromeClient::menubarVisible()
{
    WebKitWebWindowFeatures* webWindowFeatures = webkit_web_view_get_window_features(m_webView);
    gboolean visible;

    g_object_get(webWindowFeatures, "menubar-visible", &visible, NULL);
    return visible;
}

void ChromeClient::setResizable(bool)
{
    // Ignored for now
}

static gboolean emitCloseWebViewSignalLater(WebKitWebView* view)
{
    gboolean isHandled;
    g_signal_emit_by_name(view, "close-web-view", &isHandled);
    return FALSE;
}

void ChromeClient::closeWindowSoon()
{
    // We may not have a WebView as create-web-view can return NULL.
    if (!m_webView)
        return;
    if (m_closeSoonTimer) // Don't call close-web-view more than once.
        return;

    // We need to remove the parent WebView from WebViewSets here, before it actually
    // closes, to make sure that JavaScript code that executes before it closes
    // can't find it. Otherwise, window.open will select a closed WebView instead of 
    // opening a new one <rdar://problem/3572585>.
    m_webView->priv->corePage->setGroupName("");

    // We also need to stop the load to prevent further parsing or JavaScript execution
    // after the window has torn down <rdar://problem/4161660>.
    webkit_web_view_stop_loading(m_webView);

    // Clients commonly destroy the web view during the close-web-view signal, but our caller
    // may need to send more signals to the web view. For instance, if this happened in the
    // onload handler, it will need to call FrameLoaderClient::dispatchDidHandleOnloadEvents.
    // Instead of firing the close-web-view signal now, fire it after the caller finishes.
    // This seems to match the Mac/Windows port behavior.
    m_closeSoonTimer = g_timeout_add(0, reinterpret_cast<GSourceFunc>(emitCloseWebViewSignalLater), m_webView);
}

bool ChromeClient::canTakeFocus(FocusDirection)
{
    return gtk_widget_get_can_focus(GTK_WIDGET(m_webView));
}

void ChromeClient::takeFocus(FocusDirection)
{
    unfocus();
}

void ChromeClient::focusedNodeChanged(Node*)
{
}

void ChromeClient::focusedFrameChanged(Frame*)
{
}

bool ChromeClient::canRunBeforeUnloadConfirmPanel()
{
    return true;
}

bool ChromeClient::runBeforeUnloadConfirmPanel(const WTF::String& message, WebCore::Frame* frame)
{
    return runJavaScriptConfirm(frame, message);
}

void ChromeClient::addMessageToConsole(WebCore::MessageSource source, WebCore::MessageType type, WebCore::MessageLevel level, const WTF::String& message, unsigned int lineNumber, const WTF::String& sourceId)
{
    gboolean retval;
    g_signal_emit_by_name(m_webView, "console-message", message.utf8().data(), lineNumber, sourceId.utf8().data(), &retval);
}

void ChromeClient::runJavaScriptAlert(Frame* frame, const String& message)
{
    gboolean retval;
    g_signal_emit_by_name(m_webView, "script-alert", kit(frame), message.utf8().data(), &retval);
}

bool ChromeClient::runJavaScriptConfirm(Frame* frame, const String& message)
{
    gboolean retval;
    gboolean didConfirm;
    g_signal_emit_by_name(m_webView, "script-confirm", kit(frame), message.utf8().data(), &didConfirm, &retval);
    return didConfirm == TRUE;
}

bool ChromeClient::runJavaScriptPrompt(Frame* frame, const String& message, const String& defaultValue, String& result)
{
    gboolean retval;
    gchar* value = 0;
    g_signal_emit_by_name(m_webView, "script-prompt", kit(frame), message.utf8().data(), defaultValue.utf8().data(), &value, &retval);
    if (value) {
        result = String::fromUTF8(value);
        g_free(value);
        return true;
    }
    return false;
}

void ChromeClient::setStatusbarText(const String& string)
{
    CString stringMessage = string.utf8();
    g_signal_emit_by_name(m_webView, "status-bar-text-changed", stringMessage.data());
}

bool ChromeClient::shouldInterruptJavaScript()
{
    notImplemented();
    return false;
}

bool ChromeClient::tabsToLinks() const
{
    if (DumpRenderTreeSupportGtk::dumpRenderTreeModeEnabled())
        return DumpRenderTreeSupportGtk::linksIncludedInFocusChain();

    return true;
}

IntRect ChromeClient::windowResizerRect() const
{
    notImplemented();
    return IntRect();
}

void ChromeClient::invalidateWindow(const IntRect&, bool)
{
    notImplemented();
}

void ChromeClient::invalidateContentsAndWindow(const IntRect& updateRect, bool immediate)
{
    GdkRectangle rect = updateRect;
    GdkWindow* window = gtk_widget_get_window(GTK_WIDGET(m_webView));

    if (window) {
        gdk_window_invalidate_rect(window, &rect, FALSE);
        // We don't currently do immediate updates since they delay other UI elements.
        //if (immediate)
        //    gdk_window_process_updates(window, FALSE);
    }
}

void ChromeClient::invalidateContentsForSlowScroll(const IntRect& updateRect, bool immediate)
{
    invalidateContentsAndWindow(updateRect, immediate);
}

void ChromeClient::scroll(const IntSize& delta, const IntRect& rectToScroll, const IntRect& clipRect)
{
    GdkWindow* window = gtk_widget_get_window(GTK_WIDGET(m_webView));
    if (!window)
        return;

    // We cannot use gdk_window_scroll here because it is only able to
    // scroll the whole window at once, and we often need to scroll
    // portions of the window only (think frames).
    GdkRectangle area = clipRect;
    GdkRectangle moveRect;

    GdkRectangle sourceRect = area;
    sourceRect.x -= delta.width();
    sourceRect.y -= delta.height();

#ifdef GTK_API_VERSION_2
    GdkRegion* invalidRegion = gdk_region_rectangle(&area);

    if (gdk_rectangle_intersect(&area, &sourceRect, &moveRect)) {
        GdkRegion* moveRegion = gdk_region_rectangle(&moveRect);
        gdk_window_move_region(window, moveRegion, delta.width(), delta.height());
        gdk_region_offset(moveRegion, delta.width(), delta.height());
        gdk_region_subtract(invalidRegion, moveRegion);
        gdk_region_destroy(moveRegion);
    }

    gdk_window_invalidate_region(window, invalidRegion, FALSE);
    gdk_region_destroy(invalidRegion);
#else
    cairo_region_t* invalidRegion = cairo_region_create_rectangle(&area);

    if (gdk_rectangle_intersect(&area, &sourceRect, &moveRect)) {
        cairo_region_t* moveRegion = cairo_region_create_rectangle(&moveRect);
        gdk_window_move_region(window, moveRegion, delta.width(), delta.height());
        cairo_region_translate(moveRegion, delta.width(), delta.height());
        cairo_region_subtract(invalidRegion, moveRegion);
        cairo_region_destroy(moveRegion);
    }

    gdk_window_invalidate_region(window, invalidRegion, FALSE);
    cairo_region_destroy(invalidRegion);
#endif

    gdk_window_process_updates(window, TRUE);
}

// FIXME: this does not take into account the WM decorations
static IntPoint widgetScreenPosition(GtkWidget* widget)
{
    GtkWidget* window = gtk_widget_get_toplevel(widget);
    int widgetX = 0, widgetY = 0;

    gtk_widget_translate_coordinates(widget, window, 0, 0, &widgetX, &widgetY);

    IntPoint result(widgetX, widgetY);
    int originX, originY;
    gdk_window_get_origin(gtk_widget_get_window(window), &originX, &originY);
    result.move(originX, originY);

    return result;
}

IntRect ChromeClient::windowToScreen(const IntRect& rect) const
{
    IntRect result(rect);
    IntPoint screenPosition = widgetScreenPosition(GTK_WIDGET(m_webView));
    result.move(screenPosition.x(), screenPosition.y());

    return result;
}

IntPoint ChromeClient::screenToWindow(const IntPoint& point) const
{
    IntPoint result(point);
    IntPoint screenPosition = widgetScreenPosition(GTK_WIDGET(m_webView));
    result.move(-screenPosition.x(), -screenPosition.y());

    return result;
}

PlatformPageClient ChromeClient::platformPageClient() const
{
    return GTK_WIDGET(m_webView);
}

void ChromeClient::contentsSizeChanged(Frame* frame, const IntSize& size) const
{
    // We need to queue a resize request only if the size changed,
    // otherwise we get into an infinite loop!
    GtkWidget* widget = GTK_WIDGET(m_webView);
    GtkRequisition requisition;
#if GTK_CHECK_VERSION(2, 20, 0)
    gtk_widget_get_requisition(widget, &requisition);
#else
    requisition = widget->requisition;
#endif
    if (gtk_widget_get_realized(widget)
        && (requisition.height != size.height())
        || (requisition.width != size.width()))
        gtk_widget_queue_resize_no_redraw(widget);
}

void ChromeClient::scrollbarsModeDidChange() const
{
    WebKitWebFrame* webFrame = webkit_web_view_get_main_frame(m_webView);

    g_object_notify(G_OBJECT(webFrame), "horizontal-scrollbar-policy");
    g_object_notify(G_OBJECT(webFrame), "vertical-scrollbar-policy");

    gboolean isHandled;
    g_signal_emit_by_name(webFrame, "scrollbars-policy-changed", &isHandled);

    if (isHandled)
        return;

    GtkWidget* parent = gtk_widget_get_parent(GTK_WIDGET(m_webView));
    if (!parent || !GTK_IS_SCROLLED_WINDOW(parent))
        return;

    GtkPolicyType horizontalPolicy = webkit_web_frame_get_horizontal_scrollbar_policy(webFrame);
    GtkPolicyType verticalPolicy = webkit_web_frame_get_vertical_scrollbar_policy(webFrame);

    // ScrolledWindow doesn't like to display only part of a widget if
    // the scrollbars are completely disabled; We have a disparity
    // here on what the policy requested by the web app is and what we
    // can represent; the idea is not to show scrollbars, only.
    if (horizontalPolicy == GTK_POLICY_NEVER)
        horizontalPolicy = GTK_POLICY_AUTOMATIC;

    if (verticalPolicy == GTK_POLICY_NEVER)
        verticalPolicy = GTK_POLICY_AUTOMATIC;

    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(parent),
                                   horizontalPolicy, verticalPolicy);
}

void ChromeClient::mouseDidMoveOverElement(const HitTestResult& hit, unsigned modifierFlags)
{
    // check if the element is a link...
    bool isLink = hit.isLiveLink();
    if (isLink) {
        KURL url = hit.absoluteLinkURL();
        if (!url.isEmpty() && url != m_hoveredLinkURL) {
            TextDirection dir;
            CString titleString = hit.title(dir).utf8();
            CString urlString = url.prettyURL().utf8();
            g_signal_emit_by_name(m_webView, "hovering-over-link", titleString.data(), urlString.data());
            m_hoveredLinkURL = url;
        }
    } else if (!isLink && !m_hoveredLinkURL.isEmpty()) {
        g_signal_emit_by_name(m_webView, "hovering-over-link", 0, 0);
        m_hoveredLinkURL = KURL();
    }

    if (Node* node = hit.innerNonSharedNode()) {
        Frame* frame = node->document()->frame();
        FrameView* view = frame ? frame->view() : 0;
        m_webView->priv->tooltipArea = view ? view->contentsToWindow(node->getRect()) : IntRect();
    } else
        m_webView->priv->tooltipArea = IntRect();
}

void ChromeClient::setToolTip(const String& toolTip, TextDirection)
{
    webkit_web_view_set_tooltip_text(m_webView, toolTip.utf8().data());
}

void ChromeClient::print(Frame* frame)
{
    WebKitWebFrame* webFrame = kit(frame);
    gboolean isHandled = false;
    g_signal_emit_by_name(m_webView, "print-requested", webFrame, &isHandled);

    if (isHandled)
        return;

    webkit_web_frame_print(webFrame);
}

#if ENABLE(DATABASE)
void ChromeClient::exceededDatabaseQuota(Frame* frame, const String& databaseName)
{
    guint64 defaultQuota = webkit_get_default_web_database_quota();
    DatabaseTracker::tracker().setQuota(frame->document()->securityOrigin(), defaultQuota);

    WebKitWebFrame* webFrame = kit(frame);
    WebKitWebView* webView = getViewFromFrame(webFrame);

    WebKitSecurityOrigin* origin = webkit_web_frame_get_security_origin(webFrame);
    WebKitWebDatabase* webDatabase = webkit_security_origin_get_web_database(origin, databaseName.utf8().data());
    g_signal_emit_by_name(webView, "database-quota-exceeded", webFrame, webDatabase);
}
#endif

#if ENABLE(OFFLINE_WEB_APPLICATIONS)
void ChromeClient::reachedMaxAppCacheSize(int64_t spaceNeeded)
{
    // FIXME: Free some space.
    notImplemented();
}

void ChromeClient::reachedApplicationCacheOriginQuota(SecurityOrigin*)
{
    notImplemented();
}
#endif

void ChromeClient::runOpenPanel(Frame*, PassRefPtr<FileChooser> prpFileChooser)
{
    RefPtr<FileChooser> chooser = prpFileChooser;

    GtkWidget* dialog = gtk_file_chooser_dialog_new(_("Upload File"),
                                                    GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(platformPageClient()))),
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                    GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                                    NULL);

    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), chooser->allowsMultipleFiles());

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        if (gtk_file_chooser_get_select_multiple(GTK_FILE_CHOOSER(dialog))) {
            GSList* filenames = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));
            Vector<String> names;
            for (GSList* item = filenames ; item ; item = item->next) {
                if (!item->data)
                    continue;
                names.append(filenameToString(static_cast<char*>(item->data)));
                g_free(item->data);
            }
            g_slist_free(filenames);
            chooser->chooseFiles(names);
        } else {
            gchar* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
            if (filename)
                chooser->chooseFile(filenameToString(filename));
            g_free(filename);
        }
    }
    gtk_widget_destroy(dialog);
}

void ChromeClient::chooseIconForFiles(const Vector<WTF::String>& filenames, WebCore::FileChooser* chooser)
{
    chooser->iconLoaded(Icon::createIconForFiles(filenames));
}

void ChromeClient::dispatchViewportDataDidChange(const ViewportArguments& arguments) const
{
    // Recompute the viewport attributes making it valid.
    webkitViewportAttributesRecompute(webkit_web_view_get_viewport_attributes(m_webView));
}

void ChromeClient::setCursor(const Cursor&)
{
    notImplemented();
}

void ChromeClient::requestGeolocationPermissionForFrame(Frame* frame, Geolocation* geolocation)
{
    WebKitWebFrame* webFrame = kit(frame);
    WebKitWebView* webView = getViewFromFrame(webFrame);

    PlatformRefPtr<WebKitGeolocationPolicyDecision> policyDecision(adoptPlatformRef(webkit_geolocation_policy_decision_new(webFrame, geolocation)));

    gboolean isHandled = FALSE;
    g_signal_emit_by_name(webView, "geolocation-policy-decision-requested", webFrame, policyDecision.get(), &isHandled);
    if (!isHandled)
        webkit_geolocation_policy_deny(policyDecision.get());
}

void ChromeClient::cancelGeolocationPermissionRequestForFrame(WebCore::Frame* frame, WebCore::Geolocation*)
{
    WebKitWebFrame* webFrame = kit(frame);
    WebKitWebView* webView = getViewFromFrame(webFrame);
    g_signal_emit_by_name(webView, "geolocation-policy-decision-cancelled", webFrame);
}

bool ChromeClient::selectItemWritingDirectionIsNatural()
{
    return true;
}

PassRefPtr<WebCore::PopupMenu> ChromeClient::createPopupMenu(WebCore::PopupMenuClient* client) const
{
    return adoptRef(new PopupMenuGtk(client));
}

PassRefPtr<WebCore::SearchPopupMenu> ChromeClient::createSearchPopupMenu(WebCore::PopupMenuClient* client) const
{
    return adoptRef(new SearchPopupMenuGtk(client));
}

#if ENABLE(VIDEO)

bool ChromeClient::supportsFullscreenForNode(const Node* node)
{
    return node->hasTagName(HTMLNames::videoTag);
}

void ChromeClient::enterFullscreenForNode(Node* node)
{
    WebCore::Frame* frame = node->document()->frame();
    WebKitWebFrame* webFrame = kit(frame);
    WebKitWebView* webView = getViewFromFrame(webFrame);
    webViewEnterFullscreen(webView, node);
}

void ChromeClient::exitFullscreenForNode(Node* node)
{
    WebCore::Frame* frame = node->document()->frame();
    WebKitWebFrame* webFrame = kit(frame);
    WebKitWebView* webView = getViewFromFrame(webFrame);
    webViewExitFullscreen(webView);
}
#endif

#if ENABLE(FULLSCREEN_API)
bool ChromeClient::supportsFullScreenForElement(const WebCore::Element* element)
{
    return true;
}

void ChromeClient::enterFullScreenForElement(WebCore::Element* element)
{
    element->document()->webkitWillEnterFullScreenForElement(element);
    element->document()->webkitDidEnterFullScreenForElement(element);
}

void ChromeClient::exitFullScreenForElement(WebCore::Element* element)
{
    element->document()->webkitWillExitFullScreenForElement(element);
    element->document()->webkitDidExitFullScreenForElement(element);
}
#endif


}
