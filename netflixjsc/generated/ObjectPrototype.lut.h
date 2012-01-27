// Automatically generated from ../../../../Source/JavaScriptCore/runtime/ObjectPrototype.cpp using /home/abakken/dev/WebKit/Source/JavaScriptCore/create_hash_table. DO NOT EDIT!

#include "Lookup.h"

namespace JSC {

static const struct HashTableValue objectPrototypeTableValues[11] = {
   { "toString", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(objectProtoFuncToString), (intptr_t)0, NoIntrinsic },
   { "toLocaleString", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(objectProtoFuncToLocaleString), (intptr_t)0, NoIntrinsic },
   { "valueOf", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(objectProtoFuncValueOf), (intptr_t)0, NoIntrinsic },
   { "hasOwnProperty", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(objectProtoFuncHasOwnProperty), (intptr_t)1, NoIntrinsic },
   { "propertyIsEnumerable", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(objectProtoFuncPropertyIsEnumerable), (intptr_t)1, NoIntrinsic },
   { "isPrototypeOf", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(objectProtoFuncIsPrototypeOf), (intptr_t)1, NoIntrinsic },
   { "__defineGetter__", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(objectProtoFuncDefineGetter), (intptr_t)2, NoIntrinsic },
   { "__defineSetter__", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(objectProtoFuncDefineSetter), (intptr_t)2, NoIntrinsic },
   { "__lookupGetter__", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(objectProtoFuncLookupGetter), (intptr_t)1, NoIntrinsic },
   { "__lookupSetter__", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(objectProtoFuncLookupSetter), (intptr_t)1, NoIntrinsic },
   { 0, 0, 0, 0, NoIntrinsic }
};

extern const struct HashTable objectPrototypeTable =
    { 32, 31, objectPrototypeTableValues, 0 };
} // namespace
