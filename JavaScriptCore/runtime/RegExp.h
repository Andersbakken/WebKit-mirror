/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2007, 2008, 2009 Apple Inc. All rights reserved.
 *  Copyright (C) 2009 Torch Mobile, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef RegExp_h
#define RegExp_h

#include "UString.h"
#include "ExecutableAllocator.h"
#include <wtf/Forward.h>
#include <wtf/RefCounted.h>

namespace JSC {

    struct RegExpRepresentation;
    class JSGlobalData;

    class RegExp : public RefCounted<RegExp> {
    public:
        static PassRefPtr<RegExp> create(JSGlobalData* globalData, const UString& pattern, const UString& flags);
        ~RegExp();

        bool global() const { return m_flagBits & Global; }
        bool ignoreCase() const { return m_flagBits & IgnoreCase; }
        bool multiline() const { return m_flagBits & Multiline; }

        const UString& pattern() const { return m_patternString; }

        bool isValid() const { return !m_constructionError; }
        const char* errorMessage() const { return m_constructionError; }

        int match(const UString&, int startOffset, Vector<int, 32>* ovector = 0);
        unsigned numSubpatterns() const { return m_numSubpatterns; }
        
#if ENABLE(REGEXP_TRACING)
        void printTraceData();
#endif

    private:
        RegExp(JSGlobalData* globalData, const UString& pattern, const UString& flags);

        enum RegExpState {
            ParseError,
            JITCode,
            ByteCode
        } m_state;

        RegExpState compile(JSGlobalData*);

        enum FlagBits { Global = 1, IgnoreCase = 2, Multiline = 4 };
        UString m_patternString;
        int m_flagBits;
        const char* m_constructionError;
        unsigned m_numSubpatterns;
#if ENABLE(REGEXP_TRACING)
        unsigned m_rtMatchCallCount;
        unsigned m_rtMatchFoundCount;
#endif

        OwnPtr<RegExpRepresentation> m_representation;
    };

} // namespace JSC

#endif // RegExp_h
