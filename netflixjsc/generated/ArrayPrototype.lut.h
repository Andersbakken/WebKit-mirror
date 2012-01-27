// Automatically generated from ../../../../Source/JavaScriptCore/runtime/ArrayPrototype.cpp using /home/abakken/dev/WebKit/Source/JavaScriptCore/create_hash_table. DO NOT EDIT!

#include "Lookup.h"

namespace JSC {

static const struct HashTableValue arrayPrototypeTableValues[22] = {
   { "toString", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncToString), (intptr_t)0, NoIntrinsic },
   { "toLocaleString", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncToLocaleString), (intptr_t)0, NoIntrinsic },
   { "concat", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncConcat), (intptr_t)1, NoIntrinsic },
   { "join", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncJoin), (intptr_t)1, NoIntrinsic },
   { "pop", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncPop), (intptr_t)0, ArrayPopIntrinsic },
   { "push", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncPush), (intptr_t)1, ArrayPushIntrinsic },
   { "reverse", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncReverse), (intptr_t)0, NoIntrinsic },
   { "shift", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncShift), (intptr_t)0, NoIntrinsic },
   { "slice", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncSlice), (intptr_t)2, NoIntrinsic },
   { "sort", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncSort), (intptr_t)1, NoIntrinsic },
   { "splice", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncSplice), (intptr_t)2, NoIntrinsic },
   { "unshift", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncUnShift), (intptr_t)1, NoIntrinsic },
   { "every", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncEvery), (intptr_t)1, NoIntrinsic },
   { "forEach", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncForEach), (intptr_t)1, NoIntrinsic },
   { "some", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncSome), (intptr_t)1, NoIntrinsic },
   { "indexOf", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncIndexOf), (intptr_t)1, NoIntrinsic },
   { "lastIndexOf", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncLastIndexOf), (intptr_t)1, NoIntrinsic },
   { "filter", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncFilter), (intptr_t)1, NoIntrinsic },
   { "reduce", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncReduce), (intptr_t)1, NoIntrinsic },
   { "reduceRight", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncReduceRight), (intptr_t)1, NoIntrinsic },
   { "map", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncMap), (intptr_t)1, NoIntrinsic },
   { 0, 0, 0, 0, NoIntrinsic }
};

extern const struct HashTable arrayPrototypeTable =
    { 65, 63, arrayPrototypeTableValues, 0 };
} // namespace
