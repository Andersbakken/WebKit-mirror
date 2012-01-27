// Automatically generated from ../../../../Source/JavaScriptCore/runtime/StringPrototype.cpp using /home/abakken/dev/WebKit/Source/JavaScriptCore/create_hash_table. DO NOT EDIT!

#include "Lookup.h"

namespace JSC {

static const struct HashTableValue stringTableValues[36] = {
   { "toString", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncToString), (intptr_t)0, NoIntrinsic },
   { "valueOf", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncToString), (intptr_t)0, NoIntrinsic },
   { "charAt", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncCharAt), (intptr_t)1, CharAtIntrinsic },
   { "charCodeAt", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncCharCodeAt), (intptr_t)1, CharCodeAtIntrinsic },
   { "concat", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncConcat), (intptr_t)1, NoIntrinsic },
   { "indexOf", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncIndexOf), (intptr_t)1, NoIntrinsic },
   { "lastIndexOf", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncLastIndexOf), (intptr_t)1, NoIntrinsic },
   { "match", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncMatch), (intptr_t)1, NoIntrinsic },
   { "replace", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncReplace), (intptr_t)2, NoIntrinsic },
   { "search", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncSearch), (intptr_t)1, NoIntrinsic },
   { "slice", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncSlice), (intptr_t)2, NoIntrinsic },
   { "split", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncSplit), (intptr_t)2, NoIntrinsic },
   { "substr", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncSubstr), (intptr_t)2, NoIntrinsic },
   { "substring", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncSubstring), (intptr_t)2, NoIntrinsic },
   { "toLowerCase", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncToLowerCase), (intptr_t)0, NoIntrinsic },
   { "toUpperCase", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncToUpperCase), (intptr_t)0, NoIntrinsic },
   { "localeCompare", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncLocaleCompare), (intptr_t)1, NoIntrinsic },
   { "toLocaleLowerCase", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncToLowerCase), (intptr_t)0, NoIntrinsic },
   { "toLocaleUpperCase", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncToUpperCase), (intptr_t)0, NoIntrinsic },
   { "big", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncBig), (intptr_t)0, NoIntrinsic },
   { "small", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncSmall), (intptr_t)0, NoIntrinsic },
   { "blink", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncBlink), (intptr_t)0, NoIntrinsic },
   { "bold", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncBold), (intptr_t)0, NoIntrinsic },
   { "fixed", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncFixed), (intptr_t)0, NoIntrinsic },
   { "italics", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncItalics), (intptr_t)0, NoIntrinsic },
   { "strike", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncStrike), (intptr_t)0, NoIntrinsic },
   { "sub", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncSub), (intptr_t)0, NoIntrinsic },
   { "sup", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncSup), (intptr_t)0, NoIntrinsic },
   { "fontcolor", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncFontcolor), (intptr_t)1, NoIntrinsic },
   { "fontsize", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncFontsize), (intptr_t)1, NoIntrinsic },
   { "anchor", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncAnchor), (intptr_t)1, NoIntrinsic },
   { "link", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncLink), (intptr_t)1, NoIntrinsic },
   { "trim", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncTrim), (intptr_t)0, NoIntrinsic },
   { "trimLeft", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncTrimLeft), (intptr_t)0, NoIntrinsic },
   { "trimRight", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(stringProtoFuncTrimRight), (intptr_t)0, NoIntrinsic },
   { 0, 0, 0, 0, NoIntrinsic }
};

extern const struct HashTable stringTable =
    { 133, 127, stringTableValues, 0 };
} // namespace
