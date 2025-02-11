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
#include "AsyncFileSystem.h"

#if ENABLE(FILE_SYSTEM)

#include "AsyncFileSystemCallbacks.h"
#include "FileSystem.h"
#include "NotImplemented.h"

namespace WebCore {

#if !PLATFORM(CHROMIUM)
bool AsyncFileSystem::isAvailable()
{
    notImplemented();
    return false;
}

PassOwnPtr<AsyncFileSystem> AsyncFileSystem::create(const String&)
{
    notImplemented();
    return 0;
}

// Default implementation.
void AsyncFileSystem::openFileSystem(const String& basePath, const String& storageIdentifier, Type type, bool, PassOwnPtr<AsyncFileSystemCallbacks> callbacks)
{
    String typeString = (type == Persistent) ? "Persistent" : "Temporary";

    String name = storageIdentifier;
    name += ":";
    name += typeString;

    String rootPath = basePath;
    rootPath.append(PlatformFilePathSeparator);
    rootPath += storageIdentifier;
    rootPath.append(PlatformFilePathSeparator);
    rootPath += typeString;
    rootPath.append(PlatformFilePathSeparator);

    callbacks->didOpenFileSystem(name, AsyncFileSystem::create(rootPath));
}
#endif

// Default implementation.
String AsyncFileSystem::virtualToPlatformPath(const String& path) const
{
    ASSERT(!m_platformRootPath.isEmpty());
    String virtualPath = path;
    return m_platformRootPath + virtualPath.replace('/', PlatformFilePathSeparator);
}

} // namespace

#endif // ENABLE(FILE_SYSTEM)
