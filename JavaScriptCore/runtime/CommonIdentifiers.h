/*
 *  Copyright (C) 2003, 2007, 2009 Apple Inc. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#ifndef CommonIdentifiers_h
#define CommonIdentifiers_h

#include "Identifier.h"
#include <wtf/Noncopyable.h>

// MarkedArgumentBuffer of property names, passed to a macro so we can do set them up various
// ways without repeating the list.
#define JSC_COMMON_IDENTIFIERS_EACH_PROPERTY_NAME(macro) \
    macro(__defineGetter__) \
    macro(__defineSetter__) \
    macro(__lookupGetter__) \
    macro(__lookupSetter__) \
    macro(apply) \
    macro(arguments) \
    macro(call) \
    macro(callee) \
    macro(caller) \
    macro(compile) \
    macro(configurable) \
    macro(constructor) \
    macro(create) \
    macro(defineProperty) \
    macro(defineProperties) \
    macro(enumerable) \
    macro(eval) \
    macro(exec) \
    macro(fromCharCode) \
    macro(global) \
    macro(get) \
    macro(getPrototypeOf) \
    macro(getOwnPropertyDescriptor) \
    macro(getOwnPropertyNames) \
    macro(hasOwnProperty) \
    macro(ignoreCase) \
    macro(index) \
    macro(input) \
    macro(isArray) \
    macro(isPrototypeOf) \
    macro(keys) \
    macro(length) \
    macro(message) \
    macro(multiline) \
    macro(name) \
    macro(now) \
    macro(parse) \
    macro(propertyIsEnumerable) \
    macro(prototype) \
    macro(set) \
    macro(source) \
    macro(test) \
    macro(toExponential) \
    macro(toFixed) \
    macro(toISOString) \
    macro(toJSON) \
    macro(toLocaleString) \
    macro(toPrecision) \
    macro(toString) \
    macro(UTC) \
    macro(value) \
    macro(valueOf) \
    macro(writable) \
    macro(displayName)

namespace JSC {

    class CommonIdentifiers : public Noncopyable {
    private:
        CommonIdentifiers(JSGlobalData*);
        friend class JSGlobalData;

    public:
        const Identifier nullIdentifier;
        const Identifier emptyIdentifier;
        const Identifier underscoreProto;
        const Identifier thisIdentifier;
        const Identifier useStrictIdentifier;

#define JSC_IDENTIFIER_DECLARE_PROPERTY_NAME_GLOBAL(name) const Identifier name;
        JSC_COMMON_IDENTIFIERS_EACH_PROPERTY_NAME(JSC_IDENTIFIER_DECLARE_PROPERTY_NAME_GLOBAL)
#undef JSC_IDENTIFIER_DECLARE_PROPERTY_NAME_GLOBAL
    };

} // namespace JSC

#endif // CommonIdentifiers_h
