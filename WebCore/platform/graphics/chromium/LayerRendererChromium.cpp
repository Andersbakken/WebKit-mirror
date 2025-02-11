/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
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


#include "config.h"

#if USE(ACCELERATED_COMPOSITING)
#include "LayerRendererChromium.h"

#include "Canvas2DLayerChromium.h"
#include "GraphicsContext3D.h"
#include "LayerChromium.h"
#include "LayerTexture.h"
#include "NotImplemented.h"
#include "TextureManager.h"
#include "WebGLLayerChromium.h"
#if PLATFORM(SKIA)
#include "NativeImageSkia.h"
#include "PlatformContextSkia.h"
#elif PLATFORM(CG)
#include <CoreGraphics/CGBitmapContext.h>
#endif

namespace WebCore {

// FIXME: Make this limit adjustable and give it a useful value.
static size_t textureMemoryLimitBytes = 64 * 1024 * 1024;

static TransformationMatrix orthoMatrix(float left, float right, float bottom, float top)
{
    float deltaX = right - left;
    float deltaY = top - bottom;
    TransformationMatrix ortho;
    if (!deltaX || !deltaY)
        return ortho;
    ortho.setM11(2.0f / deltaX);
    ortho.setM41(-(right + left) / deltaX);
    ortho.setM22(2.0f / deltaY);
    ortho.setM42(-(top + bottom) / deltaY);

    // Z component of vertices is always set to zero as we don't use the depth buffer
    // while drawing.
    ortho.setM33(0);

    return ortho;
}

// Returns true if the matrix has no rotation, skew or perspective components to it.
static bool isScaleOrTranslation(const TransformationMatrix& m)
{
    return !m.m12() && !m.m13() && !m.m14()
           && !m.m21() && !m.m23() && !m.m24()
           && !m.m31() && !m.m32() && !m.m43()
           && m.m44();

}

bool LayerRendererChromium::compareLayerZ(const LayerChromium* a, const LayerChromium* b)
{
    return a->m_drawDepth < b->m_drawDepth;
}

PassRefPtr<LayerRendererChromium> LayerRendererChromium::create(PassRefPtr<GraphicsContext3D> context)
{
    if (!context)
        return 0;

    RefPtr<LayerRendererChromium> layerRenderer(adoptRef(new LayerRendererChromium(context)));
    if (!layerRenderer->hardwareCompositing())
        return 0;

    return layerRenderer.release();
}

LayerRendererChromium::LayerRendererChromium(PassRefPtr<GraphicsContext3D> context)
    : m_rootLayerTextureWidth(0)
    , m_rootLayerTextureHeight(0)
    , m_rootLayer(0)
    , m_scrollPosition(IntPoint(-1, -1))
    , m_currentShader(0)
    , m_currentRenderSurface(0)
    , m_offscreenFramebufferId(0)
    , m_context(context)
    , m_defaultRenderSurface(0)
{
    m_hardwareCompositing = initializeSharedObjects();
    m_rootLayerTiler = LayerTilerChromium::create(this, IntSize(256, 256));
    ASSERT(m_rootLayerTiler);
}

LayerRendererChromium::~LayerRendererChromium()
{
    cleanupSharedObjects();

    // Because the tilers need to clean up textures, clean them up explicitly
    // before the GraphicsContext3D is destroyed.
    m_rootLayerTiler.clear();
    m_horizontalScrollbarTiler.clear();
    m_verticalScrollbarTiler.clear();
}

GraphicsContext3D* LayerRendererChromium::context()
{
    return m_context.get();
}

void LayerRendererChromium::debugGLCall(GraphicsContext3D* context, const char* command, const char* file, int line)
{
    unsigned long error = context->getError();
    if (error != GraphicsContext3D::NO_ERROR)
        LOG_ERROR("GL command failed: File: %s\n\tLine %d\n\tcommand: %s, error %x\n", file, line, command, static_cast<int>(error));
}

void LayerRendererChromium::useShader(unsigned programId)
{
    if (programId != m_currentShader) {
        GLC(m_context.get(), m_context->useProgram(programId));
        m_currentShader = programId;
    }
}

IntRect LayerRendererChromium::verticalScrollbarRect(const IntRect& visibleRect, const IntRect& contentRect)
{
    IntRect verticalScrollbar(IntPoint(contentRect.right(), contentRect.y()), IntSize(visibleRect.width() - contentRect.width(), visibleRect.height()));
    return verticalScrollbar;
}

IntRect LayerRendererChromium::horizontalScrollbarRect(const IntRect& visibleRect, const IntRect& contentRect)
{
    IntRect horizontalScrollbar(IntPoint(contentRect.x(), contentRect.bottom()), IntSize(visibleRect.width(), visibleRect.height() - contentRect.height()));
    return horizontalScrollbar;
}

void LayerRendererChromium::invalidateRootLayerRect(const IntRect& dirtyRect, const IntRect& visibleRect, const IntRect& contentRect)
{
    if (contentRect.intersects(dirtyRect))
        m_rootLayerTiler->invalidateRect(dirtyRect);
    if (m_horizontalScrollbarTiler) {
        IntRect scrollbar = horizontalScrollbarRect(visibleRect, contentRect);
        if (dirtyRect.intersects(scrollbar)) {
            m_horizontalScrollbarTiler->setLayerPosition(scrollbar.location());
            m_horizontalScrollbarTiler->invalidateRect(dirtyRect);
        }
    }
    if (m_verticalScrollbarTiler) {
        IntRect scrollbar = verticalScrollbarRect(visibleRect, contentRect);
        if (dirtyRect.intersects(scrollbar)) {
            m_verticalScrollbarTiler->setLayerPosition(scrollbar.location());
            m_verticalScrollbarTiler->invalidateRect(dirtyRect);
        }
    }
}

void LayerRendererChromium::updateAndDrawRootLayer(TilePaintInterface& tilePaint, TilePaintInterface& scrollbarPaint, const IntRect& visibleRect, const IntRect& contentRect)
{
    // Mask out writes to alpha channel: subpixel antialiasing via Skia results in invalid
    // zero alpha values on text glyphs. The root layer is always opaque.
    GLC(m_context.get(), m_context->colorMask(true, true, true, false));

    m_rootLayerTiler->update(tilePaint, visibleRect);
    m_rootLayerTiler->draw(visibleRect);

    if (visibleRect.width() > contentRect.width()) {
        IntRect verticalScrollbar = verticalScrollbarRect(visibleRect, contentRect);
        IntSize tileSize = verticalScrollbar.size().shrunkTo(IntSize(m_maxTextureSize, m_maxTextureSize));
        if (!m_verticalScrollbarTiler)
            m_verticalScrollbarTiler = LayerTilerChromium::create(this, tileSize);
        else
            m_verticalScrollbarTiler->setTileSize(tileSize);
        m_verticalScrollbarTiler->setLayerPosition(verticalScrollbar.location());
        m_verticalScrollbarTiler->update(scrollbarPaint, visibleRect);
        m_verticalScrollbarTiler->draw(visibleRect);
    }

    if (visibleRect.height() > contentRect.height()) {
        IntRect horizontalScrollbar = horizontalScrollbarRect(visibleRect, contentRect);
        IntSize tileSize = horizontalScrollbar.size().shrunkTo(IntSize(m_maxTextureSize, m_maxTextureSize));
        if (!m_horizontalScrollbarTiler)
            m_horizontalScrollbarTiler = LayerTilerChromium::create(this, tileSize);
        else
            m_horizontalScrollbarTiler->setTileSize(tileSize);
        m_horizontalScrollbarTiler->setLayerPosition(horizontalScrollbar.location());
        m_horizontalScrollbarTiler->update(scrollbarPaint, visibleRect);
        m_horizontalScrollbarTiler->draw(visibleRect);
    }
}

void LayerRendererChromium::drawLayers(const IntRect& visibleRect, const IntRect& contentRect,
                                       const IntPoint& scrollPosition, TilePaintInterface& tilePaint,
                                       TilePaintInterface& scrollbarPaint)
{
    ASSERT(m_hardwareCompositing);

    if (!m_rootLayer)
        return;

    makeContextCurrent();

    // If the size of the visible area has changed then allocate a new texture
    // to store the contents of the root layer and adjust the projection matrix
    // and viewport.
    int visibleRectWidth = visibleRect.width();
    int visibleRectHeight = visibleRect.height();
    if (visibleRectWidth != m_rootLayerTextureWidth || visibleRectHeight != m_rootLayerTextureHeight) {
        m_rootLayerTextureWidth = visibleRectWidth;
        m_rootLayerTextureHeight = visibleRectHeight;

        // Reset the current render surface to force an update of the viewport and
        // projection matrix next time useRenderSurface is called.
        m_currentRenderSurface = 0;
    }

    // The GL viewport covers the entire visible area, including the scrollbars.
    GLC(m_context.get(), m_context->viewport(0, 0, visibleRectWidth, visibleRectHeight));

    // Bind the common vertex attributes used for drawing all the layers.
    LayerChromium::prepareForDraw(layerSharedValues());

    // FIXME: These calls can be made once, when the compositor context is initialized.
    GLC(m_context.get(), m_context->disable(GraphicsContext3D::DEPTH_TEST));
    GLC(m_context.get(), m_context->disable(GraphicsContext3D::CULL_FACE));

    // Blending disabled by default. Root layer alpha channel on Windows is incorrect when Skia uses ClearType.
    GLC(m_context.get(), m_context->disable(GraphicsContext3D::BLEND));

    m_scrollPosition = scrollPosition;

    m_defaultRenderSurface = m_rootLayer->m_renderSurface.get();
    if (!m_defaultRenderSurface)
        m_defaultRenderSurface = m_rootLayer->createRenderSurface();
    m_defaultRenderSurface->m_contentRect = IntRect(0, 0, m_rootLayerTextureWidth, m_rootLayerTextureHeight);

    useRenderSurface(m_defaultRenderSurface);

    // Clear to blue to make it easier to spot unrendered regions.
    m_context->clearColor(0, 0, 1, 1);
    m_context->clear(GraphicsContext3D::COLOR_BUFFER_BIT);

    updateAndDrawRootLayer(tilePaint, scrollbarPaint, visibleRect, contentRect);

    // Set the root visible/content rects --- used by subsequent drawLayers calls.
    m_rootVisibleRect = visibleRect;
    m_rootContentRect = contentRect;

    // Scissor out the scrollbars to avoid rendering on top of them.
    IntRect rootScissorRect(contentRect);
    // The scissorRect should not include the scroll offset.
    rootScissorRect.move(-m_scrollPosition.x(), -m_scrollPosition.y());
    m_rootLayer->m_scissorRect = rootScissorRect;

    Vector<LayerChromium*> renderSurfaceLayerList;
    renderSurfaceLayerList.append(m_rootLayer.get());

    TransformationMatrix identityMatrix;
    m_defaultRenderSurface->m_layerList.clear();
    updateLayersRecursive(m_rootLayer.get(), identityMatrix, renderSurfaceLayerList, m_defaultRenderSurface->m_layerList);

    // The shader used to render layers returns pre-multiplied alpha colors
    // so we need to send the blending mode appropriately.
    GLC(m_context.get(), m_context->enable(GraphicsContext3D::BLEND));
    GLC(m_context.get(), m_context->blendFunc(GraphicsContext3D::ONE, GraphicsContext3D::ONE_MINUS_SRC_ALPHA));
    GLC(m_context.get(), m_context->enable(GraphicsContext3D::SCISSOR_TEST));

    // Update the contents of the render surfaces. We traverse the array from
    // back to front to guarantee that nested render surfaces get rendered in the
    // correct order.
    for (int surfaceIndex = renderSurfaceLayerList.size() - 1; surfaceIndex >= 0 ; --surfaceIndex) {
        LayerChromium* renderSurfaceLayer = renderSurfaceLayerList[surfaceIndex];
        ASSERT(renderSurfaceLayer->m_renderSurface);

        // Render surfaces whose drawable area has zero width or height
        // will have no layers associated with them and should be skipped.
        if (!renderSurfaceLayer->m_renderSurface->m_layerList.size())
            continue;

        if (useRenderSurface(renderSurfaceLayer->m_renderSurface.get())) {
            if (renderSurfaceLayer != m_rootLayer) {
                GLC(m_context.get(), m_context->disable(GraphicsContext3D::SCISSOR_TEST));
                GLC(m_context.get(), m_context->clearColor(0, 0, 0, 0));
                GLC(m_context.get(), m_context->clear(GraphicsContext3D::COLOR_BUFFER_BIT));
                GLC(m_context.get(), m_context->enable(GraphicsContext3D::SCISSOR_TEST));
            }

            Vector<LayerChromium*>& layerList = renderSurfaceLayer->m_renderSurface->m_layerList;
            ASSERT(layerList.size());
            for (unsigned layerIndex = 0; layerIndex < layerList.size(); ++layerIndex)
                drawLayer(layerList[layerIndex], renderSurfaceLayer->m_renderSurface.get());
        }
    }

    GLC(m_context.get(), m_context->disable(GraphicsContext3D::SCISSOR_TEST));
    GLC(m_context.get(), m_context->disable(GraphicsContext3D::BLEND));
}

void LayerRendererChromium::finish()
{
    m_context->finish();
}

void LayerRendererChromium::present()
{
    // We're done! Time to swapbuffers!

    // Note that currently this has the same effect as swapBuffers; we should
    // consider exposing a different entry point on GraphicsContext3D.
    m_context->prepareTexture();
}

void LayerRendererChromium::setRootLayer(PassRefPtr<LayerChromium> layer)
{
    m_rootLayer = layer;
    m_rootLayerTiler->invalidateEntireLayer();
    if (m_horizontalScrollbarTiler)
        m_horizontalScrollbarTiler->invalidateEntireLayer();
    if (m_verticalScrollbarTiler)
        m_verticalScrollbarTiler->invalidateEntireLayer();
}

void LayerRendererChromium::getFramebufferPixels(void *pixels, const IntRect& rect)
{
    ASSERT(rect.right() <= rootLayerTextureSize().width()
           && rect.bottom() <= rootLayerTextureSize().height());

    if (!pixels)
        return;

    makeContextCurrent();

    GLC(m_context.get(), m_context->readPixels(rect.x(), rect.y(), rect.width(), rect.height(),
                                         GraphicsContext3D::RGBA, GraphicsContext3D::UNSIGNED_BYTE, pixels));
}

// FIXME: This method should eventually be replaced by a proper texture manager.
unsigned LayerRendererChromium::createLayerTexture()
{
    unsigned textureId = 0;
    GLC(m_context.get(), textureId = m_context->createTexture());
    GLC(m_context.get(), m_context->bindTexture(GraphicsContext3D::TEXTURE_2D, textureId));
    // Do basic linear filtering on resize.
    GLC(m_context.get(), m_context->texParameteri(GraphicsContext3D::TEXTURE_2D, GraphicsContext3D::TEXTURE_MIN_FILTER, GraphicsContext3D::LINEAR));
    GLC(m_context.get(), m_context->texParameteri(GraphicsContext3D::TEXTURE_2D, GraphicsContext3D::TEXTURE_MAG_FILTER, GraphicsContext3D::LINEAR));
    // NPOT textures in GL ES only work when the wrap mode is set to GraphicsContext3D::CLAMP_TO_EDGE.
    GLC(m_context.get(), m_context->texParameteri(GraphicsContext3D::TEXTURE_2D, GraphicsContext3D::TEXTURE_WRAP_S, GraphicsContext3D::CLAMP_TO_EDGE));
    GLC(m_context.get(), m_context->texParameteri(GraphicsContext3D::TEXTURE_2D, GraphicsContext3D::TEXTURE_WRAP_T, GraphicsContext3D::CLAMP_TO_EDGE));
    return textureId;
}

void LayerRendererChromium::deleteLayerTexture(unsigned textureId)
{
    if (!textureId)
        return;

    GLC(m_context.get(), m_context->deleteTexture(textureId));
}

// Returns true if any part of the layer falls within the visibleRect
bool LayerRendererChromium::isLayerVisible(LayerChromium* layer, const TransformationMatrix& matrix, const IntRect& visibleRect)
{
    // Form the matrix used by the shader to map the corners of the layer's
    // bounds into clip space.
    TransformationMatrix renderMatrix = matrix;
    renderMatrix.scale3d(layer->bounds().width(), layer->bounds().height(), 1);
    renderMatrix.multiply(m_projectionMatrix);

    FloatRect layerRect(-0.5, -0.5, 1, 1);
    FloatRect mappedRect = renderMatrix.mapRect(layerRect);

    // The layer is visible if it intersects any part of a rectangle whose origin
    // is at (-1, -1) and size is 2x2.
    return mappedRect.intersects(FloatRect(-1, -1, 2, 2));
}

// Recursively walks the layer tree starting at the given node and computes all the
// necessary transformations, scissor rectangles, render surfaces, etc.
void LayerRendererChromium::updateLayersRecursive(LayerChromium* layer, const TransformationMatrix& parentMatrix, Vector<LayerChromium*>& renderSurfaceLayerList, Vector<LayerChromium*>& layerList)
{
    layer->setLayerRenderer(this);

    // Compute the new matrix transformation that will be applied to this layer and
    // all its sublayers. It's important to remember that the layer's position
    // is the position of the layer's anchor point. Also, the coordinate system used
    // assumes that the origin is at the lower left even though the coordinates the browser
    // gives us for the layers are for the upper left corner. The Y flip happens via
    // the orthographic projection applied at render time.
    // The transformation chain for the layer is (using the Matrix x Vector order):
    // M = M[p] * Tr[l] * M[l] * Tr[c]
    // Where M[p] is the parent matrix passed down to the function
    //       Tr[l] is the translation matrix locating the layer's anchor point
    //       Tr[c] is the translation offset between the anchor point and the center of the layer
    //       M[l] is the layer's matrix (applied at the anchor point)
    // This transform creates a coordinate system whose origin is the center of the layer.
    // Note that the final matrix used by the shader for the layer is P * M * S . This final product
    // is computed in drawTexturedQuad().
    // Where: P is the projection matrix
    //        M is the layer's matrix computed above
    //        S is the scale adjustment (to scale up to the layer size)
    IntSize bounds = layer->bounds();
    FloatPoint anchorPoint = layer->anchorPoint();
    FloatPoint position = layer->position();

    // Offset between anchor point and the center of the quad.
    float centerOffsetX = (0.5 - anchorPoint.x()) * bounds.width();
    float centerOffsetY = (0.5 - anchorPoint.y()) * bounds.height();

    TransformationMatrix layerLocalTransform;
    // LT = Tr[l]
    layerLocalTransform.translate3d(position.x(), position.y(), layer->anchorPointZ());
    // LT = Tr[l] * M[l]
    layerLocalTransform.multLeft(layer->transform());
    // LT = Tr[l] * M[l] * Tr[c]
    layerLocalTransform.translate3d(centerOffsetX, centerOffsetY, -layer->anchorPointZ());

    TransformationMatrix combinedTransform = parentMatrix;
    combinedTransform = combinedTransform.multLeft(layerLocalTransform);

    FloatRect layerRect(-0.5 * layer->bounds().width(), -0.5 * layer->bounds().height(), layer->bounds().width(), layer->bounds().height());
    IntRect transformedLayerRect;

    // The layer and its descendants render on a new RenderSurface if any of
    // these conditions hold:
    // 1. The layer clips its descendants and its transform is not a simple translation.
    // 2. If the layer has opacity != 1 and does not have a preserves-3d transform style.
    // If a layer preserves-3d then we don't create a RenderSurface for it to avoid flattening
    // out its children. The opacity value of the children layers is multiplied by the opacity
    // of their parent.
    bool useSurfaceForClipping = layer->masksToBounds() && !isScaleOrTranslation(combinedTransform);
    bool useSurfaceForOpacity = layer->opacity() != 1 && !layer->preserves3D();
    if ((useSurfaceForClipping || useSurfaceForOpacity) && layer->descendantsDrawContent()) {
        RenderSurfaceChromium* renderSurface = layer->m_renderSurface.get();
        if (!renderSurface)
            renderSurface = layer->createRenderSurface();

        // The origin of the new surface is the upper left corner of the layer.
        layer->m_drawTransform = TransformationMatrix();
        layer->m_drawTransform.translate3d(0.5 * bounds.width(), 0.5 * bounds.height(), 0);

        transformedLayerRect = IntRect(0, 0, bounds.width(), bounds.height());

        // Layer's opacity will be applied when drawing the render surface.
        renderSurface->m_drawOpacity = layer->opacity();
        if (layer->superlayer()->preserves3D())
            renderSurface->m_drawOpacity *= layer->superlayer()->drawOpacity();
        layer->m_drawOpacity = 1;

        TransformationMatrix layerOriginTransform = combinedTransform;
        layerOriginTransform.translate3d(-0.5 * bounds.width(), -0.5 * bounds.height(), 0);
        renderSurface->m_originTransform = layerOriginTransform;
        if (layerOriginTransform.isInvertible() && layer->superlayer()) {
            TransformationMatrix parentToLayer = layerOriginTransform.inverse();

            layer->m_scissorRect = parentToLayer.mapRect(layer->superlayer()->m_scissorRect);
        } else
            layer->m_scissorRect = IntRect();

        // The render surface scissor rect is the scissor rect that needs to
        // be applied before drawing the render surface onto its containing
        // surface and is therefore expressed in the superlayer's coordinate system.
        renderSurface->m_scissorRect = layer->superlayer()->m_scissorRect;

        renderSurface->m_layerList.clear();

        renderSurfaceLayerList.append(layer);
    } else {
        // DT = M[p] * LT
        layer->m_drawTransform = combinedTransform;
        transformedLayerRect = enclosingIntRect(layer->m_drawTransform.mapRect(layerRect));

        layer->m_drawOpacity = layer->opacity();

        if (layer->superlayer()) {
            if (layer->superlayer()->preserves3D())
               layer->m_drawOpacity *= layer->superlayer()->m_drawOpacity;

            // Layers inherit the scissor rect from their superlayer.
            layer->m_scissorRect = layer->superlayer()->m_scissorRect;

            layer->m_targetRenderSurface = layer->superlayer()->m_targetRenderSurface;
        }

        if (layer != m_rootLayer)
            layer->m_renderSurface = 0;

        if (layer->masksToBounds())
            layer->m_scissorRect.intersect(transformedLayerRect);
    }

    if (layer->m_renderSurface)
        layer->m_targetRenderSurface = layer->m_renderSurface.get();
    else {
        ASSERT(layer->superlayer());
        layer->m_targetRenderSurface = layer->superlayer()->m_targetRenderSurface;
    }

    // m_drawableContentRect is always stored in the coordinate system of the
    // RenderSurface the layer draws into.
    if (layer->drawsContent())
        layer->m_drawableContentRect = transformedLayerRect;
    else
        layer->m_drawableContentRect = IntRect();

    TransformationMatrix sublayerMatrix = layer->m_drawTransform;

    // Flatten to 2D if the layer doesn't preserve 3D.
    if (!layer->preserves3D()) {
        sublayerMatrix.setM13(0);
        sublayerMatrix.setM23(0);
        sublayerMatrix.setM31(0);
        sublayerMatrix.setM32(0);
        sublayerMatrix.setM33(1);
        sublayerMatrix.setM34(0);
        sublayerMatrix.setM43(0);
    }

    // Apply the sublayer transform at the center of the layer.
    sublayerMatrix.multLeft(layer->sublayerTransform());

    // The origin of the sublayers is the top left corner of the layer, not the
    // center. The matrix passed down to the sublayers is therefore:
    // M[s] = M * Tr[-center]
    sublayerMatrix.translate3d(-bounds.width() * 0.5, -bounds.height() * 0.5, 0);

    Vector<LayerChromium*>& descendants = (layer->m_renderSurface ? layer->m_renderSurface->m_layerList : layerList);
    descendants.append(layer);
    unsigned thisLayerIndex = descendants.size() - 1;

    const Vector<RefPtr<LayerChromium> >& sublayers = layer->getSublayers();
    for (size_t i = 0; i < sublayers.size(); ++i) {
        LayerChromium* sublayer = sublayers[i].get();
        updateLayersRecursive(sublayer, sublayerMatrix, renderSurfaceLayerList, descendants);

        if (sublayer->m_renderSurface) {
            RenderSurfaceChromium* sublayerRenderSurface = sublayer->m_renderSurface.get();
            const IntRect& contentRect = sublayerRenderSurface->contentRect();
            FloatRect sublayerRect(-0.5 * contentRect.width(), -0.5 * contentRect.height(),
                                   contentRect.width(), contentRect.height());
            layer->m_drawableContentRect.unite(enclosingIntRect(sublayerRenderSurface->m_drawTransform.mapRect(sublayerRect)));
            descendants.append(sublayer);
        } else
            layer->m_drawableContentRect.unite(sublayer->m_drawableContentRect);
    }

    if (layer->masksToBounds())
        layer->m_drawableContentRect.intersect(transformedLayerRect);

    if (layer->m_renderSurface && layer != m_rootLayer) {
        RenderSurfaceChromium* renderSurface = layer->m_renderSurface.get();
        renderSurface->m_contentRect = layer->m_drawableContentRect;
        FloatPoint surfaceCenter = renderSurface->contentRectCenter();

        // Restrict the RenderSurface size to the portion that's visible.
        FloatSize centerOffsetDueToClipping;
        renderSurface->m_contentRect.intersect(layer->m_scissorRect);
        FloatPoint clippedSurfaceCenter = renderSurface->contentRectCenter();
        centerOffsetDueToClipping = clippedSurfaceCenter - surfaceCenter;

        // The RenderSurface backing texture cannot exceed the maximum supported
        // texture size.
        renderSurface->m_contentRect.setWidth(std::min(renderSurface->m_contentRect.width(), m_maxTextureSize));
        renderSurface->m_contentRect.setHeight(std::min(renderSurface->m_contentRect.height(), m_maxTextureSize));

        if (renderSurface->m_contentRect.isEmpty())
            renderSurface->m_layerList.clear();

        // Since the layer starts a new render surface we need to adjust its
        // scissor rect to be expressed in the new surface's coordinate system.
        layer->m_scissorRect = layer->m_drawableContentRect;

        // Adjust the origin of the transform to be the center of the render surface.
        renderSurface->m_drawTransform = renderSurface->m_originTransform;
        renderSurface->m_drawTransform.translate3d(surfaceCenter.x() + centerOffsetDueToClipping.width(), surfaceCenter.y() + centerOffsetDueToClipping.height(), 0);
    }

    // Compute the depth value of the center of the layer which will be used when
    // sorting the layers for the preserves-3d property.
    TransformationMatrix& layerDrawMatrix = layer->m_renderSurface ? layer->m_renderSurface->m_drawTransform : layer->m_drawTransform;
    if (layer->superlayer()) {
        if (!layer->superlayer()->preserves3D())
            layer->m_drawDepth = layer->superlayer()->m_drawDepth;
        else
            layer->m_drawDepth = layerDrawMatrix.m43();
    } else
        layer->m_drawDepth = 0;

    // If preserves-3d then sort all the descendants by the Z coordinate of their
    // center. If the preserves-3d property is also set on the superlayer then
    // skip the sorting as the superlayer will sort all the descendants anyway.
    if (layer->preserves3D() && (!layer->superlayer() || !layer->superlayer()->preserves3D()))
        std::stable_sort(&descendants.at(thisLayerIndex), descendants.end(), compareLayerZ);
}

bool LayerRendererChromium::useRenderSurface(RenderSurfaceChromium* renderSurface)
{
    if (m_currentRenderSurface == renderSurface)
        return true;

    m_currentRenderSurface = renderSurface;

    if (renderSurface == m_defaultRenderSurface) {
        GLC(m_context.get(), m_context->bindFramebuffer(GraphicsContext3D::FRAMEBUFFER, 0));
        setDrawViewportRect(renderSurface->m_contentRect, true);
        return true;
    }

    GLC(m_context.get(), m_context->bindFramebuffer(GraphicsContext3D::FRAMEBUFFER, m_offscreenFramebufferId));

    if (!renderSurface->prepareContentsTexture())
        return false;

    renderSurface->m_contentsTexture->framebufferTexture2D();

#if !defined ( NDEBUG )
    if (m_context->checkFramebufferStatus(GraphicsContext3D::FRAMEBUFFER) != GraphicsContext3D::FRAMEBUFFER_COMPLETE) {
        ASSERT_NOT_REACHED();
        return false;
    }
#endif

    setDrawViewportRect(renderSurface->m_contentRect, false);
    return true;
}

void LayerRendererChromium::drawLayer(LayerChromium* layer, RenderSurfaceChromium* targetSurface)
{
    if (layer->m_renderSurface && layer->m_renderSurface != targetSurface) {
        layer->m_renderSurface->draw();
        return;
    }

    if (layer->m_bounds.isEmpty())
        return;

    setScissorToRect(layer->m_scissorRect);

    // Check if the layer falls within the visible bounds of the page.
    IntRect layerRect = layer->getDrawRect();
    bool isLayerVisible = layer->m_scissorRect.intersects(layerRect);
    if (!isLayerVisible)
        return;

    // FIXME: Need to take into account the transform of the containing
    // RenderSurface here, otherwise single-sided layers that draw on
    // transformed surfaces won't always be culled properly.
    if (!layer->doubleSided() && layer->m_drawTransform.m33() < 0)
         return;

    if (layer->drawsContent()) {
        // Update the contents of the layer if necessary.
        layer->updateContentsIfDirty();
        m_context->makeContextCurrent();
        layer->draw();
    }

    // Draw the debug border if there is one.
    layer->drawDebugBorder();
}

// Sets the scissor region to the given rectangle. The coordinate system for the
// scissorRect has its origin at the top left corner of the current visible rect.
void LayerRendererChromium::setScissorToRect(const IntRect& scissorRect)
{
    // The scissor coordinates must be supplied in viewport space so we need to offset
    // by the relative position of the top left corner of the current render surface.
    int scissorX = scissorRect.x() - m_currentRenderSurface->m_contentRect.x();
    // When rendering to the default render surface we're rendering upside down so the top
    // of the GL scissor is the bottom of our layer.
    int scissorY;
    if (m_currentRenderSurface == m_defaultRenderSurface)
        scissorY = m_currentRenderSurface->m_contentRect.height() - (scissorRect.bottom() - m_currentRenderSurface->m_contentRect.y());
    else
        scissorY = scissorRect.y() - m_currentRenderSurface->m_contentRect.y();
    GLC(m_context.get(), m_context->scissor(scissorX, scissorY, scissorRect.width(), scissorRect.height()));
}

bool LayerRendererChromium::makeContextCurrent()
{
    m_context->makeContextCurrent();
    return true;
}

// Checks whether a given size is within the maximum allowed texture size range.
bool LayerRendererChromium::checkTextureSize(const IntSize& textureSize)
{
    if (textureSize.width() > m_maxTextureSize || textureSize.height() > m_maxTextureSize)
        return false;
    return true;
}

// Sets the coordinate range of content that ends being drawn onto the target render surface.
// The target render surface is assumed to have an origin at 0, 0 and the width and height of
// of the drawRect.
void LayerRendererChromium::setDrawViewportRect(const IntRect& drawRect, bool flipY)
{
    if (flipY)
        m_projectionMatrix = orthoMatrix(drawRect.x(), drawRect.right(), drawRect.bottom(), drawRect.y());
    else
        m_projectionMatrix = orthoMatrix(drawRect.x(), drawRect.right(), drawRect.y(), drawRect.bottom());
    GLC(m_context.get(), m_context->viewport(0, 0, drawRect.width(), drawRect.height()));
}



void LayerRendererChromium::resizeOnscreenContent(const IntSize& size)
{
    if (m_context)
        m_context->reshape(size.width(), size.height());
}

bool LayerRendererChromium::initializeSharedObjects()
{
    makeContextCurrent();

    // Get the max texture size supported by the system.
    m_maxTextureSize = 0;
    GLC(m_context.get(), m_context->getIntegerv(GraphicsContext3D::MAX_TEXTURE_SIZE, &m_maxTextureSize));

    // Create an FBO for doing offscreen rendering.
    GLC(m_context.get(), m_offscreenFramebufferId = m_context->createFramebuffer());

    m_layerSharedValues = adoptPtr(new LayerChromium::SharedValues(m_context.get()));
    m_contentLayerSharedValues = adoptPtr(new ContentLayerChromium::SharedValues(m_context.get()));
    m_canvasLayerSharedValues = adoptPtr(new CanvasLayerChromium::SharedValues(m_context.get()));
    m_videoLayerSharedValues = adoptPtr(new VideoLayerChromium::SharedValues(m_context.get()));
    m_pluginLayerSharedValues = adoptPtr(new PluginLayerChromium::SharedValues(m_context.get()));
    m_renderSurfaceSharedValues = adoptPtr(new RenderSurfaceChromium::SharedValues(m_context.get()));

    if (!m_layerSharedValues->initialized() || !m_contentLayerSharedValues->initialized() || !m_canvasLayerSharedValues->initialized()
        || !m_videoLayerSharedValues->initialized() || !m_pluginLayerSharedValues->initialized() || !m_renderSurfaceSharedValues->initialized()) {
        cleanupSharedObjects();
        return false;
    }

    m_textureManager = TextureManager::create(m_context.get(), textureMemoryLimitBytes, m_maxTextureSize);
    return true;
}

void LayerRendererChromium::cleanupSharedObjects()
{
    makeContextCurrent();

    m_layerSharedValues.clear();
    m_contentLayerSharedValues.clear();
    m_canvasLayerSharedValues.clear();
    m_videoLayerSharedValues.clear();
    m_pluginLayerSharedValues.clear();
    m_renderSurfaceSharedValues.clear();
    if (m_offscreenFramebufferId)
        GLC(m_context.get(), m_context->deleteFramebuffer(m_offscreenFramebufferId));

    m_textureManager.clear();
}

} // namespace WebCore

#endif // USE(ACCELERATED_COMPOSITING)
