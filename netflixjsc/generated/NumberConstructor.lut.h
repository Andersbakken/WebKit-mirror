// Automatically generated from ../../../../Source/JavaScriptCore/runtime/NumberConstructor.cpp using /home/abakken/dev/WebKit/Source/JavaScriptCore/create_hash_table. DO NOT EDIT!

#include "Lookup.h"

namespace JSC {

static const struct HashTableValue numberConstructorTableValues[6] = {
   { "NaN", DontEnum|DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(numberConstructorNaNValue), (intptr_t)0, NoIntrinsic },
   { "NEGATIVE_INFINITY", DontEnum|DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(numberConstructorNegInfinity), (intptr_t)0, NoIntrinsic },
   { "POSITIVE_INFINITY", DontEnum|DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(numberConstructorPosInfinity), (intptr_t)0, NoIntrinsic },
   { "MAX_VALUE", DontEnum|DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(numberConstructorMaxValue), (intptr_t)0, NoIntrinsic },
   { "MIN_VALUE", DontEnum|DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(numberConstructorMinValue), (intptr_t)0, NoIntrinsic },
   { 0, 0, 0, 0, NoIntrinsic }
};

extern const struct HashTable numberConstructorTable =
    { 16, 15, numberConstructorTableValues, 0 };
} // namespace
