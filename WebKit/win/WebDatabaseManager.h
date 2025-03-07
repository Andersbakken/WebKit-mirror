/*
 * Copyright (C) 2007, 2008 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WebDatabaseManager_h
#define WebDatabaseManager_h

#if ENABLE(DATABASE)

#include <WebCore/DatabaseTrackerClient.h>

#include "WebKit.h"

class WebDatabaseManager : public IWebDatabaseManager, private WebCore::DatabaseTrackerClient {
public:
    static WebDatabaseManager* createInstance();

    // IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ void** ppvObject);

    virtual ULONG STDMETHODCALLTYPE AddRef();

    virtual ULONG STDMETHODCALLTYPE Release();

    // IWebDatabaseManager
    virtual HRESULT STDMETHODCALLTYPE sharedWebDatabaseManager( 
        /* [retval][out] */ IWebDatabaseManager** result);
        
    virtual HRESULT STDMETHODCALLTYPE origins( 
        /* [retval][out] */ IEnumVARIANT** result);
        
    virtual HRESULT STDMETHODCALLTYPE databasesWithOrigin( 
        /* [in] */ IWebSecurityOrigin* origin,
        /* [retval][out] */ IEnumVARIANT** result);
        
    virtual HRESULT STDMETHODCALLTYPE detailsForDatabase( 
        /* [in] */ BSTR databaseName,
        /* [in] */ IWebSecurityOrigin* origin,
        /* [retval][out] */ IPropertyBag** result);
        
    virtual HRESULT STDMETHODCALLTYPE deleteAllDatabases();
        
    virtual HRESULT STDMETHODCALLTYPE deleteOrigin( 
        /* [in] */ IWebSecurityOrigin* origin);
        
    virtual HRESULT STDMETHODCALLTYPE deleteDatabase( 
        /* [in] */ BSTR databaseName,
        /* [in] */ IWebSecurityOrigin* origin);

    virtual HRESULT STDMETHODCALLTYPE setQuota(
        /* [in] */ BSTR origin,
        /* [in] */ unsigned long long quota);

    // DatabaseTrackerClient
    virtual void dispatchDidModifyOrigin(WebCore::SecurityOrigin*);
    virtual void dispatchDidModifyDatabase(WebCore::SecurityOrigin*, const WTF::String& databaseName);

private:
    WebDatabaseManager();
    ~WebDatabaseManager();

    ULONG m_refCount;
};

void WebKitInitializeWebDatabasesIfNecessary();

#endif

#endif
