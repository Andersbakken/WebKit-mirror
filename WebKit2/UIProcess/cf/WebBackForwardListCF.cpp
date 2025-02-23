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

#include "WebBackForwardList.h"
#include "Logging.h"
#include <wtf/RetainPtr.h>
#include <CoreFoundation/CoreFoundation.h>

using namespace WebCore;

namespace WebKit {

DEFINE_STATIC_GETTER(CFStringRef, SessionHistoryCurrentIndexKey, (CFSTR("SessionHistoryCurrentIndex")));
DEFINE_STATIC_GETTER(CFStringRef, SessionHistoryEntriesKey, (CFSTR("SessionHistoryEntries")));
DEFINE_STATIC_GETTER(CFStringRef, SessionHistoryEntryTitleKey, (CFSTR("SessionHistoryEntryTitle")));
DEFINE_STATIC_GETTER(CFStringRef, SessionHistoryEntryURLKey, (CFSTR("SessionHistoryEntryURL")));

CFDictionaryRef WebBackForwardList::createCFDictionaryRepresentation(WebPageProxy::WebPageProxySessionStateFilterCallback filter, void* context) const
{
    RetainPtr<CFNumberRef> currentIndex(AdoptCF, CFNumberCreate(0, kCFNumberIntType, &m_current));    
    RetainPtr<CFMutableArrayRef> entries(AdoptCF, CFArrayCreateMutable(0, m_entries.size(), &kCFTypeArrayCallBacks));
    
    const void* keys[2] = { SessionHistoryCurrentIndexKey(), SessionHistoryEntriesKey() };
    const void* values[2] = { currentIndex.get(), entries.get() };

    RetainPtr<CFDictionaryRef> dictionary(AdoptCF, CFDictionaryCreate(0, keys, values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));

    for (size_t i = 0; i < m_entries.size(); ++i) {
        RefPtr<WebURL> webURL = WebURL::create(m_entries[i]->url());
        if (!filter(toAPI(m_page), WKPageGetSessionHistoryURLValueType(), toURLRef(m_entries[i]->originalURL().impl()), context))
            continue;
        
        RetainPtr<CFStringRef> url(AdoptCF, m_entries[i]->url().createCFString());
        RetainPtr<CFStringRef> title(AdoptCF, m_entries[i]->title().createCFString());
        
        const void* keys[2] = { SessionHistoryEntryURLKey(), SessionHistoryEntryTitleKey() };
        const void* values[2] = { url.get(), title.get() };

        RetainPtr<CFDictionaryRef> entryDictionary(AdoptCF, CFDictionaryCreate(0, keys, values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
        CFArrayAppendValue(entries.get(), entryDictionary.get());
    }
    
    return dictionary.leakRef();
}

bool WebBackForwardList::restoreFromCFDictionaryRepresentation(CFDictionaryRef dictionary)
{
    CFNumberRef cfIndex = (CFNumberRef)CFDictionaryGetValue(dictionary, SessionHistoryCurrentIndexKey());
    if (!cfIndex || CFGetTypeID(cfIndex) != CFNumberGetTypeID()) {
        LOG(SessionState, "WebBackForwardList dictionary representation does not have a valid current index");
        return false;
    }

    CFIndex currentIndex;
    if (!CFNumberGetValue(cfIndex, kCFNumberCFIndexType, &currentIndex)) {
        LOG(SessionState, "WebBackForwardList dictionary representation does not have a valid integer current index");
        return false;
    }

    CFArrayRef cfEntries = (CFArrayRef)CFDictionaryGetValue(dictionary, SessionHistoryEntriesKey());
    if (!cfEntries || CFGetTypeID(cfEntries) != CFArrayGetTypeID()) {
        LOG(SessionState, "WebBackForwardList dictionary representation does not have a valid list of entries");
        return false;
    }

    CFIndex size = CFArrayGetCount(cfEntries);
    for (CFIndex i = 0; i < size; ++i) {
        CFDictionaryRef entryDictionary = (CFDictionaryRef)CFArrayGetValueAtIndex(cfEntries, i);
        if (!entryDictionary || CFGetTypeID(entryDictionary) != CFDictionaryGetTypeID()) {
            LOG(SessionState, "WebBackForwardList entry array does not have a valid entry at index %i", (int)i);
            return false;
        }
        
        CFStringRef entryURL = (CFStringRef)CFDictionaryGetValue(entryDictionary, SessionHistoryEntryURLKey());
        if (!entryURL || CFGetTypeID(entryURL) != CFStringGetTypeID()) {
            LOG(SessionState, "WebBackForwardList entry at index %i does not have a valid URL", (int)i);
            return false;
        }

        CFStringRef entryTitle = (CFStringRef)CFDictionaryGetValue(entryDictionary, SessionHistoryEntryTitleKey());
        if (!entryTitle || CFGetTypeID(entryTitle) != CFStringGetTypeID()) {
            LOG(SessionState, "WebBackForwardList entry at index %i does not have a valid title", (int)i);
            return false;
        }
        
        // FIXME <rdar://problem/8261624> and https://bugs.webkit.org/show_bug.cgi?id=47355 - 
        // The data for the above entry needs to be added to the full back/forward list.
        // When we have a solution that restores the full back/forwardlist then causes a load of the current item,
        // we will no longer need this.
        if (i == currentIndex) {
            m_restoredCurrentURL = entryURL;
            break;
        }
    }
    return true;
}

} // namespace WebKit
