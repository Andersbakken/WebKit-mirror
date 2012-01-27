// Automatically generated from ../../../../Source/JavaScriptCore/runtime/JSGlobalObject.cpp using /home/abakken/dev/WebKit/Source/JavaScriptCore/create_hash_table. DO NOT EDIT!

#include "Lookup.h"

namespace JSC {

static const struct HashTableValue globalObjectTableValues[11] = {
   { "parseInt", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(globalFuncParseInt), (intptr_t)2, NoIntrinsic },
   { "parseFloat", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(globalFuncParseFloat), (intptr_t)1, NoIntrinsic },
   { "isNaN", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(globalFuncIsNaN), (intptr_t)1, NoIntrinsic },
   { "isFinite", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(globalFuncIsFinite), (intptr_t)1, NoIntrinsic },
   { "escape", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(globalFuncEscape), (intptr_t)1, NoIntrinsic },
   { "unescape", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(globalFuncUnescape), (intptr_t)1, NoIntrinsic },
   { "decodeURI", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(globalFuncDecodeURI), (intptr_t)1, NoIntrinsic },
   { "decodeURIComponent", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(globalFuncDecodeURIComponent), (intptr_t)1, NoIntrinsic },
   { "encodeURI", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(globalFuncEncodeURI), (intptr_t)1, NoIntrinsic },
   { "encodeURIComponent", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(globalFuncEncodeURIComponent), (intptr_t)1, NoIntrinsic },
   { 0, 0, 0, 0, NoIntrinsic }
};

extern const struct HashTable globalObjectTable =
    { 34, 31, globalObjectTableValues, 0 };
} // namespace
