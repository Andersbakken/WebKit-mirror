/*
 * Copyright (C) 2007 Alp Toker <alp@atoker.com>
 * Copyright (C) 2008 INdT - Instituto Nokia de Tecnologia
 * Copyright (C) 2009-2010 ProFUSION embedded systems
 * Copyright (C) 2009-2010 Samsung Electronics
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "Logging.h"

#include "PlatformString.h"

namespace WebCore {

void InitializeLoggingChannelsIfNecessary()
{
    static bool didInitializeLoggingChannels = false;
    if (didInitializeLoggingChannels)
        return;


    LogNotYetImplemented.state = WTFLogChannelOn;
    LogFrames.state = WTFLogChannelOn;
    LogLoading.state = WTFLogChannelOn;
    LogPopupBlocking.state = WTFLogChannelOn;
    LogEvents.state = WTFLogChannelOn;
    LogEditing.state = WTFLogChannelOn;
    LogLiveConnect.state = WTFLogChannelOn;
    LogIconDatabase.state = WTFLogChannelOn;
    LogSQLDatabase.state = WTFLogChannelOn;
    LogSpellingAndGrammar.state = WTFLogChannelOn;
    LogBackForward.state = WTFLogChannelOn;
    LogHistory.state = WTFLogChannelOn;
    LogPageCache.state = WTFLogChannelOn;
    LogPlatformLeaks.state = WTFLogChannelOn;
    LogResourceLoading.state = WTFLogChannelOn;
    LogNetwork.state = WTFLogChannelOn;
    LogFTP.state = WTFLogChannelOn;
    LogThreading.state = WTFLogChannelOn;
    LogStorageAPI.state = WTFLogChannelOn;
    LogMedia.state = WTFLogChannelOn;
    LogPlugins.state = WTFLogChannelOn;
    LogArchives.state = WTFLogChannelOn;
    LogProgress.state = WTFLogChannelOn;
    LogFileAPI.state = WTFLogChannelOn;

    didInitializeLoggingChannels = true;

    const char* logEnv = getenv("NF_WEBKIT_DEBUG");
    if (!logEnv) {
#if 1
        logEnv = "Events,Frames,Loading,Network,PageCache,PlatformLeaks,SQLDatabase,StorageAPI,Progress";
#else
        return;
#endif
    }

#if 0
    char** logv = eina_str_split(logEnv, ",", -1);

    for (int i = 0; logv[i]; i++) {
        if (WTFLogChannel* channel = getChannelFromName(logv[i]))
            channel->state = WTFLogChannelOn;
    }

    free(*logv);
    free(logv);
#endif


    // To disable logging notImplemented set the DISABLE_NI_WARNING
    // environment variable to 1.
    LogNotYetImplemented.state = WTFLogChannelOn;
}

}
