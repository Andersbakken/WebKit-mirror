// Automatically generated from ../../../../Source/JavaScriptCore/runtime/RegExpPrototype.cpp using /home/abakken/dev/WebKit/Source/JavaScriptCore/create_hash_table. DO NOT EDIT!

#include "Lookup.h"

namespace JSC {

static const struct HashTableValue regExpPrototypeTableValues[5] = {
   { "compile", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(regExpProtoFuncCompile), (intptr_t)2, NoIntrinsic },
   { "exec", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(regExpProtoFuncExec), (intptr_t)1, NoIntrinsic },
   { "test", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(regExpProtoFuncTest), (intptr_t)1, NoIntrinsic },
   { "toString", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(regExpProtoFuncToString), (intptr_t)0, NoIntrinsic },
   { 0, 0, 0, 0, NoIntrinsic }
};

extern const struct HashTable regExpPrototypeTable =
    { 9, 7, regExpPrototypeTableValues, 0 };
} // namespace
