/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
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

#ifndef IDBIndexBackendImpl_h
#define IDBIndexBackendImpl_h

#include "IDBIndexBackendInterface.h"

#if ENABLE(INDEXED_DATABASE)

namespace WebCore {

class IDBKey;
class IDBObjectStoreBackendImpl;
class IDBSQLiteDatabase;
class SQLiteDatabase;
class ScriptExecutionContext;

class IDBIndexBackendImpl : public IDBIndexBackendInterface {
public:
    static PassRefPtr<IDBIndexBackendImpl> create(IDBSQLiteDatabase* database, int64_t id, const String& name, const String& storeName, const String& keyPath, bool unique)
    {
        return adoptRef(new IDBIndexBackendImpl(database, id, name, storeName, keyPath, unique));
    }
    static PassRefPtr<IDBIndexBackendImpl> create(IDBSQLiteDatabase* database, const String& name, const String& storeName, const String& keyPath, bool unique)
    {
        return adoptRef(new IDBIndexBackendImpl(database, name, storeName, keyPath, unique));
    }
    virtual ~IDBIndexBackendImpl();

    int64_t id() const
    {
        ASSERT(m_id != InvalidId);
        return m_id;
    }
    void setId(int64_t id) { m_id = id; }

    bool addingKeyAllowed(IDBKey*);

    // Implements IDBIndexBackendInterface.
    virtual String name() { return m_name; }
    virtual String storeName() { return m_storeName; }
    virtual String keyPath() { return m_keyPath; }
    virtual bool unique() { return m_unique; }

    virtual void openCursor(PassRefPtr<IDBKeyRange>, unsigned short direction, PassRefPtr<IDBCallbacks>, IDBTransactionBackendInterface*, ExceptionCode&);
    virtual void openKeyCursor(PassRefPtr<IDBKeyRange>, unsigned short direction, PassRefPtr<IDBCallbacks>, IDBTransactionBackendInterface*, ExceptionCode&);
    virtual void get(PassRefPtr<IDBKey>, PassRefPtr<IDBCallbacks>, IDBTransactionBackendInterface*, ExceptionCode&);
    virtual void getKey(PassRefPtr<IDBKey>, PassRefPtr<IDBCallbacks>, IDBTransactionBackendInterface*, ExceptionCode&);

private:
    IDBIndexBackendImpl(IDBSQLiteDatabase*, int64_t id, const String& name, const String& storeName, const String& keyPath, bool unique);
    IDBIndexBackendImpl(IDBSQLiteDatabase*, const String& name, const String& storeName, const String& keyPath, bool unique);

    SQLiteDatabase& sqliteDatabase() const;

    static void openCursorInternal(ScriptExecutionContext*, PassRefPtr<IDBIndexBackendImpl>, PassRefPtr<IDBKeyRange>, unsigned short direction, bool objectCursor, PassRefPtr<IDBCallbacks>, PassRefPtr<IDBTransactionBackendInterface>);
    static void getInternal(ScriptExecutionContext*, PassRefPtr<IDBIndexBackendImpl>, PassRefPtr<IDBKey>, bool getObject, PassRefPtr<IDBCallbacks>);

    static const int64_t InvalidId = 0;

    RefPtr<IDBSQLiteDatabase> m_database;

    int64_t m_id;
    String m_name;
    String m_storeName;
    String m_keyPath;
    bool m_unique;
};

} // namespace WebCore

#endif

#endif // IDBIndexBackendImpl_h
