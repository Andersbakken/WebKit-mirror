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

#include "texmap/TextureMapper.h"

#include "RefPtr.h"
#include "ImageBuffer.h"
#include "GraphicsContext.h"

#ifndef TextureMapperGeneric_h
#define TextureMapperGeneric_h

namespace WebCore {

class BitmapTextureGeneric : public BitmapTexture {
    friend class TextureMapperGeneric;
public:
    BitmapTextureGeneric();
    ~BitmapTextureGeneric() { destroy(); }
    virtual void destroy();
    virtual IntSize size() const { return m_image->size(); }
    virtual void reset(const IntSize&, bool opaque);
    virtual GraphicsContext* beginPaint(const IntRect& dirtyRect);
    virtual void endPaint();
    virtual void setContentsToImage(Image*);
    virtual bool save(const String& path);
    virtual bool isValid() const { return m_image; }
    IntRect sourceRect() const { return IntRect(0, 0, contentSize().width(), contentSize().height()); }
    virtual void pack() { }
    virtual void unpack() { }
    virtual bool isPacked() const { return false; }

private:
    OwnPtr<ImageBuffer> m_image;
};

class TextureMapperGeneric : public TextureMapper {
public:
    TextureMapperGeneric();

    virtual void drawTexture(const BitmapTexture&, const FloatRect& targetRect, const TransformationMatrix&, float opacity, const BitmapTexture* maskTexture);
    virtual void bindSurface(BitmapTexture* surface);
    virtual void beginClip(const TransformationMatrix&, const FloatRect&);
    virtual void endClip();
    virtual void setGraphicsContext(GraphicsContext*);
    virtual GraphicsContext* graphicsContext();
    virtual bool allowSurfaceForRoot() const { return false; }
    virtual PassRefPtr<BitmapTexture> createTexture();
    virtual void beginPainting();
    virtual void endPainting();

    static PassOwnPtr<TextureMapper> create() { return adoptPtr(new TextureMapperGeneric); }
private:
    GraphicsContext *currentContext() const;
    GraphicsContext* m_context;
    RefPtr<BitmapTextureGeneric> m_currentSurface;
};

}
#endif
