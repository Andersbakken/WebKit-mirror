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

#include "AuthenticationManager.h"

#include "MessageID.h"
#include "WebCoreArgumentCoders.h"
#include "WebFrame.h"
#include "WebPage.h"
#include "WebPageProxyMessages.h"
#include "WebProcess.h"
#include <WebCore/AuthenticationChallenge.h>
#include <WebCore/AuthenticationClient.h>

namespace WebKit {

static uint64_t generateAuthenticationChallengeID()
{
    static uint64_t uniqueAuthenticationChallengeID = 1;
    return uniqueAuthenticationChallengeID++;
}

AuthenticationManager& AuthenticationManager::shared()
{
    static AuthenticationManager& manager = *new AuthenticationManager;
    return manager;
}

AuthenticationManager::AuthenticationManager()
{
}

void AuthenticationManager::didReceiveMessage(CoreIPC::Connection* connection, CoreIPC::MessageID messageID, CoreIPC::ArgumentDecoder* arguments)
{
    didReceiveAuthenticationManagerMessage(connection, messageID, arguments);
}

void AuthenticationManager::didReceiveAuthenticationChallenge(WebFrame* frame, const WebCore::AuthenticationChallenge& authenticationChallenge)
{
    ASSERT(frame);
    ASSERT(frame->page());

    uint64_t id = generateAuthenticationChallengeID();
    m_challenges.set(id, authenticationChallenge);    
    
    WebProcess::shared().connection()->send(Messages::WebPageProxy::DidReceiveAuthenticationChallenge(frame->frameID(), authenticationChallenge, id), frame->page()->pageID());
}

void AuthenticationManager::useCredentialForChallenge(uint64_t challengeID, const WebCore::Credential& credential)
{
    WebCore::AuthenticationChallenge challenge = m_challenges.take(challengeID);
    ASSERT(!challenge.isNull());
    WebCore::AuthenticationClient* coreClient = challenge.authenticationClient();
    if (!coreClient)
        return;

    coreClient->receivedCredential(challenge, credential);
}

void AuthenticationManager::continueWithoutCredentialForChallenge(uint64_t challengeID)
{
    WebCore::AuthenticationChallenge challenge = m_challenges.take(challengeID);
    ASSERT(!challenge.isNull());
    WebCore::AuthenticationClient* coreClient = challenge.authenticationClient();
    if (!coreClient)
        return;

    coreClient->receivedRequestToContinueWithoutCredential(challenge);
}

void AuthenticationManager::cancelChallenge(uint64_t challengeID)
{
    WebCore::AuthenticationChallenge challenge = m_challenges.take(challengeID);
    ASSERT(!challenge.isNull());
    WebCore::AuthenticationClient* coreClient = challenge.authenticationClient();
    if (!coreClient)
        return;

    coreClient->receivedCancellation(challenge);
}

} // namespace WebKit
