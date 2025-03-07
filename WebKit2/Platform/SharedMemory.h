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

#ifndef SharedMemory_h
#define SharedMemory_h

#include <wtf/Noncopyable.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>

#if PLATFORM(QT)
#include <QtGlobal>
QT_BEGIN_NAMESPACE
class QSharedMemory;
QT_END_NAMESPACE
#include <wtf/text/WTFString.h>
#endif

namespace CoreIPC {
    class ArgumentDecoder;
    class ArgumentEncoder;
}

namespace WebKit {

class SharedMemory : public RefCounted<SharedMemory> {
public:
    enum Protection {
        ReadOnly,
        ReadWrite
    };

    class Handle : Noncopyable {
    public:
        Handle();
        ~Handle();

        bool isNull() const;

        void encode(CoreIPC::ArgumentEncoder*) const;
        static bool decode(CoreIPC::ArgumentDecoder*, Handle&);

    private:
        friend class SharedMemory;
#if PLATFORM(MAC)
        mutable mach_port_t m_port;
#elif PLATFORM(WIN)
        mutable HANDLE m_handle;
#elif PLATFORM(QT)
        mutable String m_key;
#endif
        size_t m_size;
    };
    
    // Create a shared memory object with the given size. Will return 0 on failure.
    static PassRefPtr<SharedMemory> create(size_t);

    // Create a shared memory object from the given handle and the requested protection. Will return 0 on failure.
    static PassRefPtr<SharedMemory> create(const Handle&, Protection);
    
    ~SharedMemory();

    bool createHandle(Handle&, Protection);

    size_t size() const { return m_size; }
    void* data() const { return m_data; }

    // Return the system page size in bytes.
    static unsigned systemPageSize();

private:
    size_t m_size;
    void* m_data;
#if PLATFORM(WIN)
    HANDLE m_handle;
#elif PLATFORM(QT)
    QSharedMemory* m_impl;
#endif
};

};

#endif // SharedMemory_h
