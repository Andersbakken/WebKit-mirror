/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#if ENABLE(3D_CANVAS) || ENABLE(BLOB)

#include "DataView.h"

#include "CheckedInt.h"

namespace {

template<typename T>
union Value {
    T data;
    char bytes[sizeof(T)];
};

}

namespace WebCore {

PassRefPtr<DataView> DataView::create(PassRefPtr<ArrayBuffer> buffer, unsigned byteOffset, unsigned byteLength)
{
    if (byteOffset > buffer->byteLength())
        return 0;
    CheckedInt<uint32_t> checkedOffset(byteOffset);
    CheckedInt<uint32_t> checkedLength(byteLength);
    CheckedInt<uint32_t> checkedMax = checkedOffset + checkedLength;
    if (!checkedMax.valid() || checkedMax.value() > buffer->byteLength())
        return 0;
    return adoptRef(new DataView(buffer, byteOffset, byteLength));
}

DataView::DataView(PassRefPtr<ArrayBuffer> buffer, unsigned byteOffset, unsigned byteLength)
    : ArrayBufferView(buffer, byteOffset)
    , m_byteLength(byteLength)
{
}

static bool needToFlipBytes(bool littleEndian)
{
#if CPU(BIG_ENDIAN)
    return littleEndian;
#else
    return !littleEndian;
#endif
}

inline void swapBytes(char* p, char* q)
{
    char temp = *p;
    *p = *q;
    *q = temp;
}

static void flipBytesFor16Bits(char* p)
{
    swapBytes(p, p + 1);
}

static void flipBytesFor32Bits(char* p)
{
    swapBytes(p, p + 3);
    swapBytes(p + 1, p + 2);
}

static void flipBytesFor64Bits(char* p)
{
    swapBytes(p, p + 7);
    swapBytes(p + 1, p + 6);
    swapBytes(p + 2, p + 5);
    swapBytes(p + 3, p + 4);
}

static void flipBytesIfNeeded(char* value, size_t size, bool littleEndian)
{
    if (!needToFlipBytes(littleEndian))
        return;

    switch (size) {
    case 1:
        // Nothing to do.
        break;
    case 2:
        flipBytesFor16Bits(value);
        break;
    case 4:
        flipBytesFor32Bits(value);
        break;
    case 8:
        flipBytesFor64Bits(value);
        break;
    default:
        ASSERT_NOT_REACHED();
        break;
    }
}

template<typename T>
T DataView::getData(unsigned byteOffset, bool littleEndian, ExceptionCode& ec) const
{
    if (beyondRange<T>(byteOffset)) {
        ec = INDEX_SIZE_ERR;
        return 0;
    }

    // We do not want to load the data directly since it would cause a bus error on architectures that don't support unaligned loads.
    Value<T> value;
    memcpy(value.bytes, static_cast<const char*>(m_baseAddress) + byteOffset, sizeof(T));
    flipBytesIfNeeded(value.bytes, sizeof(T), littleEndian);
    return value.data;
}

template<typename T>
void DataView::setData(unsigned byteOffset, T value, bool littleEndian, ExceptionCode& ec)
{
    if (beyondRange<T>(byteOffset)) {
        ec = INDEX_SIZE_ERR;
        return;
    }

    // We do not want to store the data directly since it would cause a bus error on architectures that don't support unaligned stores.
    Value<T> tempValue;
    tempValue.data = value;
    flipBytesIfNeeded(tempValue.bytes, sizeof(T), littleEndian);
    memcpy(static_cast<char*>(m_baseAddress) + byteOffset, tempValue.bytes, sizeof(T));
}

char DataView::getInt8(unsigned byteOffset, ExceptionCode& ec)
{
    return getData<char>(byteOffset, false, ec);
}

unsigned char DataView::getUint8(unsigned byteOffset, ExceptionCode& ec)
{
    return getData<unsigned char>(byteOffset, false, ec);
}

short DataView::getInt16(unsigned byteOffset, bool littleEndian, ExceptionCode& ec)
{
    return getData<short>(byteOffset, littleEndian, ec);
}

unsigned short DataView::getUint16(unsigned byteOffset, bool littleEndian, ExceptionCode& ec)
{
    return getData<unsigned short>(byteOffset, littleEndian, ec);
}

int DataView::getInt32(unsigned byteOffset, bool littleEndian, ExceptionCode& ec)
{
    return getData<int>(byteOffset, littleEndian, ec);
}

unsigned DataView::getUint32(unsigned byteOffset, bool littleEndian, ExceptionCode& ec)
{
    return getData<unsigned>(byteOffset, littleEndian, ec);
}

float DataView::getFloat32(unsigned byteOffset, bool littleEndian, ExceptionCode& ec)
{
    return getData<float>(byteOffset, littleEndian, ec);
}

double DataView::getFloat64(unsigned byteOffset, bool littleEndian, ExceptionCode& ec)
{
    return getData<double>(byteOffset, littleEndian, ec);
}

void DataView::setInt8(unsigned byteOffset, char value, ExceptionCode& ec)
{
    setData<char>(byteOffset, value, false, ec);
}

void DataView::setUint8(unsigned byteOffset, unsigned char value, ExceptionCode& ec)
{
    setData<unsigned char>(byteOffset, value, false, ec);
}

void DataView::setInt16(unsigned byteOffset, short value, bool littleEndian, ExceptionCode& ec)
{
    setData<short>(byteOffset, value, littleEndian, ec);
}

void DataView::setUint16(unsigned byteOffset, unsigned short value, bool littleEndian, ExceptionCode& ec)
{
    setData<unsigned short>(byteOffset, value, littleEndian, ec);
}

void DataView::setInt32(unsigned byteOffset, int value, bool littleEndian, ExceptionCode& ec)
{
    setData<int>(byteOffset, value, littleEndian, ec);
}

void DataView::setUint32(unsigned byteOffset, unsigned value, bool littleEndian, ExceptionCode& ec)
{
    setData<unsigned>(byteOffset, value, littleEndian, ec);
}

void DataView::setFloat32(unsigned byteOffset, float value, bool littleEndian, ExceptionCode& ec)
{
    setData<float>(byteOffset, value, littleEndian, ec);
}

void DataView::setFloat64(unsigned byteOffset, double value, bool littleEndian, ExceptionCode& ec)
{
    setData<double>(byteOffset, value, littleEndian, ec);
}

}

#endif // ENABLE(3D_CANVAS) || ENABLE(BLOB)
