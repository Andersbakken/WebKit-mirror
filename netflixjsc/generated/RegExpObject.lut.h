// Automatically generated from ../../../../Source/JavaScriptCore/runtime/RegExpObject.cpp using /home/abakken/dev/WebKit/Source/JavaScriptCore/create_hash_table. DO NOT EDIT!

#include "Lookup.h"

namespace JSC {

static const struct HashTableValue regExpTableValues[6] = {
   { "global", DontDelete|ReadOnly|DontEnum, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpObjectGlobal), (intptr_t)0, NoIntrinsic },
   { "ignoreCase", DontDelete|ReadOnly|DontEnum, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpObjectIgnoreCase), (intptr_t)0, NoIntrinsic },
   { "multiline", DontDelete|ReadOnly|DontEnum, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpObjectMultiline), (intptr_t)0, NoIntrinsic },
   { "source", DontDelete|ReadOnly|DontEnum, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpObjectSource), (intptr_t)0, NoIntrinsic },
   { "lastIndex", DontDelete|DontEnum, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpObjectLastIndex), (intptr_t)setRegExpObjectLastIndex, NoIntrinsic },
   { 0, 0, 0, 0, NoIntrinsic }
};

extern const struct HashTable regExpTable =
    { 17, 15, regExpTableValues, 0 };
} // namespace
