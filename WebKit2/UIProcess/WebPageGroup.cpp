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

#include "WebPageGroup.h"

#include "WebPageProxy.h"
#include "WebPreferences.h"
#include <wtf/HashMap.h>
#include <wtf/text/StringConcatenate.h>

namespace WebKit {

static uint64_t generatePageGroupID()
{
    static uint64_t uniquePageGroupID = 1;
    return uniquePageGroupID++;
}

typedef HashMap<uint64_t, WebPageGroup*> WebPageGroupMap;

static WebPageGroupMap& webPageGroupMap()
{
    static WebPageGroupMap map;
    return map;
}

PassRefPtr<WebPageGroup> WebPageGroup::create(const String& identifier, bool visibleToInjectedBundle)
{
    RefPtr<WebPageGroup> pageGroup = adoptRef(new WebPageGroup(identifier, visibleToInjectedBundle));

    webPageGroupMap().set(pageGroup->pageGroupID(), pageGroup.get());

    return pageGroup.release();
}

WebPageGroup* WebPageGroup::get(uint64_t pageGroupID)
{
    return webPageGroupMap().get(pageGroupID);
}

WebPageGroup::WebPageGroup(const String& identifier, bool visibleToInjectedBundle)
{
    m_data.pageGroupID = generatePageGroupID();

    if (!identifier.isNull()) {
        m_data.identifer = identifier;
        m_preferences = WebPreferences::create(identifier);
    } else {
        m_data.identifer = m_data.identifer = makeString("__uniquePageGroupID-", String::number(m_data.pageGroupID));
        m_preferences = WebPreferences::create();
    }
    m_preferences->addPageGroup(this);

    m_data.visibleToInjectedBundle = visibleToInjectedBundle;    
}

WebPageGroup::~WebPageGroup()
{
    m_preferences->removePageGroup(this);
    webPageGroupMap().remove(pageGroupID());
}

void WebPageGroup::addPage(WebPageProxy* page)
{
    m_pages.add(page);
}

void WebPageGroup::removePage(WebPageProxy* page)
{
    m_pages.remove(page);
}

void WebPageGroup::setPreferences(WebPreferences* preferences)
{
    if (preferences == m_preferences)
        return;

    m_preferences->removePageGroup(this);
    m_preferences = preferences;
    m_preferences->addPageGroup(this);

    preferencesDidChange();
}

WebPreferences* WebPageGroup::preferences() const
{
    return m_preferences.get();
}

void WebPageGroup::preferencesDidChange()
{
    for (HashSet<WebPageProxy*>::iterator it = m_pages.begin(), end = m_pages.end(); it != end; ++it) {
        WebPageProxy* page = *it;
        page->preferencesDidChange();
    }
}

} // namespace WebKit
