/*
 * Copyright (C) 2009 Google Inc. All rights reserved.
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

#ifndef BackForwardListClientImpl_h
#define BackForwardListClientImpl_h

#include "BackForwardListImpl.h"

namespace WebKit {
class WebViewImpl;

extern const char backForwardNavigationScheme[];

class BackForwardListClientImpl : public WebCore::BackForwardListClient {
public:
    BackForwardListClientImpl(WebViewImpl* webview);
    ~BackForwardListClientImpl();

    void setCurrentHistoryItem(WebCore::HistoryItem* item);
    WebCore::HistoryItem* previousHistoryItem() const;

private:
    // WebCore::BackForwardListClient methods:
    virtual void addItem(PassRefPtr<WebCore::HistoryItem>);
    virtual void goToItem(WebCore::HistoryItem*);
    virtual WebCore::HistoryItem* itemAtIndex(int index);
    virtual int backListCount();
    virtual int forwardListCount();
    virtual void close();

    WebViewImpl* m_webView;

    RefPtr<WebCore::HistoryItem> m_previousItem;
    RefPtr<WebCore::HistoryItem> m_currentItem;

    // The last history item that was accessed via itemAtIndex().  We keep track
    // of this until goToItem() is called, so we can track the navigation.
    RefPtr<WebCore::HistoryItem> m_pendingHistoryItem;
};

} // namespace WebKit

#endif
