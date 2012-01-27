// Automatically generated from ../../../../Source/JavaScriptCore/runtime/DateConstructor.cpp using /home/abakken/dev/WebKit/Source/JavaScriptCore/create_hash_table. DO NOT EDIT!

#include "Lookup.h"

namespace JSC {

static const struct HashTableValue dateConstructorTableValues[4] = {
   { "parse", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateParse), (intptr_t)1, NoIntrinsic },
   { "UTC", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateUTC), (intptr_t)7, NoIntrinsic },
   { "now", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateNow), (intptr_t)0, NoIntrinsic },
   { 0, 0, 0, 0, NoIntrinsic }
};

extern const struct HashTable dateConstructorTable =
    { 9, 7, dateConstructorTableValues, 0 };
} // namespace
