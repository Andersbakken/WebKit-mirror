SET(WTF_HEADERS
    ASCIICType.h
    AVLTree.h
    AlwaysInline.h
    Assertions.h
    Atomics.h
    Bitmap.h
    BumpPointerAllocator.h
    ByteArray.h
    Complex.h
    CrossThreadRefCounted.h
    CurrentTime.h
    DateMath.h
    DecimalNumber.h
    Deque.h
    DisallowCType.h
    FastAllocBase.h
    FastMalloc.h
    FixedArray.h
    Forward.h
    GetPtr.h
    HashCountedSet.h
    HashFunctions.h
    HashIterators.h
    HashMap.h
    HashSet.h
    HashTable.h
    HashTraits.h
    ListHashSet.h
    ListRefPtr.h
    Locker.h
    MD5.h
    MainThread.h
    MallocZoneSupport.h
    MathExtras.h
    MessageQueue.h
    NonCopyingSort.h
    Noncopyable.h
    NotFound.h
    NullPtr.h
    OSAllocator.h
    OwnArrayPtr.h
    OwnArrayPtrCommon.h
    OwnFastMallocPtr.h
    OwnPtr.h
    OwnPtrCommon.h
    PageAllocation.h
    PageAllocationAligned.h
    PageBlock.h
    PageReservation.h
    PassOwnArrayPtr.h
    PassOwnPtr.h
    PassRefPtr.h
    Platform.h
    PlatformRefPtr.h
    PossiblyNull.h
    RandomNumber.h
    RandomNumberSeed.h
    RefCounted.h
    RefCountedLeakCounter.h
    RefPtr.h
    RefPtrHashMap.h
    RetainPtr.h
    SegmentedVector.h
    StackBounds.h
    StaticConstructors.h
    StdLibExtras.h
    StringExtras.h
    StringHasher.h
    TCPackedCache.h
    TCPageMap.h
    TCSpinLock.h
    TCSystemAlloc.h
    ThreadIdentifierDataPthreads.h
    ThreadSafeShared.h
    ThreadSpecific.h
    Threading.h
    ThreadingPrimitives.h
    TypeTraits.h
    UnusedParam.h
    VMTags.h
    ValueCheck.h
    Vector.h
    VectorTraits.h
    WTFThreadData.h
    dtoa.h

    text/AtomicString.h
    text/AtomicStringImpl.h
    text/CString.h
    text/StringBuffer.h
    text/StringHash.h
    text/StringImpl.h
    text/StringImplBase.h
    text/WTFString.h

    unicode/Collator.h
    unicode/UTF8.h
    unicode/Unicode.h
)

SET(WTF_SOURCES
    Assertions.cpp
    ByteArray.cpp
    CurrentTime.cpp
    DecimalNumber.cpp
    FastMalloc.cpp
    HashTable.cpp
    MainThread.cpp
    MD5.cpp
    RandomNumber.cpp
    RefCountedLeakCounter.cpp
    StackBounds.cpp
    StringExtras.cpp
    Threading.cpp
    TypeTraits.cpp
    WTFThreadData.cpp
    dtoa.cpp

    text/AtomicString.cpp
    text/CString.cpp
    text/StringBuilder.cpp
    text/StringImpl.cpp
    text/StringStatics.cpp
    text/WTFString.cpp

    unicode/UTF8.cpp
)

SET(WTF_LIBRARIES
)

SET(WTF_PORT_FLAGS )
INCLUDE_IF_EXISTS(${JAVASCRIPTCORE_DIR}/wtf/CMakeLists${PORT}.txt)

LIST(APPEND WTF_INCLUDE_DIRECTORIES
    "${CMAKE_BINARY_DIR}"
)

WEBKIT_WRAP_SOURCELIST(${WTF_SOURCES})
INCLUDE_DIRECTORIES(${WTF_INCLUDE_DIRECTORIES})
ADD_DEFINITIONS(-DBUILDING_WTF)
ADD_LIBRARY(${WTF_LIBRARY_NAME} ${WTF_LIBRARY_TYPE} ${WTF_HEADERS} ${WTF_SOURCES})
TARGET_LINK_LIBRARIES(${WTF_LIBRARY_NAME} ${WTF_LIBRARIES})

IF (WTF_LINK_FLAGS)
    ADD_TARGET_PROPERTIES(${WTF_LIBRARY_NAME} LINK_FLAGS "${WTF_LINK_FLAGS}")
ENDIF ()

IF (SHARED_CORE)
    SET_TARGET_PROPERTIES(${WTF_LIBRARY_NAME} PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})
    INSTALL(TARGETS ${WTF_LIBRARY_NAME} DESTINATION lib)
ENDIF ()
