
IF (ENABLE_FAST_MALLOC)
  LIST(APPEND WTF_SOURCES
    TCSystemAlloc.cpp
  )
ELSE ()
  ADD_DEFINITIONS(-DUSE_SYSTEM_MALLOC=1)
ENDIF()

LIST(APPEND WTF_SOURCES
    netflix/MainThreadNetflix.cpp
    netflix/OwnPtrNetflix.cpp

    OSAllocatorPosix.cpp
    ThreadIdentifierDataPthreads.cpp
    ThreadingPthreads.cpp

    unicode/icu/CollatorICU.cpp
)

LIST(APPEND WTF_LIBRARIES
    pthread
    ${ICU_LIBRARIES}
)

LIST(APPEND WTF_INCLUDE_DIRECTORIES
    ${ICU_INCLUDE_DIRS}
    ${JAVASCRIPTCORE_DIR}/wtf/unicode/
)
