/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "DOMFileSystemSync.h"

#if ENABLE(FILE_SYSTEM)

#include "DOMFilePath.h"
#include "DirectoryEntrySync.h"
#include "ErrorCallback.h"
#include "File.h"
#include "FileEntrySync.h"
#include "FileError.h"
#include "FileException.h"
#include "FileSystemCallbacks.h"
#include "FileWriterBaseCallback.h"
#include "FileWriterSync.h"

namespace WebCore {

class FileWriterBase;

PassRefPtr<DOMFileSystemSync> DOMFileSystemSync::create(DOMFileSystemBase* fileSystem)
{
    return adoptRef(new DOMFileSystemSync(fileSystem->m_name, fileSystem->m_asyncFileSystem.release()));
}

DOMFileSystemSync::DOMFileSystemSync(const String& name, PassOwnPtr<AsyncFileSystem> asyncFileSystem)
    : DOMFileSystemBase(name, asyncFileSystem)
{
}

DOMFileSystemSync::~DOMFileSystemSync()
{
}

PassRefPtr<DirectoryEntrySync> DOMFileSystemSync::root()
{
    return DirectoryEntrySync::create(this, DOMFilePath::root);
}

PassRefPtr<File> DOMFileSystemSync::createFile(const FileEntrySync* fileEntry, ExceptionCode& ec)
{
    ec = 0;
    String platformPath = m_asyncFileSystem->virtualToPlatformPath(fileEntry->fullPath());
    return File::create(platformPath);
}

namespace {

class ReceiveFileWriterCallback : public FileWriterBaseCallback {
public:
    static PassRefPtr<ReceiveFileWriterCallback> create()
    {
        return adoptRef(new ReceiveFileWriterCallback());
    }

    bool handleEvent(FileWriterBase* fileWriterBase)
    {
#ifndef NDEBUG
        m_fileWriterBase = fileWriterBase;
#else
        ASSERT_UNUSED(fileWriterBase, fileWriterBase);
#endif
        return true;
    }

#ifndef NDEBUG
    FileWriterBase* fileWriterBase()
    {
        return m_fileWriterBase;
    }
#endif

private:
    ReceiveFileWriterCallback()
#ifndef NDEBUG
        : m_fileWriterBase(0)
#endif
    {
    }

#ifndef NDEBUG
    FileWriterBase* m_fileWriterBase;
#endif
};

class LocalErrorCallback : public ErrorCallback {
public:
    static PassRefPtr<LocalErrorCallback> create()
    {
        return adoptRef(new LocalErrorCallback());
    }

    bool handleEvent(FileError* error)
    {
        m_error = error;
        return true;
    }

    FileError* error()
    {
        return m_error.get();
    }

private:
    LocalErrorCallback()
    {
    }
    RefPtr<FileError> m_error;
};

}

PassRefPtr<FileWriterSync> DOMFileSystemSync::createWriter(const FileEntrySync* fileEntry, ExceptionCode& ec)
{
    ASSERT(fileEntry);
    ec = 0;

    String platformPath = m_asyncFileSystem->virtualToPlatformPath(fileEntry->fullPath());

    RefPtr<FileWriterSync> fileWriter = FileWriterSync::create();
    RefPtr<ReceiveFileWriterCallback> successCallback = ReceiveFileWriterCallback::create();
    RefPtr<LocalErrorCallback> errorCallback = LocalErrorCallback::create();

    OwnPtr<FileWriterBaseCallbacks> callbacks = FileWriterBaseCallbacks::create(fileWriter, successCallback, errorCallback);
    m_asyncFileSystem->createWriter(fileWriter.get(), platformPath, callbacks.release());
    if (!m_asyncFileSystem->waitForOperationToComplete()) {
        ec = FileException::ABORT_ERR;
        return 0;
    }
    if (errorCallback->error()) {
        ASSERT(!successCallback->fileWriterBase());
        ec = FileException::ErrorCodeToExceptionCode(errorCallback->error()->code());
        return 0;
    }
    ASSERT(successCallback->fileWriterBase());
    ASSERT(static_cast<FileWriterSync*>(successCallback->fileWriterBase()) == fileWriter.get());
    return fileWriter;
}

}

#endif // ENABLE(FILE_SYSTEM)
