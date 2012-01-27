// Automatically generated from ../../../../Source/JavaScriptCore/runtime/MathObject.cpp using /home/abakken/dev/WebKit/Source/JavaScriptCore/create_hash_table. DO NOT EDIT!

#include "Lookup.h"

namespace JSC {

static const struct HashTableValue mathTableValues[19] = {
   { "abs", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncAbs), (intptr_t)1, AbsIntrinsic },
   { "acos", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncACos), (intptr_t)1, NoIntrinsic },
   { "asin", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncASin), (intptr_t)1, NoIntrinsic },
   { "atan", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncATan), (intptr_t)1, NoIntrinsic },
   { "atan2", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncATan2), (intptr_t)2, NoIntrinsic },
   { "ceil", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncCeil), (intptr_t)1, CeilIntrinsic },
   { "cos", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncCos), (intptr_t)1, NoIntrinsic },
   { "exp", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncExp), (intptr_t)1, ExpIntrinsic },
   { "floor", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncFloor), (intptr_t)1, FloorIntrinsic },
   { "log", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncLog), (intptr_t)1, LogIntrinsic },
   { "max", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncMax), (intptr_t)2, MaxIntrinsic },
   { "min", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncMin), (intptr_t)2, MinIntrinsic },
   { "pow", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncPow), (intptr_t)2, PowIntrinsic },
   { "random", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncRandom), (intptr_t)0, NoIntrinsic },
   { "round", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncRound), (intptr_t)1, RoundIntrinsic },
   { "sin", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncSin), (intptr_t)1, NoIntrinsic },
   { "sqrt", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncSqrt), (intptr_t)1, SqrtIntrinsic },
   { "tan", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncTan), (intptr_t)1, NoIntrinsic },
   { 0, 0, 0, 0, NoIntrinsic }
};

extern const struct HashTable mathTable =
    { 67, 63, mathTableValues, 0 };
} // namespace
