/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ArgumentDecoder.h"

#include "DataReference.h"
#include <stdio.h>

namespace CoreIPC {

ArgumentDecoder::ArgumentDecoder(const uint8_t* buffer, size_t bufferSize)
{
    initialize(buffer, bufferSize);
}

ArgumentDecoder::ArgumentDecoder(const uint8_t* buffer, size_t bufferSize, Deque<Attachment>& attachments)
{
    initialize(buffer, bufferSize);

    m_attachments.swap(attachments);
}

ArgumentDecoder::~ArgumentDecoder()
{
    ASSERT(m_buffer);
    fastFree(m_buffer);
    // FIXME: We need to dispose of the mach ports in cases of failure.
}

void ArgumentDecoder::initialize(const uint8_t* buffer, size_t bufferSize)
{
    m_buffer = static_cast<uint8_t*>(fastMalloc(bufferSize));
    m_bufferPos = m_buffer;
    m_bufferEnd = m_buffer + bufferSize;
    memcpy(m_buffer, buffer, bufferSize);

    // Decode the destination ID.
    decodeUInt64(m_destinationID);
}

static inline uint8_t* roundUpToAlignment(uint8_t* ptr, unsigned alignment)
{
    return (uint8_t*)(((uintptr_t)ptr + alignment - 1) & ~(uintptr_t)(alignment - 1));
}

bool ArgumentDecoder::alignBufferPosition(unsigned alignment, size_t size)
{
    uint8_t* buffer = roundUpToAlignment(m_bufferPos, alignment);
    if (buffer + size > m_bufferEnd) {
        // We've walked off the end of this buffer.
        markInvalid();
        return false;
    }
    
    m_bufferPos = buffer;
    return true;
}

bool ArgumentDecoder::bufferIsLargeEnoughToContain(unsigned alignment, size_t size) const
{
    return roundUpToAlignment(m_bufferPos, alignment) + size <= m_bufferEnd;
}

bool ArgumentDecoder::decodeBytes(Vector<uint8_t>& buffer)
{
    uint64_t size;
    if (!decodeUInt64(size))
        return false;

    if (!alignBufferPosition(1, size))
        return false;

    buffer.resize(size);
    if (size > 0)
        memcpy(&buffer[0], m_bufferPos, size);
    m_bufferPos += size;
    return true;
}

bool ArgumentDecoder::decodeBytes(DataReference& dataReference)
{
    uint64_t size;
    if (!decodeUInt64(size))
        return false;
    
    if (!alignBufferPosition(1, size))
        return false;

    uint8_t* data = m_bufferPos;
    m_bufferPos += size;

    dataReference = DataReference(data, size);
    return true;
}

bool ArgumentDecoder::decodeBytes(uint8_t* buffer, size_t bufferSize)
{
    // FIXME: Decoding the size is not strictly necessary here since we know the size upfront.
    uint64_t size;
    if (!decodeUInt64(size))
        return false;

    ASSERT(size == bufferSize);
    if (size != bufferSize)
        return false;

    if (!alignBufferPosition(1, size))
        return false;

    memcpy(buffer, m_bufferPos, size);
    m_bufferPos += size;
    return true;
}

bool ArgumentDecoder::decodeBool(bool& result)
{
    if (!alignBufferPosition(__alignof(result), sizeof(result)))
        return false;
    
    result = *reinterpret_cast<bool*>(m_bufferPos);
    m_bufferPos += sizeof(result);
    return true;
}

bool ArgumentDecoder::decodeUInt32(uint32_t& result)
{
    if (!alignBufferPosition(__alignof(result), sizeof(result)))
        return false;
    
    result = *reinterpret_cast<uint32_t*>(m_bufferPos);
    m_bufferPos += sizeof(result);
    return true;
}

bool ArgumentDecoder::decodeUInt64(uint64_t& result)
{
    if (!alignBufferPosition(__alignof(result), sizeof(result)))
        return false;
    
    result = *reinterpret_cast<uint64_t*>(m_bufferPos);
    m_bufferPos += sizeof(result);
    return true;
}

bool ArgumentDecoder::decodeInt32(int32_t& result)
{
    if (!alignBufferPosition(__alignof(result), sizeof(result)))
        return false;
    
    result = *reinterpret_cast<uint32_t*>(m_bufferPos);
    m_bufferPos += sizeof(result);
    return true;
}

bool ArgumentDecoder::decodeInt64(int64_t& result)
{
    if (!alignBufferPosition(__alignof(result), sizeof(result)))
        return false;
    
    result = *reinterpret_cast<uint64_t*>(m_bufferPos);
    m_bufferPos += sizeof(result);
    return true;
}

bool ArgumentDecoder::decodeFloat(float& result)
{
    if (!alignBufferPosition(__alignof(result), sizeof(result)))
        return false;
    
    result = *reinterpret_cast<float*>(m_bufferPos);
    m_bufferPos += sizeof(result);
    return true;
}

bool ArgumentDecoder::decodeDouble(double& result)
{
    if (!alignBufferPosition(__alignof(result), sizeof(result)))
        return false;
    
    result = *reinterpret_cast<double*>(m_bufferPos);
    m_bufferPos += sizeof(result);
    return true;
}

bool ArgumentDecoder::removeAttachment(Attachment& attachment)
{
    if (m_attachments.isEmpty())
        return false;

    attachment = m_attachments.takeFirst();
    return true;
}

#ifndef NDEBUG
void ArgumentDecoder::debug()
{
    printf("ArgumentDecoder::debug()\n");
    printf("Number of Attachments: %d\n", (int)m_attachments.size());
    printf("Size of buffer: %d\n", (int)(m_bufferEnd - m_buffer));
}
#endif

} // namespace CoreIPC
