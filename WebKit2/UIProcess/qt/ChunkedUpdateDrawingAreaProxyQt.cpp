/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
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

#include "ChunkedUpdateDrawingAreaProxy.h"

#include "DrawingAreaMessageKinds.h"
#include "DrawingAreaProxyMessageKinds.h"
#include "UpdateChunk.h"
#include "WKAPICast.h"
#include "WebPageProxy.h"
#include "qgraphicswkview.h"
#include <QPainter>

using namespace WebCore;

namespace WebKit {

WebPageProxy* ChunkedUpdateDrawingAreaProxy::page()
{
    return toImpl(m_webView->page()->pageRef());
}

void ChunkedUpdateDrawingAreaProxy::ensureBackingStore()
{
    if (!m_backingStoreImage.isNull())
        return;

    m_backingStoreImage = QImage(size().width(), size().height(), QImage::Format_RGB32);
}

void ChunkedUpdateDrawingAreaProxy::invalidateBackingStore()
{
    m_backingStoreImage = QImage();
}

void ChunkedUpdateDrawingAreaProxy::platformPaint(const IntRect& rect, QPainter* painter)
{
    if (m_backingStoreImage.isNull())
        return;

    painter->drawImage(QPoint(0, 0), m_backingStoreImage);
}

void ChunkedUpdateDrawingAreaProxy::drawUpdateChunkIntoBackingStore(UpdateChunk* updateChunk)
{
    ensureBackingStore();

    QImage image(updateChunk->createImage());
    const IntRect& updateChunkRect = updateChunk->rect();

    QPainter painter(&m_backingStoreImage);
    painter.drawImage(updateChunkRect.topLeft(), image);

    m_webView->update(QRect(updateChunkRect));
}

} // namespace WebKit
