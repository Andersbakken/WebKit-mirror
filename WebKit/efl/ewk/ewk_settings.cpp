/*
    Copyright (C) 2009-2010 ProFUSION embedded systems
    Copyright (C) 2009-2010 Samsung Electronics

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "config.h"
#include "ewk_settings.h"

#include "EWebKit.h"
#if ENABLE(DATABASE)
#include "DatabaseTracker.h"
#endif
#include "IconDatabase.h"
#include "Image.h"
#include "IntSize.h"
#include "KURL.h"
#include "Language.h"
#include "ewk_private.h"

#include <Eina.h>
#include <eina_safety_checks.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <wtf/text/CString.h>
#include <wtf/text/StringConcatenate.h>

#if USE(SOUP)
#include "ResourceHandle.h"
#include <libsoup/soup.h>
#endif

static const char* _ewk_default_web_database_path = 0;
static const char* _ewk_icon_database_path = 0;
static uint64_t _ewk_default_web_database_quota = 1 * 1024 * 1024;

static WTF::String _ewk_settings_webkit_platform_get()
{
    WTF::String ua_platform;
#if PLATFORM(X11)
    ua_platform = "X11";
#else
    ua_platform = "Unknown";
#endif
    return ua_platform;
}

static WTF::String _ewk_settings_webkit_os_version_get()
{
    WTF::String ua_os_version;
    struct utsname name;

    if (uname(&name) != -1)
        ua_os_version = WTF::String(name.sysname) + " " + WTF::String(name.machine);
    else
        ua_os_version = "Unknown";

    return ua_os_version;
}

/**
 * Returns the default quota for Web Database databases. By default
 * this value is 1MB.
 *
 * @return the current default database quota in bytes
 **/
uint64_t ewk_settings_web_database_default_quota_get()
{
    return _ewk_default_web_database_quota;
}

/**
 * Sets the current path to the directory WebKit will write Web
 * Database databases.
 *
 * @path: the new database directory path
 *
 */
void ewk_settings_web_database_path_set(const char *path)
{
#if ENABLE(DATABASE)
    WTF::String corePath = WTF::String::fromUTF8(path);
    WebCore::DatabaseTracker::tracker().setDatabaseDirectoryPath(corePath);
    if (!_ewk_default_web_database_path)
        _ewk_default_web_database_path = eina_stringshare_add(corePath.utf8().data());
    else
        eina_stringshare_replace(&_ewk_default_web_database_path, corePath.utf8().data());

#endif
}

/**
 * Return directory path where web database is stored.
 *
 * @return database path or NULL if none or web database is not supported.
 *         This is guaranteed to be eina_stringshare, so whenever possible
 *         save yourself some cpu cycles and use
 *         eina_stringshare_ref() instead of eina_stringshare_add() or
 *         strdup().
 */
const char *ewk_settings_web_database_path_get()
{
#if ENABLE(DATABASE)
    return _ewk_default_web_database_path;
#else
    return 0;
#endif
}

/**
 * Sets directory where to store icon database, opening database.
 *
 * @param directory where to store icon database, must be
 *        write-able. If @c NULL is given, then database is closed.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on errors.
 */
Eina_Bool ewk_settings_icon_database_path_set(const char *directory)
{
    WebCore::iconDatabase()->delayDatabaseCleanup();

    if (directory) {
        struct stat st;

        if (stat(directory, &st)) {
            ERR("could not stat(%s): %s", directory, strerror(errno));
            return EINA_FALSE;
        }

        if (!S_ISDIR(st.st_mode)) {
            ERR("not a directory: %s", directory);
            return EINA_FALSE;
        }

        if (access(directory, R_OK | W_OK)) {
            ERR("could not access directory '%s' for read and write: %s",
                directory, strerror(errno));
            return EINA_FALSE;
        }

        WebCore::iconDatabase()->setEnabled(true);
        WebCore::iconDatabase()->open(WTF::String::fromUTF8(directory));
        if (!_ewk_icon_database_path)
            _ewk_icon_database_path = eina_stringshare_add(directory);
        else
            eina_stringshare_replace(&_ewk_icon_database_path, directory);
    } else {
        WebCore::iconDatabase()->setEnabled(false);
        WebCore::iconDatabase()->close();
        if (_ewk_icon_database_path) {
            eina_stringshare_del(_ewk_icon_database_path);
            _ewk_icon_database_path = 0;
        }
    }
    return EINA_TRUE;
}

/**
 * Return directory path where icon database is stored.
 *
 * @return database path or @c NULL if none is set or database is closed.
 *         This is guaranteed to be eina_stringshare, so whenever possible
 *         save yourself some cpu cycles and use
 *         eina_stringshare_ref() instead of eina_stringshare_add() or
 *         strdup().
 */
const char* ewk_settings_icon_database_path_get(void)
{
    if (!WebCore::iconDatabase()->isEnabled())
        return 0;
    if (!WebCore::iconDatabase()->isOpen())
        return 0;

    return _ewk_icon_database_path;
}

/**
 * Remove all known icons from database.
 *
 * Database must be opened with ewk_settings_icon_database_path_set()
 * in order to work.
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE otherwise, like
 *         closed database.
 */
Eina_Bool ewk_settings_icon_database_clear(void)
{
    if (!WebCore::iconDatabase()->isEnabled())
        return EINA_FALSE;
    if (!WebCore::iconDatabase()->isOpen())
        return EINA_FALSE;

    WebCore::iconDatabase()->removeAllIcons();
    return EINA_TRUE;
}

/**
 * Query icon for given URL, returning associated cairo surface.
 *
 * @note in order to have this working, one must open icon database
 *       with ewk_settings_icon_database_path_set().
 *
 * @param url which url to query icon.
 *
 * @return cairo surface if any, or NULL on failure.
 */
cairo_surface_t* ewk_settings_icon_database_icon_surface_get(const char *url)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(url, 0);

    WebCore::KURL kurl(WebCore::KURL(), WTF::String::fromUTF8(url));
    WebCore::Image *icon = WebCore::iconDatabase()->iconForPageURL(kurl.string(), WebCore::IntSize(16, 16));

    if (!icon) {
        ERR("no icon for url %s", url);
        return 0;
    }

    return icon->nativeImageForCurrentFrame();
}

/**
 * Create Evas_Object of type image representing the given URL.
 *
 * This is an utility function that creates an Evas_Object of type
 * image set to have fill always match object size
 * (evas_object_image_filled_add()), saving some code to use it from Evas.
 *
 * @note in order to have this working, one must open icon database
 *       with ewk_settings_icon_database_path_set().
 *
 * @param url which url to query icon.
 * @param canvas evas instance where to add resulting object.
 *
 * @return newly allocated Evas_Object instance or @c NULL on
 *         errors. Delete the object with evas_object_del().
 */
Evas_Object* ewk_settings_icon_database_icon_object_add(const char* url, Evas* canvas)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(url, 0);
    EINA_SAFETY_ON_NULL_RETURN_VAL(canvas, 0);

    WebCore::KURL kurl(WebCore::KURL(), WTF::String::fromUTF8(url));
    WebCore::Image* icon = WebCore::iconDatabase()->iconForPageURL(kurl.string(), WebCore::IntSize(16, 16));
    cairo_surface_t* surface;

    if (!icon) {
        ERR("no icon for url %s", url);
        return 0;
    }

    surface = icon->nativeImageForCurrentFrame();
    return ewk_util_image_from_cairo_surface_add(canvas, surface);
}

/**
 * Sets the given proxy URI to network backend.
 *
 * @param proxy URI.
 */
void ewk_settings_proxy_uri_set(const char* proxy)
{
#if USE(SOUP)
    SoupURI* uri = soup_uri_new(proxy);
    EINA_SAFETY_ON_NULL_RETURN(uri);

    SoupSession* session = WebCore::ResourceHandle::defaultSession();
    g_object_set(session, SOUP_SESSION_PROXY_URI, uri, NULL);
    soup_uri_free(uri);
#elif USE(CURL)
    EINA_SAFETY_ON_TRUE_RETURN(1);
#endif
}

/**
* @internal
* Gets the default user agent string.
*
* @return A pointer to an eina_stringshare containing the user agent string.
*/
const char* ewk_settings_default_user_agent_get()
{
    WTF::String ua_version = makeString(String::number(WEBKIT_USER_AGENT_MAJOR_VERSION), '.', String::number(WEBKIT_USER_AGENT_MINOR_VERSION), '+');
    WTF::String static_ua = makeString("Mozilla/5.0 (", _ewk_settings_webkit_platform_get(), "; U; ", _ewk_settings_webkit_os_version_get(), "; ", WebCore::defaultLanguage(), ") AppleWebKit/", ua_version) + makeString(" (KHTML, like Gecko) Version/5.0 Safari/", ua_version);

    return eina_stringshare_add(static_ua.utf8().data());
} 
