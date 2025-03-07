SET(PROJECT_VERSION_MAJOR 0)
SET(PROJECT_VERSION_MINOR 1)
SET(PROJECT_VERSION_PATCH 0)
SET(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

# -----------------------------------------------------------------------------
# We mention Safari version because many sites check for it.
# Sync with WebCore/Configurations/Version.xcconfig whenever Safari is version 
# up.
# -----------------------------------------------------------------------------
SET(USER_AGENT_VERSION_MAJOR 534)
SET(USER_AGENT_VERSION_MINOR 16)

ADD_DEFINITIONS(-DWTF_PLATFORM_EFL=1)
SET(WTF_PLATFORM_EFL 1)

# -----------------------------------------------------------------------------
# Determine which network backend will be used
# -----------------------------------------------------------------------------
SET(ALL_NETWORK_BACKENDS soup curl)
SET(NETWORK_BACKEND "soup" CACHE STRING "choose which network backend to use (one of ${ALL_NETWORK_BACKENDS})")

# -----------------------------------------------------------------------------
# Determine which font backend will be used
# -----------------------------------------------------------------------------
SET(ALL_FONT_BACKENDS freetype pango)
SET(FONT_BACKEND "freetype" CACHE STRING "choose which network backend to use (one of ${ALL_FONT_BACKENDS})")

FIND_PACKAGE(Cairo 1.6 REQUIRED)
FIND_PACKAGE(EFL REQUIRED)
FIND_PACKAGE(Fontconfig 2.8.0 REQUIRED)
FIND_PACKAGE(Sqlite REQUIRED)
FIND_PACKAGE(LibXml2 2.6 REQUIRED)
FIND_PACKAGE(LibXslt 1.1.7 REQUIRED)
FIND_PACKAGE(ICU REQUIRED)
FIND_PACKAGE(Threads REQUIRED)
FIND_PACKAGE(JPEG REQUIRED)
FIND_PACKAGE(PNG REQUIRED)

ADD_DEFINITIONS(-DENABLE_CONTEXT_MENUS=0)

SET(WTF_USE_PTHREADS 1)
ADD_DEFINITIONS(-DWTF_USE_PTHREADS=1)

SET(WTF_USE_ICU_UNICODE 1)
ADD_DEFINITIONS(-DWTF_USE_ICU_UNICODE=1)
LIST(APPEND WTF_INCLUDE_DIRECTORIES ${ICU_INCLUDE_DIRS})

SET(WTF_PLATFORM_CAIRO 1)
ADD_DEFINITIONS(-DWTF_PLATFORM_CAIRO=1)

SET(JSC_EXECUTABLE_NAME jsc_efl)
SET(WTF_LIBRARY_NAME wtf_efl)
SET(JavaScriptCore_LIBRARY_NAME javascriptcore_efl)
SET(WebCore_LIBRARY_NAME webcore_efl)
SET(WebKit_LIBRARY_NAME ewebkit)

SET(DATA_DIR ${CMAKE_INSTALL_PREFIX}/share/${WebKit_LIBRARY_NAME}-${PROJECT_VERSION_MAJOR})
ADD_DEFINITIONS(-DDATA_DIR="${DATA_DIR}")

SET(VERSION_SCRIPT "-Wl,--version-script,${CMAKE_MODULE_PATH}/eflsymbols.filter")

WEBKIT_FEATURE(ENABLE_AS_IMAGE "Enable SVG as image" DEFAULT ON SVG)
WEBKIT_FEATURE(ENABLE_BLOB "Enable blob slice" DEFAULT OFF)
WEBKIT_FEATURE(ENABLE_CHANNEL_MESSAGING "Enable channel messaging" DEFAULT ON)
WEBKIT_FEATURE(ENABLE_DATABASE "Enable database" DEFAULT ON)
WEBKIT_FEATURE(ENABLE_DATAGRID "Enable datagrid" DEFAULT OFF)
WEBKIT_FEATURE(ENABLE_DATALIST "Enable datalist" DEFAULT ON HTML)
WEBKIT_FEATURE(ENABLE_DOM_STORAGE "Enable DOM storage" DEFAULT ON)
WEBKIT_FEATURE(ENABLE_EVENTSOURCE "Enable event source" DEFAULT ON)
WEBKIT_FEATURE(ENABLE_FAST_MALLOC "Enable TCmalloc instead of system's allocator" DEFAULT ON)
WEBKIT_FEATURE(ENABLE_FAST_MOBILE_SCROLLING "Enable fast mobile scrolling" DEFAULT ON)
WEBKIT_FEATURE(ENABLE_FILTERS "Enable SVG filters" DEFAULT ON SVG)
WEBKIT_FEATURE(ENABLE_FTPDIR "Enable FTP directory support" DEFAULT ON)
WEBKIT_FEATURE(ENABLE_GEOLOCATION "Enable geolocation" DEFAULT OFF)
WEBKIT_FEATURE(ENABLE_GLIB_SUPPORT "Enable Glib support" DEFAULT ON)
WEBKIT_FEATURE(ENABLE_ICONDATABASE "Enable icon database" DEFAULT ON)
WEBKIT_FEATURE(ENABLE_INSPECTOR "Enable inspector" DEFAULT ON)
WEBKIT_FEATURE(ENABLE_JAVASCRIPT_DEBUGGER "Enable JavaScript debugger" DEFAULT ON)
WEBKIT_FEATURE(ENABLE_JIT "Enable JIT code" DEFAULT OFF)
WEBKIT_FEATURE(ENABLE_MATHML "Enable MathML" DEFAULT ON)
WEBKIT_FEATURE(ENABLE_NETSCAPE_PLUGIN_API "Enable Netscape plugin API" DEFAULT OFF)
WEBKIT_FEATURE(ENABLE_NOTIFICATIONS "Enable notifications" DEFAULT OFF)
WEBKIT_FEATURE(ENABLE_OFFLINE_WEB_APPLICATIONS "Enable offline web applications" DEFAULT ON)
WEBKIT_FEATURE(ENABLE_ORIENTATION_EVENTS "Enable orientation events" DEFAULT OFF)
WEBKIT_FEATURE(ENABLE_PROGRESS_TAG "Enable progress tag" DEFAULT OFF)
WEBKIT_FEATURE(ENABLE_SHARED_WORKERS "Enable shared workers" DEFAULT ON)
WEBKIT_FEATURE(ENABLE_SVG "Enable SVG" DEFAULT ON)
WEBKIT_FEATURE(ENABLE_SVG_ANIMATION "Enable SVG animation" DEFAULT ON SVG)
WEBKIT_FEATURE(ENABLE_SVG_FONTS "Enable SVG fonts" DEFAULT ON SVG)
WEBKIT_FEATURE(ENABLE_SVG_FOREIGN_OBJECT "Enable SVG foreign object" DEFAULT ON SVG)
WEBKIT_FEATURE(ENABLE_SVG_USE "Enable SVG use" DEFAULT ON SVG)
WEBKIT_FEATURE(ENABLE_TOUCH_EVENTS "Enable Touch Events" DEFAULT OFF)
WEBKIT_FEATURE(ENABLE_VIDEO "Enable video" DEFAULT ON HTML)
WEBKIT_FEATURE(ENABLE_WEB_SOCKETS "Enable web sockets" DEFAULT OFF)
WEBKIT_FEATURE(ENABLE_WML "Enable WML" DEFAULT OFF)
WEBKIT_FEATURE(ENABLE_WORKERS "Enable workers" DEFAULT ON)
WEBKIT_FEATURE(ENABLE_XHTMLMP "Enable XHTMLMP" DEFAULT OFF)
WEBKIT_FEATURE(ENABLE_XPATH "Enable XPath" DEFAULT ON)
WEBKIT_FEATURE(ENABLE_XSLT "Enable XSLT" DEFAULT ON)

OPTION(ENABLE_ECORE_X "Enable Ecore_X specific usage (cursor, bell)" ON)
IF (ENABLE_ECORE_X)
    IF (ECORE_X_FOUND)
        MESSAGE(STATUS "Using Ecore-X to provide extended support.")
        ADD_DEFINITIONS(-DHAVE_ECORE_X)
    ELSE ()
        MESSAGE(ERROR "Requested Ecore-X but it was not found!")
    ENDIF ()
ENDIF ()

IF (NETWORK_BACKEND STREQUAL "soup")
  IF (NOT ENABLE_GLIB_SUPPORT)
    SET(ENABLE_GLIB_SUPPORT 1)
    MESSAGE("Forcing Glib support")
  ENDIF ()
  FIND_PACKAGE(LibSoup2 2.28.2 REQUIRED)
  SET(WTF_USE_SOUP 1)
  ADD_DEFINITIONS(-DWTF_USE_SOUP=1)

  IF (LIBSOUP24_VERSION VERSION_LESS 2.29.90)
  ELSE (LIBSOUP24_VERSION VERSION_LESS 2.29.90)
      SET(HAVE_LIBSOUP_2_29_90 1)
      ADD_DEFINITIONS(-DHAVE_LIBSOUP_2_29_90=1)
  ENDIF ()
ELSE ()
  FIND_PACKAGE(CURL REQUIRED)
  SET(WTF_USE_CURL 1)
  ADD_DEFINITIONS(-DWTF_USE_CURL=1)
ENDIF ()

IF (FONT_BACKEND STREQUAL "freetype")
  FIND_PACKAGE(Freetype 9.0 REQUIRED)
  SET(WTF_USE_FREETYPE 1)
  ADD_DEFINITIONS(-DUSE_FREETYPE=1)
ELSE ()
  FIND_PACKAGE(Pango REQUIRED)
  SET(WTF_USE_PANGO 1)
  ADD_DEFINITIONS(-DUSE_PANGO=1)

  IF (NOT ENABLE_GLIB_SUPPORT)
    SET(ENABLE_GLIB_SUPPORT 1)
    MESSAGE("Forcing Glib support")
  ENDIF ()
ENDIF ()

IF (ENABLE_VIDEO)
  SET(ENABLE_GLIB_SUPPORT 1)
  MESSAGE("Forcing Glib support")
ENDIF()

IF (ENABLE_GLIB_SUPPORT)
  FIND_PACKAGE(GDK 2.10 REQUIRED)
  FIND_PACKAGE(Glib REQUIRED)
  FIND_PACKAGE(Gthread REQUIRED)
ENDIF ()

IF (ENABLE_VIDEO)
  FIND_PACKAGE(GStreamer REQUIRED)
  FIND_PACKAGE(GStreamer-App REQUIRED)
  FIND_PACKAGE(GStreamer-Base REQUIRED)
  FIND_PACKAGE(GStreamer-Pbutils REQUIRED)
  FIND_PACKAGE(GStreamer-Interfaces REQUIRED)
  FIND_PACKAGE(GStreamer-Plugins-Base REQUIRED)
  FIND_PACKAGE(GStreamer-Video REQUIRED)
  SET(WTF_USE_GSTREAMER 1)
  ADD_DEFINITIONS(-DWTF_USE_GSTREAMER=1)
ENDIF()

SET(CPACK_SOURCE_GENERATOR TBZ2)
