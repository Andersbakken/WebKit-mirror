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

#include "NetscapePluginModule.h"

#include "PluginDatabase.h"
#include "PluginPackage.h"

using namespace WebCore;

namespace WebKit {

bool NetscapePluginModule::getPluginInfo(const String& pluginPath, PluginInfoStore::Plugin& plugin)
{
    // We are loading the plugin here since it does not seem to be a standardized way to
    // get the needed informations from a UNIX plugin without loading it.

    RefPtr<PluginPackage> package = PluginPackage::createPackage(pluginPath, 0 /*lastModified*/);
    if (!package)
        return false;

    plugin.path = pluginPath;
    plugin.info.desc = package->description();
    plugin.info.file = package->fileName();

    const MIMEToDescriptionsMap& descriptions = package->mimeToDescriptions();
    const MIMEToExtensionsMap& extensions = package->mimeToExtensions();
    MIMEToDescriptionsMap::const_iterator descEnd = descriptions.end();
    plugin.info.mimes.reserveCapacity(descriptions.size());
    unsigned i = 0;
    for (MIMEToDescriptionsMap::const_iterator it = descriptions.begin(); it != descEnd; ++it) {
        plugin.info.mimes.uncheckedAppend(MimeClassInfo());
        MimeClassInfo& mime = plugin.info.mimes[i++];
        mime.type = it->first;
        mime.desc = it->second;
        MIMEToExtensionsMap::const_iterator extensionIt = extensions.find(it->first);
        ASSERT(extensionIt != extensions.end());
        mime.extensions = extensionIt->second;
    }

    package->unload();
    return true;
}

void NetscapePluginModule::determineQuirks()
{
}

} // namespace WebKit
