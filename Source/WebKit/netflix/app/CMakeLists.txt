add_executable(app_netflix main.cpp)
include_directories( ${WEBKIT_ROOTDIR} ${JAVASCRIPTCORE_DIR}/wtf ${JAVASCRIPTCORE_DIR}/runtime ${JAVASCRIPTCORE_DIR} ${WEBCORE_DIR}/platform/text ${WEBKIT_DIR}/netflix ${WEBCORE_DIR}/platform/graphics ${WEBCORE_DIR}/platform/animation ${WEBCORE_DIR}/platform )
target_link_libraries(app_netflix ${WebKit_LIBRARY_NAME} -lcairo -lfreetype -lfontconfig)
