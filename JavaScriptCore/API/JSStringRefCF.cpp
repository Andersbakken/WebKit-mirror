/*
 * Copyright (C) 2006, 2007 Apple Computer, Inc.  All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "JSStringRefCF.h"

#include "APICast.h"
#include "InitializeThreading.h"
#include "JSStringRef.h"
#include "OpaqueJSString.h"
#include <runtime/UString.h>
#include <runtime/JSValue.h>
#include <wtf/OwnArrayPtr.h>

JSStringRef JSStringCreateWithCFString(CFStringRef string)
{
    JSC::initializeThreading();

    // We cannot use CFIndex here since CFStringGetLength can return values larger than
    // it can hold.  (<rdar://problem/6806478>)
    size_t length = CFStringGetLength(string);
    if (length) {
        OwnArrayPtr<UniChar> buffer(new UniChar[length]);
        CFStringGetCharacters(string, CFRangeMake(0, length), buffer.get());
        COMPILE_ASSERT(sizeof(UniChar) == sizeof(UChar), unichar_and_uchar_must_be_same_size);
        return OpaqueJSString::create(reinterpret_cast<UChar*>(buffer.get()), length).leakRef();
    } else {
        return OpaqueJSString::create(0, 0).leakRef();
    }
}

CFStringRef JSStringCopyCFString(CFAllocatorRef alloc, JSStringRef string)
{
    return CFStringCreateWithCharacters(alloc, reinterpret_cast<const UniChar*>(string->characters()), string->length());
}
