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

#include "SharedMemory.h"

#include "ArgumentDecoder.h"
#include "ArgumentEncoder.h"
#include <wtf/RefPtr.h>

namespace WebKit {

SharedMemory::Handle::Handle()
    : m_handle(0)
    , m_size(0)
{
}

SharedMemory::Handle::~Handle()
{
    if (!m_handle)
        return;

    ::CloseHandle(m_handle);
}

void SharedMemory::Handle::encode(CoreIPC::ArgumentEncoder* encoder) const
{
    encoder->encodeUInt64(m_size);

    // Hand off ownership of our HANDLE to the receiving process. It will close it for us.
    // FIXME: If the receiving process crashes before it receives the memory, the memory will be
    // leaked. See <http://webkit.org/b/47502>.
    encoder->encodeUInt64(reinterpret_cast<uint64_t>(m_handle));
    m_handle = 0;

    // Send along our PID so that the receiving process can duplicate the HANDLE for its own use.
    encoder->encodeUInt32(::GetCurrentProcessId());
}

bool SharedMemory::Handle::decode(CoreIPC::ArgumentDecoder* decoder, Handle& handle)
{
    ASSERT_ARG(handle, !handle.m_handle);
    ASSERT_ARG(handle, !handle.m_size);

    uint64_t size;
    if (!decoder->decodeUInt64(size))
        return false;

    uint64_t sourceHandle;
    if (!decoder->decodeUInt64(sourceHandle))
        return false;

    uint32_t sourcePID;
    if (!decoder->decodeUInt32(sourcePID))
        return false;

    HANDLE sourceProcess = ::OpenProcess(PROCESS_DUP_HANDLE, FALSE, sourcePID);
    if (!sourceProcess)
        return false;

    // Copy the handle into our process and close the handle that the sending process created for us.
    HANDLE duplicatedHandle;
    BOOL success = ::DuplicateHandle(sourceProcess, reinterpret_cast<HANDLE>(sourceHandle), ::GetCurrentProcess(), &duplicatedHandle, 0, FALSE, DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE);

    ::CloseHandle(sourceProcess);

    if (!success)
        return false;

    handle.m_handle = duplicatedHandle;
    handle.m_size = size;
    return true;
}

PassRefPtr<SharedMemory> SharedMemory::create(size_t size)
{
    HANDLE handle = ::CreateFileMappingW(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, size, 0);
    if (!handle)
        return 0;

    void* baseAddress = ::MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, size);
    if (!baseAddress) {
        ::CloseHandle(handle);
        return 0;
    }

    RefPtr<SharedMemory> memory = adoptRef(new SharedMemory);
    memory->m_size = size;
    memory->m_data = baseAddress;
    memory->m_handle = handle;

    return memory.release();
}

static DWORD accessRights(SharedMemory::Protection protection)
{
    switch (protection) {
    case SharedMemory::ReadOnly:
        return FILE_MAP_READ;
    case SharedMemory::ReadWrite:
        // FILE_MAP_WRITE implies read access, too.
        return FILE_MAP_WRITE;
    }

    ASSERT_NOT_REACHED();
    return 0;
}

PassRefPtr<SharedMemory> SharedMemory::create(const Handle& handle, Protection protection)
{
    DWORD desiredAccess = accessRights(protection);

    void* baseAddress = ::MapViewOfFile(handle.m_handle, desiredAccess, 0, 0, handle.m_size);
    if (!baseAddress)
        return 0;

    RefPtr<SharedMemory> memory = adoptRef(new SharedMemory);
    memory->m_size = handle.m_size;
    memory->m_data = baseAddress;

    // Adopt the HANDLE.
    memory->m_handle = handle.m_handle;
    handle.m_handle = 0;

    return memory.release();
}

SharedMemory::~SharedMemory()
{
    ASSERT(m_data);
    ASSERT(m_handle);

    ::UnmapViewOfFile(m_data);
    ::CloseHandle(m_handle);
}
    
bool SharedMemory::createHandle(Handle& handle, Protection protection)
{
    ASSERT_ARG(handle, !handle.m_handle);
    ASSERT_ARG(handle, !handle.m_size);

    HANDLE processHandle = ::GetCurrentProcess();

    HANDLE duplicatedHandle;
    if (!::DuplicateHandle(processHandle, m_handle, processHandle, &duplicatedHandle, accessRights(protection), FALSE, 0))
        return false;

    handle.m_handle = duplicatedHandle;
    handle.m_size = m_size;
    return true;
}

unsigned SharedMemory::systemPageSize()
{
    static unsigned pageSize = 0;

    if (!pageSize) {
        SYSTEM_INFO systemInfo;
        ::GetSystemInfo(&systemInfo);
        pageSize = systemInfo.dwPageSize;
    }

    return pageSize;
}

} // namespace WebKit
