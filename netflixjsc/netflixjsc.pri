ENV_CC=$$(CC)
ENV_CXX=$$(CXX)
ENV_CFLAGS=$$(CFLAGS)
ENV_CXXFLAGS=$$(CXXFLAGS)
ENV_LDFLAGS=$$(LDFLAGS)
!isEmpty(ENV_CC):QMAKE_CC=$$ENV_CC
!isEmpty(ENV_CXX):QMAKE_CXX=$$ENV_CXX
!isEmpty(ENV_CXXFLAGS) {
    QMAKE_CXXFLAGS=$$ENV_CXXFLAGS
    QMAKE_CXXFLAGS -= -pedantic
    QMAKE_CXXFLAGS -= -Wall
}
!isEmpty(ENV_CFLAGS) {
    QMAKE_CFLAGS=$$ENV_CFLAGS
    QMAKE_CFLAGS -= -pedantic
    QMAKE_CFLAGS -= -Wall
}
!isEmpty(ENV_LDFLAGS):QMAKE_LDFLAGS=$$ENV_LDFLAGS
INCLUDEPATH += $$PWD/ \
               $$PWD/../Source/ \
               $$PWD/../Source/JavaScriptCore/ \
               $$PWD/../Source/JavaScriptCore/API \
               $$PWD/../Source/JavaScriptCore/ForwardingHeaders \
               $$PWD/../Source/JavaScriptCore/assembler \
               $$PWD/../Source/JavaScriptCore/bytecode \
               $$PWD/../Source/JavaScriptCore/bytecompiler \
               $$PWD/../Source/JavaScriptCore/debugger \
               $$PWD/../Source/JavaScriptCore/dfg \
               $$PWD/../Source/JavaScriptCore/heap \
               $$PWD/../Source/JavaScriptCore/interpreter \
               $$PWD/../Source/JavaScriptCore/jit \
               $$PWD/../Source/JavaScriptCore/parser \
               $$PWD/../Source/JavaScriptCore/profiler \
               $$PWD/../Source/JavaScriptCore/runtime \
               $$PWD/../Source/JavaScriptCore/wtf \
               $$PWD/../Source/JavaScriptCore/wtf/unicode \
               $$PWD/../Source/JavaScriptCore/tools
DEFINES += WTF_USE_PTHREADS=1
 
