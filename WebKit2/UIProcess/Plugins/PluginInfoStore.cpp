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

#include "PluginInfoStore.h"

#include <WebCore/KURL.h>
#include <WebCore/MIMETypeRegistry.h>
#include <algorithm>
#include <wtf/StdLibExtras.h>

using namespace std;
using namespace WebCore;

namespace WebKit {

PluginInfoStore::PluginInfoStore()
    : m_pluginListIsUpToDate(false)
{
}

void PluginInfoStore::setAdditionalPluginsDirectories(const Vector<String>& directories)
{
    m_additionalPluginsDirectories = directories;
    refresh();
}

void PluginInfoStore::refresh()
{
    m_pluginListIsUpToDate = false;
}

template <typename T, typename U, typename V, typename W>
static void addFromVector(HashSet<T, U, V>& hashSet, const W& vector)
{
    for (size_t i = 0; i < vector.size(); ++i)
        hashSet.add(vector[i]);
}

#if OS(WINDOWS)
typedef HashSet<String, CaseFoldingHash> PathHashSet;
#else
typedef HashSet<String> PathHashSet;
#endif

void PluginInfoStore::loadPluginsIfNecessary()
{
    if (m_pluginListIsUpToDate)
        return;

    m_plugins.clear();

    PathHashSet uniquePluginPaths;

    // First, load plug-ins from the additional plug-ins directories specified.
    for (size_t i = 0; i < m_additionalPluginsDirectories.size(); ++i)
        addFromVector(uniquePluginPaths, pluginPathsInDirectory(m_additionalPluginsDirectories[i]));

    // Then load plug-ins from the standard plug-ins directories.
    Vector<String> directories = pluginsDirectories();
    for (size_t i = 0; i < directories.size(); ++i)
        addFromVector(uniquePluginPaths, pluginPathsInDirectory(directories[i]));

    // Then load plug-ins that are not in the standard plug-ins directories.
    addFromVector(uniquePluginPaths, individualPluginPaths());

    PathHashSet::const_iterator end = uniquePluginPaths.end();
    for (PathHashSet::const_iterator it = uniquePluginPaths.begin(); it != end; ++it)
        loadPlugin(*it);

    m_pluginListIsUpToDate = true;
}

void PluginInfoStore::loadPlugin(const String& pluginPath)
{
    Plugin plugin;
    
    if (!getPluginInfo(pluginPath, plugin))
        return;

    if (!shouldUsePlugin(plugin))
        return;
    
    // Add the plug-in.
    m_plugins.append(plugin);
}

void PluginInfoStore::getPlugins(Vector<PluginInfo>& plugins)
{
    loadPluginsIfNecessary();

    for (size_t i = 0; i < m_plugins.size(); ++i)
        plugins.append(m_plugins[i].info);
}

PluginInfoStore::Plugin PluginInfoStore::findPluginForMIMEType(const String& mimeType)
{
    ASSERT(!mimeType.isNull());
    
    for (size_t i = 0; i < m_plugins.size(); ++i) {
        const Plugin& plugin = m_plugins[i];
        
        for (size_t j = 0; j < plugin.info.mimes.size(); ++j) {
            const MimeClassInfo& mimeClassInfo = plugin.info.mimes[j];
            if (mimeClassInfo.type == mimeType)
                return plugin;
        }
    }
    
    return Plugin();
}

PluginInfoStore::Plugin PluginInfoStore::findPluginForExtension(const String& extension, String& mimeType)
{
    ASSERT(!extension.isNull());
    
    for (size_t i = 0; i < m_plugins.size(); ++i) {
        const Plugin& plugin = m_plugins[i];
        
        for (size_t j = 0; j < plugin.info.mimes.size(); ++j) {
            const MimeClassInfo& mimeClassInfo = plugin.info.mimes[j];
            
            const Vector<String>& extensions = mimeClassInfo.extensions;
            
            if (find(extensions.begin(), extensions.end(), extension) != extensions.end()) {
                // We found a supported extension, set the correct MIME type.
                mimeType = mimeClassInfo.type;
                return plugin;
            }
        }
    }
    
    return Plugin();
}

static inline String pathExtension(const KURL& url)
{
    String extension;
    String filename = url.lastPathComponent();
    if (!filename.endsWith("/")) {
        int extensionPos = filename.reverseFind('.');
        if (extensionPos != -1)
            extension = filename.substring(extensionPos + 1);
    }
    
    return extension;
}

#if !PLATFORM(MAC)
String PluginInfoStore::getMIMETypeForExtension(const String& extension)
{
    return MIMETypeRegistry::getMIMETypeForExtension(extension);
}
#endif

PluginInfoStore::Plugin PluginInfoStore::findPlugin(String& mimeType, const KURL& url)
{
    loadPluginsIfNecessary();
    
    // First, check if we can get the plug-in based on its MIME type.
    if (!mimeType.isNull()) {
        Plugin plugin = findPluginForMIMEType(mimeType);
        if (!plugin.path.isNull())
            return plugin;
    }

    // Next, check if any plug-ins claim to support the URL extension.
    String extension = pathExtension(url).lower();
    if (!extension.isNull() && mimeType.isEmpty()) {
        Plugin plugin = findPluginForExtension(extension, mimeType);
        if (!plugin.path.isNull())
            return plugin;
        
        // Finally, try to get the MIME type from the extension in a platform specific manner and use that.
        String extensionMimeType = getMIMETypeForExtension(extension);
        if (!extensionMimeType.isNull()) {
            Plugin plugin = findPluginForMIMEType(extensionMimeType);
            if (!plugin.path.isNull()) {
                mimeType = extensionMimeType;
                return plugin;
            }
        }
    }
    
    return Plugin();
}

PluginInfoStore::Plugin PluginInfoStore::infoForPluginWithPath(const String& pluginPath)
{
    for (size_t i = 0; i < m_plugins.size(); ++i) {
        if (m_plugins[i].path == pluginPath)
            return m_plugins[i];
    }
    
    ASSERT_NOT_REACHED();
    return Plugin();
}

} // namespace WebKit
