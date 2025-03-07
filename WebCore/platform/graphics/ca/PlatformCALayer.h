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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
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

#ifndef PlatformCALayer_h
#define PlatformCALayer_h

#if USE(ACCELERATED_COMPOSITING)

#include "GraphicsContext.h"
#include "GraphicsLayerCA.h"
#include "PlatformCAAnimation.h"
#include "PlatformString.h"
#include <wtf/HashMap.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/RetainPtr.h>
#include <wtf/Vector.h>
#include <wtf/text/StringHash.h>

namespace WebCore {

class PlatformCALayer;

typedef Vector<RefPtr<PlatformCALayer> > PlatformCALayerList;

class PlatformCALayer : public RefCounted<PlatformCALayer> {
public:
    // TiledLayer used in GraphicsLayer has constant settings:
    //      cTiledLayerTileSize = 512
    //      setTileSize:(cTiledLayerTileSize, cTiledLayerTileSize)
    //      setLevelsOfDetail:1
    //      setLevelsOfDetailBias:0
    //      setContentsGravity:@"bottomLeft"
    //
    // TiledLayer also has drawing functions like WebLayer
    //
    // WebLayer is a CALayer with drawing functions specific to WebKit
    //
    // Layer and TransformLayer are used as is

    enum LayerType { LayerTypeLayer, LayerTypeWebLayer, LayerTypeTransformLayer, LayerTypeWebTiledLayer, LayerTypeCustom };
    enum FilterType { Linear, Nearest, Trilinear };

    static PassRefPtr<PlatformCALayer> create(LayerType, GraphicsLayerCA*);
    
    // This function passes the layer as a void* rather than a PlatformLayer because PlatformLayer
    // is defined differently for Obj C and C++. This allows callers from both languages.
    static PassRefPtr<PlatformCALayer> create(void* platformLayer, GraphicsLayerCA*);

    ~PlatformCALayer();
    
    // This function passes the layer as a void* rather than a PlatformLayer because PlatformLayer
    // is defined differently for Obj C and C++. This allows callers from both languages.
    static PlatformCALayer* platformCALayer(void* platformLayer);
    
    PlatformLayer* platformLayer() const;
    
    static bool isValueFunctionSupported();
    
    GraphicsLayerCA* owner() const { return m_owner; }
    void setOwner(GraphicsLayerCA* owner);

    void animationStarted(CFTimeInterval beginTime)
    {
        if (m_owner)
            m_owner->animationStarted(beginTime);
    }

    void setNeedsDisplay(const FloatRect* dirtyRect = 0);
    
    void setContentsChanged();
    
    LayerType layerType() const { return m_layerType; }

    PlatformCALayer* superlayer() const;
    void removeFromSuperlayer();
    void setSublayers(const PlatformCALayerList&);
    void removeAllSublayers();
    void appendSublayer(PlatformCALayer*);
    void insertSublayer(PlatformCALayer*, size_t index);
    void replaceSublayer(PlatformCALayer* reference, PlatformCALayer*);
    size_t sublayerCount() const;

    // This method removes the sublayers from the source and reparents them to the current layer.
    void adoptSublayers(PlatformCALayer* source);
    
    void addAnimationForKey(const String& key, PlatformCAAnimation* animation);    
    void removeAnimationForKey(const String& key);
    PassRefPtr<PlatformCAAnimation> animationForKey(const String& key);
    
    PlatformCALayer* mask() const;
    void setMask(PlatformCALayer*);
    
    bool isOpaque() const;
    void setOpaque(bool);

    FloatRect bounds() const;
    void setBounds(const FloatRect&);

    FloatPoint3D position() const;
    void setPosition(const FloatPoint3D&);
    void setPosition(const FloatPoint& pos) { setPosition(FloatPoint3D(pos.x(), pos.y(), 0)); }

    FloatPoint3D anchorPoint() const;
    void setAnchorPoint(const FloatPoint3D&);

    TransformationMatrix transform() const;
    void setTransform(const TransformationMatrix&);

    TransformationMatrix sublayerTransform() const;
    void setSublayerTransform(const TransformationMatrix&);

    TransformationMatrix contentsTransform() const;
    void setContentsTransform(const TransformationMatrix&);

    bool isHidden() const;
    void setHidden(bool);

    bool isGeometryFlipped() const;
    void setGeometryFlipped(bool);

    bool isDoubleSided() const;
    void setDoubleSided(bool);

    bool masksToBounds() const;
    void setMasksToBounds(bool);
    
    bool acceleratesDrawing() const;
    void setAcceleratesDrawing(bool);

    void* contents() const;
    void setContents(void*);

    FloatRect contentsRect() const;
    void setContentsRect(const FloatRect&);

    void setMinificationFilter(FilterType);
    void setMagnificationFilter(FilterType);

    Color backgroundColor() const;
    void setBackgroundColor(const Color&);

    float borderWidth() const;
    void setBorderWidth(float);

    Color borderColor() const;
    void setBorderColor(const Color&);

    float opacity() const;
    void setOpacity(float);

    String name() const;
    void setName(const String&);

    FloatRect frame() const;
    void setFrame(const FloatRect&);

    float speed() const;
    void setSpeed(float);

    CFTimeInterval timeOffset() const;
    void setTimeOffset(CFTimeInterval);

protected:
    PlatformCALayer(LayerType, PlatformLayer*, GraphicsLayerCA*);
    
private:
    
    GraphicsLayerCA* m_owner;
    LayerType m_layerType;
    
#if PLATFORM(MAC) || PLATFORM(WIN)
    RetainPtr<PlatformLayer> m_layer;
#endif

#if PLATFORM(MAC)
#ifdef __OBJC__
    RetainPtr<NSObject> m_delegate;
#else
    RetainPtr<void> m_delegate;
#endif
#endif
};

}

#endif // USE(ACCELERATED_COMPOSITING)

#endif // PlatformCALayer_h
