TEMPLATE = subdirs
CONFIG += ordered

include(WebKit.pri)

!v8: SUBDIRS += JavaScriptCore
webkit2 {
    SUBDIRS += WebKit2
}
SUBDIRS += WebCore

# If the source exists, built it
exists($$PWD/Tools/QtTestBrowser): SUBDIRS += Tools/QtTestBrowser
contains(QT_CONFIG, declarative) {
    exists($$PWD/WebKit/qt/declarative): SUBDIRS += WebKit/qt/declarative
}
!v8:exists($$PWD/JavaScriptCore/jsc.pro): SUBDIRS += JavaScriptCore/jsc.pro
exists($$PWD/WebKit/qt/tests): SUBDIRS += WebKit/qt/tests
exists($$PWD/Tools/DumpRenderTree/qt/DumpRenderTree.pro): SUBDIRS += Tools/DumpRenderTree/qt/DumpRenderTree.pro
exists($$PWD/Tools/DumpRenderTree/qt/ImageDiff.pro): SUBDIRS += Tools/DumpRenderTree/qt/ImageDiff.pro

!win32:!symbian {
    exists($$PWD/Tools/DumpRenderTree/qt/TestNetscapePlugin/TestNetscapePlugin.pro): SUBDIRS += Tools/DumpRenderTree/qt/TestNetscapePlugin/TestNetscapePlugin.pro
}

build-qtscript {
    SUBDIRS += \
        JavaScriptCore/qt/api/QtScript.pro \
        JavaScriptCore/qt/tests \
        JavaScriptCore/qt/benchmarks
}

webkit2 {
    exists($$PWD/WebKit2/WebProcess.pro): SUBDIRS += WebKit2/WebProcess.pro
    exists($$PWD/Tools/MiniBrowser/qt/MiniBrowser.pro): SUBDIRS += Tools/MiniBrowser/qt/MiniBrowser.pro
    exists($$PWD/Tools/WebKitTestRunner/WebKitTestRunner.pro): SUBDIRS += Tools/WebKitTestRunner/WebKitTestRunner.pro
}

symbian {
    # Forward the install target to WebCore. A workaround since INSTALLS is not implemented for symbian
    install.commands = $(MAKE) -C WebCore install
    QMAKE_EXTRA_TARGETS += install
}

include(WebKit/qt/docs/docs.pri)
