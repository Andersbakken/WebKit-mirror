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

#include "WebContextMenuClient.h"

#include "WebContextMenuItemData.h"
#include "WebPage.h"
#include <WebCore/ContextMenu.h>
#include <WebCore/Frame.h>
#include <WebCore/Page.h>
#include <WebCore/UserGestureIndicator.h>

#define DISABLE_NOT_IMPLEMENTED_WARNINGS 1
#include "NotImplemented.h"

using namespace WebCore;

namespace WebKit {

void WebContextMenuClient::contextMenuDestroyed()
{
    delete this;
}

#if USE(CROSS_PLATFORM_CONTEXT_MENUS)
PassOwnPtr<ContextMenu> WebContextMenuClient::customizeMenu(PassOwnPtr<ContextMenu> menu)
{
    // WebKit2 ignores this client callback and does context menu customization when it is told to show the menu.
    return menu;
}
#else
PlatformMenuDescription WebContextMenuClient::getCustomMenuFromDefaultItems(ContextMenu* menu)
{
    // WebKit2 ignores this client callback and does context menu customization when it is told to show the menu.
    return menu->platformDescription();
}
#endif

void WebContextMenuClient::contextMenuItemSelected(ContextMenuItem*, const ContextMenu*)
{
    notImplemented();
}

void WebContextMenuClient::downloadURL(const KURL& url)
{
    // FIXME <rdar://problem/8750248> - Need the ability to start a Download from an arbitrary URL
    notImplemented();
}

void WebContextMenuClient::searchWithGoogle(const Frame* frame)
{
    String searchString = frame->editor()->selectedText();
    searchString.stripWhiteSpace();
    String encoded = encodeWithURLEscapeSequences(searchString);
    encoded.replace("%20", "+");
    
    String url("http://www.google.com/search?q=");
    url.append(encoded);
    url.append("&ie=UTF-8&oe=UTF-8");

    if (Page* page = frame->page()) {
        UserGestureIndicator indicator(DefinitelyProcessingUserGesture);
        page->mainFrame()->loader()->urlSelected(KURL(ParsedURLString, url), String(), 0, false, false, SendReferrer);
    }
}

} // namespace WebKit
