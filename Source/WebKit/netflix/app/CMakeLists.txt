
# Options for QT
set( QTDIR "${TOOLCHAIN_DIRECTORY}/netflix/qt" CACHE PATH "QTAPP: Main Qt Directory. Default=${TOOLCHAIN_DIRECTORY}/netflix/qt" )
if( QTDIR )
    message( STATUS " - Qt base directory : ${QTDIR}" )
else()
    set( QTDIR "${TOOLCHAIN_DIRECTORY}/netflix/qt" )
endif()

set ( QT_QMAKE_EXECUTABLE ${QTDIR}/bin/qmake )

message( STATUS " Looking for Qt in ${QT_QMAKE_EXECUTABLE}" )

set( CMAKE_PREFIX_PATH ${QTDIR} )

find_package ( Qt4 REQUIRED )
include ( ${QT_USE_FILE} )


add_executable(app_netflix main.cpp)
include_directories( ${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR} )
include_directories( ${WEBKIT_ROOTDIR} ${JAVASCRIPTCORE_DIR}/wtf ${JAVASCRIPTCORE_DIR} ${WEBCORE_DIR}/platform/text ${WEBKIT_DIR}/netflix ${WEBCORE_DIR}/platform/graphics ${WEBCORE_DIR}/platform )
target_link_libraries(app_netflix ${WebKit_LIBRARY_NAME} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} -lcairo -lfreetype -lfontconfig)
