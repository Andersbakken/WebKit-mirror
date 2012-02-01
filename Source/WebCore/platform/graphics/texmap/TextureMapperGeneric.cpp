/*
 Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public
 License as published by the Free Software Foundation; either
 version 2 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.

 You should have received a copy of the GNU Library General Public License
 along with this library; see the file COPYING.LIB.  If not, write to
 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "TextureMapperGeneric.h"

#include "PassOwnPtr.h"
#include "assert.h"

namespace WebCore {

BitmapTextureGeneric::BitmapTextureGeneric()
{

}


void BitmapTextureGeneric::destroy()
{
    m_image = nullptr;
}

void BitmapTextureGeneric::reset(const IntSize& size, bool isOpaque)
{
    BitmapTexture::reset(size, isOpaque);

    if (!m_image || size.width() > m_image->size().width() || size.height() > m_image->size().height() )
        m_image = ImageBuffer::create(size, WebCore::ColorSpaceDeviceRGB, WebCore::Accelerated);
#if 0
    if (!isOpaque)
        m_pixmap.fill(Qt::transparent);
#endif
}

PlatformGraphicsContext* BitmapTextureGeneric::beginPaint(const IntRect& dirtyRect)
{
    GraphicsContext* ctx = m_image->context();
    if (ctx) {
        ctx->clearRect(dirtyRect);
        return ctx->platformContext();
    }
    return 0;
}

void BitmapTextureGeneric::endPaint()
{
}

bool BitmapTextureGeneric::save(const String& path)
{
    return false;
}

void BitmapTextureGeneric::setContentsToImage(Image* image)
{
    if (!image)
        return;
    BitmapTexture::reset(image->size(), true);
    GraphicsContext *context = m_image->context();
    assert(context);
    if(context)
        context->drawImage(image, WebCore::ColorSpaceDeviceRGB, IntPoint(0, 0), CompositeCopy);
}

void BitmapTextureGeneric::updateContents(PixelFormat format, const IntRect& rect, void* bits)
{
    (void)format;
    (void)rect;
    (void)bits;
}

void TextureMapperGeneric::beginClip(const TransformationMatrix& matrix, const FloatRect& rect)
{
    assert(matrix.isAffine());
    GraphicsContext *context = currentContext();
    assert(context);
    if(context) {
        const AffineTransform prevTransform = context->getCTM();
        context->save();
        context->setCTM(matrix.toAffineTransform());
        context->clip(rect);
        context->setCTM(prevTransform);
    }
}

void TextureMapperGeneric::endClip()
{
    GraphicsContext *context = currentContext();
    assert(context);
    if(context)
        context->restore();
}

TextureMapperGeneric::TextureMapperGeneric() : m_currentSurface(0)
{
}

void TextureMapperGeneric::setGraphicsContext(GraphicsContext* context)
{
    m_context = context;
}

GraphicsContext* TextureMapperGeneric::graphicsContext()
{
    return m_context;
}

void TextureMapperGeneric::bindSurface(BitmapTexture* surface)
{
    BitmapTextureGeneric* surfaceGeneric = static_cast<BitmapTextureGeneric*>(surface);
    m_currentSurface = surfaceGeneric;
}

GraphicsContext *TextureMapperGeneric::currentContext() const {

    GraphicsContext *ret = m_context;
    if (m_currentSurface)
        ret = m_currentSurface->m_image->context();
    return ret;
}

void TextureMapperGeneric::drawTexture(const BitmapTexture& texture, const FloatRect& targetRect, const TransformationMatrix& matrix,
                                       float opacity, const BitmapTexture* maskTexture)
{
    const BitmapTextureGeneric& textureGeneric = static_cast<const BitmapTextureGeneric&>(texture);
    ImageBuffer *image = textureGeneric.m_image.get();

    GraphicsContext *context = currentContext();
    assert(context);
    if(context) {
        assert(matrix.isAffine());
        context->save();
        context->setAlpha(opacity);
        context->setCTM(matrix.toAffineTransform());
        if (maskTexture && maskTexture->isValid()) {
            const BitmapTextureGeneric* mask = static_cast<const BitmapTextureGeneric*>(maskTexture);
            context->clipToImageBuffer(mask->m_image.get(), targetRect);
        }
        context->drawImageBuffer(image, WebCore::ColorSpaceDeviceRGB, targetRect, FloatRect(textureGeneric.sourceRect()));
        context->restore();
    }
}

PassOwnPtr<TextureMapper> TextureMapper::create(GraphicsContext*)
{
    return adoptPtr(new TextureMapperGeneric);
}

PassRefPtr<BitmapTexture> TextureMapperGeneric::createTexture()
{
    return adoptRef(new BitmapTextureGeneric());
}

void TextureMapperGeneric::beginPainting()
{
    m_context->save();
}

void TextureMapperGeneric::endPainting()
{
    m_context->restore();
}

};
