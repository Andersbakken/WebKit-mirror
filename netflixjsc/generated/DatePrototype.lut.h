// Automatically generated from ../../../../Source/JavaScriptCore/runtime/DatePrototype.cpp using /home/abakken/dev/WebKit/Source/JavaScriptCore/create_hash_table. DO NOT EDIT!

#include "Lookup.h"

namespace JSC {

static const struct HashTableValue dateTableValues[47] = {
   { "toString", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncToString), (intptr_t)0, NoIntrinsic },
   { "toISOString", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncToISOString), (intptr_t)0, NoIntrinsic },
   { "toUTCString", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncToUTCString), (intptr_t)0, NoIntrinsic },
   { "toDateString", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncToDateString), (intptr_t)0, NoIntrinsic },
   { "toTimeString", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncToTimeString), (intptr_t)0, NoIntrinsic },
   { "toLocaleString", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncToLocaleString), (intptr_t)0, NoIntrinsic },
   { "toLocaleDateString", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncToLocaleDateString), (intptr_t)0, NoIntrinsic },
   { "toLocaleTimeString", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncToLocaleTimeString), (intptr_t)0, NoIntrinsic },
   { "valueOf", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncGetTime), (intptr_t)0, NoIntrinsic },
   { "getTime", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncGetTime), (intptr_t)0, NoIntrinsic },
   { "getFullYear", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncGetFullYear), (intptr_t)0, NoIntrinsic },
   { "getUTCFullYear", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncGetUTCFullYear), (intptr_t)0, NoIntrinsic },
   { "toGMTString", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncToGMTString), (intptr_t)0, NoIntrinsic },
   { "getMonth", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncGetMonth), (intptr_t)0, NoIntrinsic },
   { "getUTCMonth", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncGetUTCMonth), (intptr_t)0, NoIntrinsic },
   { "getDate", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncGetDate), (intptr_t)0, NoIntrinsic },
   { "getUTCDate", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncGetUTCDate), (intptr_t)0, NoIntrinsic },
   { "getDay", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncGetDay), (intptr_t)0, NoIntrinsic },
   { "getUTCDay", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncGetUTCDay), (intptr_t)0, NoIntrinsic },
   { "getHours", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncGetHours), (intptr_t)0, NoIntrinsic },
   { "getUTCHours", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncGetUTCHours), (intptr_t)0, NoIntrinsic },
   { "getMinutes", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncGetMinutes), (intptr_t)0, NoIntrinsic },
   { "getUTCMinutes", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncGetUTCMinutes), (intptr_t)0, NoIntrinsic },
   { "getSeconds", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncGetSeconds), (intptr_t)0, NoIntrinsic },
   { "getUTCSeconds", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncGetUTCSeconds), (intptr_t)0, NoIntrinsic },
   { "getMilliseconds", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncGetMilliSeconds), (intptr_t)0, NoIntrinsic },
   { "getUTCMilliseconds", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncGetUTCMilliseconds), (intptr_t)0, NoIntrinsic },
   { "getTimezoneOffset", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncGetTimezoneOffset), (intptr_t)0, NoIntrinsic },
   { "setTime", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncSetTime), (intptr_t)1, NoIntrinsic },
   { "setMilliseconds", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncSetMilliSeconds), (intptr_t)1, NoIntrinsic },
   { "setUTCMilliseconds", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncSetUTCMilliseconds), (intptr_t)1, NoIntrinsic },
   { "setSeconds", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncSetSeconds), (intptr_t)2, NoIntrinsic },
   { "setUTCSeconds", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncSetUTCSeconds), (intptr_t)2, NoIntrinsic },
   { "setMinutes", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncSetMinutes), (intptr_t)3, NoIntrinsic },
   { "setUTCMinutes", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncSetUTCMinutes), (intptr_t)3, NoIntrinsic },
   { "setHours", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncSetHours), (intptr_t)4, NoIntrinsic },
   { "setUTCHours", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncSetUTCHours), (intptr_t)4, NoIntrinsic },
   { "setDate", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncSetDate), (intptr_t)1, NoIntrinsic },
   { "setUTCDate", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncSetUTCDate), (intptr_t)1, NoIntrinsic },
   { "setMonth", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncSetMonth), (intptr_t)2, NoIntrinsic },
   { "setUTCMonth", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncSetUTCMonth), (intptr_t)2, NoIntrinsic },
   { "setFullYear", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncSetFullYear), (intptr_t)3, NoIntrinsic },
   { "setUTCFullYear", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncSetUTCFullYear), (intptr_t)3, NoIntrinsic },
   { "setYear", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncSetYear), (intptr_t)1, NoIntrinsic },
   { "getYear", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncGetYear), (intptr_t)0, NoIntrinsic },
   { "toJSON", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(dateProtoFuncToJSON), (intptr_t)1, NoIntrinsic },
   { 0, 0, 0, 0, NoIntrinsic }
};

extern const struct HashTable dateTable =
    { 134, 127, dateTableValues, 0 };
} // namespace
