/*
 * Copyright (c) 2008, 2009, Google Inc. All rights reserved.
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
#include "FileSystem.h"

#include "ChromiumBridge.h"
#include "NotImplemented.h"
#include "PlatformString.h"

namespace WebCore {

bool deleteFile(const String& path)
{
    return ChromiumBridge::deleteFile(path);
}

bool deleteEmptyDirectory(const String& path)
{
    return ChromiumBridge::deleteEmptyDirectory(path);
}

bool getFileSize(const String& path, long long& result)
{
    return ChromiumBridge::getFileSize(path, result);
}

bool getFileModificationTime(const String& path, time_t& result)
{
    return ChromiumBridge::getFileModificationTime(path, result);
}

void revealFolderInOS(const String& path)
{
    ChromiumBridge::revealFolderInOS(path);
}

String directoryName(const String& path)
{
    return ChromiumBridge::directoryName(path);
}

String pathByAppendingComponent(const String& path, const String& component)
{
    return ChromiumBridge::pathByAppendingComponent(path, component);
}

bool makeAllDirectories(const String& path)
{
    return ChromiumBridge::makeAllDirectories(path);
}

bool fileExists(const String& path)
{
    return ChromiumBridge::fileExists(path);
}

PlatformFileHandle openFile(const String& path, FileOpenMode mode)
{
    return ChromiumBridge::openFile(path, mode);
}

void closeFile(PlatformFileHandle& handle)
{
    return ChromiumBridge::closeFile(handle);
}

long long seekFile(PlatformFileHandle handle, long long offset, FileSeekOrigin origin)
{
    return ChromiumBridge::seekFile(handle, offset, origin);
}

bool truncateFile(PlatformFileHandle handle, long long offset)
{
    return ChromiumBridge::truncateFile(handle, offset);
}

int readFromFile(PlatformFileHandle handle, char* data, int length)
{
    return ChromiumBridge::readFromFile(handle, data, length);
}

int writeToFile(PlatformFileHandle handle, const char* data, int length)
{
    return ChromiumBridge::writeToFile(handle, data, length);
}

} // namespace WebCore
