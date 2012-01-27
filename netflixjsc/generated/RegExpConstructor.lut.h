// Automatically generated from ../../../../Source/JavaScriptCore/runtime/RegExpConstructor.cpp using /home/abakken/dev/WebKit/Source/JavaScriptCore/create_hash_table. DO NOT EDIT!

#include "Lookup.h"

namespace JSC {

static const struct HashTableValue regExpConstructorTableValues[22] = {
   { "input", None, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorInput), (intptr_t)setRegExpConstructorInput, NoIntrinsic },
   { "$_", DontEnum, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorInput), (intptr_t)setRegExpConstructorInput, NoIntrinsic },
   { "multiline", None, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorMultiline), (intptr_t)setRegExpConstructorMultiline, NoIntrinsic },
   { "$*", DontEnum, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorMultiline), (intptr_t)setRegExpConstructorMultiline, NoIntrinsic },
   { "lastMatch", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorLastMatch), (intptr_t)0, NoIntrinsic },
   { "$&", DontDelete|ReadOnly|DontEnum, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorLastMatch), (intptr_t)0, NoIntrinsic },
   { "lastParen", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorLastParen), (intptr_t)0, NoIntrinsic },
   { "$+", DontDelete|ReadOnly|DontEnum, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorLastParen), (intptr_t)0, NoIntrinsic },
   { "leftContext", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorLeftContext), (intptr_t)0, NoIntrinsic },
   { "$`", DontDelete|ReadOnly|DontEnum, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorLeftContext), (intptr_t)0, NoIntrinsic },
   { "rightContext", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorRightContext), (intptr_t)0, NoIntrinsic },
   { "$'", DontDelete|ReadOnly|DontEnum, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorRightContext), (intptr_t)0, NoIntrinsic },
   { "$1", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorDollar1), (intptr_t)0, NoIntrinsic },
   { "$2", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorDollar2), (intptr_t)0, NoIntrinsic },
   { "$3", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorDollar3), (intptr_t)0, NoIntrinsic },
   { "$4", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorDollar4), (intptr_t)0, NoIntrinsic },
   { "$5", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorDollar5), (intptr_t)0, NoIntrinsic },
   { "$6", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorDollar6), (intptr_t)0, NoIntrinsic },
   { "$7", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorDollar7), (intptr_t)0, NoIntrinsic },
   { "$8", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorDollar8), (intptr_t)0, NoIntrinsic },
   { "$9", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorDollar9), (intptr_t)0, NoIntrinsic },
   { 0, 0, 0, 0, NoIntrinsic }
};

extern const struct HashTable regExpConstructorTable =
    { 65, 63, regExpConstructorTableValues, 0 };
} // namespace
