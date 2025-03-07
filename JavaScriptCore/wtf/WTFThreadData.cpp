/*
 * Copyright (C) 2008, 2010 Apple Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 *
 */

#include "config.h"
#include "WTFThreadData.h"

namespace WTF {

#if WTFTHREADDATA_MULTITHREADED
ThreadSpecific<WTFThreadData>* WTFThreadData::staticData;
#else
WTFThreadData* WTFThreadData::staticData;
#endif

WTFThreadData::WTFThreadData()
    : m_atomicStringTable(0)
    , m_atomicStringTableDestructor(0)
#if USE(JSC)
    , m_defaultIdentifierTable(new JSC::IdentifierTable())
    , m_currentIdentifierTable(m_defaultIdentifierTable)
#endif
    , m_stackBounds(StackBounds::currentThreadStackBounds())
{
}

WTFThreadData::~WTFThreadData()
{
    if (m_atomicStringTableDestructor)
        m_atomicStringTableDestructor(m_atomicStringTable);
#if USE(JSC)
    delete m_defaultIdentifierTable;
#endif
}

}
