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

#if ENABLE(PLUGIN_PROCESS)
 
#include "PluginProcess.h"

#include "NetscapePlugin.h"
#include "PluginProcessShim.h"
#include <dlfcn.h>

namespace WebKit {

static bool isUserbreakSet = false;

static void initShouldCallRealDebugger()
{
    char* var = getenv("USERBREAK");
    
    if (var)
        isUserbreakSet = atoi(var);
}

static bool shouldCallRealDebugger()
{
    static pthread_once_t shouldCallRealDebuggerOnce = PTHREAD_ONCE_INIT;
    pthread_once(&shouldCallRealDebuggerOnce, initShouldCallRealDebugger);
    
    return isUserbreakSet;
}

static bool isWindowActive(WindowRef windowRef, bool& result)
{
#ifndef NP_NO_CARBON
    if (NetscapePlugin* plugin = NetscapePlugin::netscapePluginFromWindow(windowRef)) {
        result = plugin->isWindowActive();
        return true;
    }
#endif
    return false;
}

static UInt32 getCurrentEventButtonState()
{
#ifndef NP_NO_CARBON
    return NetscapePlugin::buttonState();
#else
    ASSERT_NOT_REACHED();
    return 0;
#endif
}
    
void PluginProcess::initializeShim()
{
    const PluginProcessShimCallbacks callbacks = {
        shouldCallRealDebugger,
        isWindowActive,
        getCurrentEventButtonState
    };

    PluginProcessShimInitializeFunc initFunc = reinterpret_cast<PluginProcessShimInitializeFunc>(dlsym(RTLD_DEFAULT, "WebKitPluginProcessShimInitialize"));
    initFunc(callbacks);
}

} // namespace WebKit

#endif // ENABLE(PLUGIN_PROCESS)
