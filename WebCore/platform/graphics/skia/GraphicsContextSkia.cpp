/*
 * Copyright (c) 2006, Google Inc. All rights reserved.
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
#include "GraphicsContext.h"

#include "AffineTransform.h"
#include "Color.h"
#include "FloatRect.h"
#include "GLES2Canvas.h"
#include "Gradient.h"
#include "GraphicsContextPlatformPrivate.h"
#include "ImageBuffer.h"
#include "IntRect.h"
#include "NativeImageSkia.h"
#include "NotImplemented.h"
#include "PlatformContextSkia.h"

#include "SkBitmap.h"
#include "SkBlurDrawLooper.h"
#include "SkCornerPathEffect.h"
#include "SkShader.h"
#include "SkiaUtils.h"
#include "skia/ext/platform_canvas.h"

#include <math.h>
#include <wtf/Assertions.h>
#include <wtf/MathExtras.h>
#include <wtf/UnusedParam.h>

using namespace std;

namespace WebCore {

namespace {

inline int fastMod(int value, int max)
{
    int sign = SkExtractSign(value);

    value = SkApplySign(value, sign);
    if (value >= max)
        value %= max;
    return SkApplySign(value, sign);
}

inline float square(float n)
{
    return n * n;
}

}  // namespace

// "Seatbelt" functions ------------------------------------------------------
//
// These functions check certain graphics primitives for being "safe".
// Skia has historically crashed when sent crazy data. These functions do
// additional checking to prevent crashes.
//
// Ideally, all of these would be fixed in the graphics layer and we would not
// have to do any checking. You can uncomment the ENSURE_VALUE_SAFETY_FOR_SKIA
// flag to check the graphics layer.

// Disabling these checks (20/01/2010), since we think we've fixed all the Skia
// bugs.  Leaving the code in for now, so we can revert easily if necessary.
// #define ENSURE_VALUE_SAFETY_FOR_SKIA

#ifdef ENSURE_VALUE_SAFETY_FOR_SKIA
static bool isCoordinateSkiaSafe(float coord)
{
    // First check for valid floats.
#if defined(_MSC_VER)
    if (!_finite(coord))
#else
    if (!finite(coord))
#endif
        return false;

    // Skia uses 16.16 fixed point and 26.6 fixed point in various places. If
    // the transformed point exceeds 15 bits, we just declare that it's
    // unreasonable to catch both of these cases.
    static const int maxPointMagnitude = 32767;
    if (coord > maxPointMagnitude || coord < -maxPointMagnitude)
        return false;

    return true;
}
#endif

static bool isPointSkiaSafe(const SkMatrix& transform, const SkPoint& pt)
{
#ifdef ENSURE_VALUE_SAFETY_FOR_SKIA
    // Now check for points that will overflow. We check the *transformed*
    // points since this is what will be rasterized.
    SkPoint xPt;
    transform.mapPoints(&xPt, &pt, 1);
    return isCoordinateSkiaSafe(xPt.fX) && isCoordinateSkiaSafe(xPt.fY);
#else
    return true;
#endif
}

static bool isRectSkiaSafe(const SkMatrix& transform, const SkRect& rc)
{
#ifdef ENSURE_VALUE_SAFETY_FOR_SKIA
    SkPoint topleft = {rc.fLeft, rc.fTop};
    SkPoint bottomright = {rc.fRight, rc.fBottom};
    return isPointSkiaSafe(transform, topleft) && isPointSkiaSafe(transform, bottomright);
#else
    return true;
#endif
}

bool isPathSkiaSafe(const SkMatrix& transform, const SkPath& path)
{
#ifdef ENSURE_VALUE_SAFETY_FOR_SKIA
    SkPoint current_points[4];
    SkPath::Iter iter(path, false);
    for (SkPath::Verb verb = iter.next(current_points);
         verb != SkPath::kDone_Verb;
         verb = iter.next(current_points)) {
        switch (verb) {
        case SkPath::kMove_Verb:
            // This move will be duplicated in the next verb, so we can ignore.
            break;
        case SkPath::kLine_Verb:
            // iter.next returns 2 points.
            if (!isPointSkiaSafe(transform, current_points[0])
                || !isPointSkiaSafe(transform, current_points[1]))
                return false;
            break;
        case SkPath::kQuad_Verb:
            // iter.next returns 3 points.
            if (!isPointSkiaSafe(transform, current_points[0])
                || !isPointSkiaSafe(transform, current_points[1])
                || !isPointSkiaSafe(transform, current_points[2]))
                return false;
            break;
        case SkPath::kCubic_Verb:
            // iter.next returns 4 points.
            if (!isPointSkiaSafe(transform, current_points[0])
                || !isPointSkiaSafe(transform, current_points[1])
                || !isPointSkiaSafe(transform, current_points[2])
                || !isPointSkiaSafe(transform, current_points[3]))
                return false;
            break;
        case SkPath::kClose_Verb:
        case SkPath::kDone_Verb:
        default:
            break;
        }
    }
    return true;
#else
    return true;
#endif
}

// Local helper functions ------------------------------------------------------

void addCornerArc(SkPath* path, const SkRect& rect, const IntSize& size, int startAngle)
{
    SkIRect ir;
    int rx = SkMin32(SkScalarRound(rect.width()), size.width());
    int ry = SkMin32(SkScalarRound(rect.height()), size.height());

    ir.set(-rx, -ry, rx, ry);
    switch (startAngle) {
    case 0:
        ir.offset(rect.fRight - ir.fRight, rect.fBottom - ir.fBottom);
        break;
    case 90:
        ir.offset(rect.fLeft - ir.fLeft, rect.fBottom - ir.fBottom);
        break;
    case 180:
        ir.offset(rect.fLeft - ir.fLeft, rect.fTop - ir.fTop);
        break;
    case 270:
        ir.offset(rect.fRight - ir.fRight, rect.fTop - ir.fTop);
        break;
    default:
        ASSERT(0);
    }

    SkRect r;
    r.set(ir);
    path->arcTo(r, SkIntToScalar(startAngle), SkIntToScalar(90), false);
}

// -----------------------------------------------------------------------------

// This may be called with a NULL pointer to create a graphics context that has
// no painting.
void GraphicsContext::platformInit(PlatformGraphicsContext* gc)
{
    m_data = new GraphicsContextPlatformPrivate(gc);
    setPaintingDisabled(!gc || !platformContext()->canvas());
}

void GraphicsContext::platformDestroy()
{
    delete m_data;
}

PlatformGraphicsContext* GraphicsContext::platformContext() const
{
    ASSERT(!paintingDisabled());
    return m_data->context();
}

// State saving ----------------------------------------------------------------

void GraphicsContext::savePlatformState()
{
    if (paintingDisabled())
        return;

    if (platformContext()->useGPU())
        platformContext()->gpuCanvas()->save();

    // Save our private State.
    platformContext()->save();
}

void GraphicsContext::restorePlatformState()
{
    if (paintingDisabled())
        return;

    if (platformContext()->useGPU())
        platformContext()->gpuCanvas()->restore();

    // Restore our private State.
    platformContext()->restore();
}

void GraphicsContext::beginTransparencyLayer(float opacity)
{
    if (paintingDisabled())
        return;

    // We need the "alpha" layer flag here because the base layer is opaque
    // (the surface of the page) but layers on top may have transparent parts.
    // Without explicitly setting the alpha flag, the layer will inherit the
    // opaque setting of the base and some things won't work properly.
    platformContext()->canvas()->saveLayerAlpha(
        0,
        static_cast<unsigned char>(opacity * 255),
        static_cast<SkCanvas::SaveFlags>(SkCanvas::kHasAlphaLayer_SaveFlag |
                                         SkCanvas::kFullColorLayer_SaveFlag));
}

void GraphicsContext::endTransparencyLayer()
{
    if (paintingDisabled())
        return;
    platformContext()->canvas()->restore();
}

// Graphics primitives ---------------------------------------------------------

void GraphicsContext::addInnerRoundedRectClip(const IntRect& rect, int thickness)
{
    if (paintingDisabled())
        return;

    SkRect r(rect);
    if (!isRectSkiaSafe(getCTM(), r))
        return;

    platformContext()->prepareForSoftwareDraw();
    SkPath path;
    path.addOval(r, SkPath::kCW_Direction);
    // only perform the inset if we won't invert r
    if (2 * thickness < rect.width() && 2 * thickness < rect.height()) {
        // Adding one to the thickness doesn't make the border too thick as
        // it's painted over afterwards. But without this adjustment the
        // border appears a little anemic after anti-aliasing.
        r.inset(SkIntToScalar(thickness + 1), SkIntToScalar(thickness + 1));
        path.addOval(r, SkPath::kCCW_Direction);
    }
    platformContext()->clipPathAntiAliased(path);
}

void GraphicsContext::addPath(const Path& path)
{
    if (paintingDisabled())
        return;
    platformContext()->addPath(*path.platformPath());
}

void GraphicsContext::beginPath()
{
    if (paintingDisabled())
        return;
    platformContext()->beginPath();
}

void GraphicsContext::clearPlatformShadow()
{
    if (paintingDisabled())
        return;
    platformContext()->setDrawLooper(0);
}

void GraphicsContext::clearRect(const FloatRect& rect)
{
    if (paintingDisabled())
        return;

    if (platformContext()->useGPU() && !platformContext()->canvasClipApplied()) {
        platformContext()->prepareForHardwareDraw();
        platformContext()->gpuCanvas()->clearRect(rect);
        return;
    }

    // Force a readback here (if we're using the GPU), since clearRect() is
    // incompatible with mixed-mode rendering.
    platformContext()->syncSoftwareCanvas();

    SkRect r = rect;
    if (!isRectSkiaSafe(getCTM(), r))
        ClipRectToCanvas(*platformContext()->canvas(), r, &r);

    SkPaint paint;
    platformContext()->setupPaintForFilling(&paint);
    paint.setXfermodeMode(SkXfermode::kClear_Mode);
    platformContext()->canvas()->drawRect(r, paint);
}

void GraphicsContext::clip(const FloatRect& rect)
{
    if (paintingDisabled())
        return;

    SkRect r(rect);
    if (!isRectSkiaSafe(getCTM(), r))
        return;

    platformContext()->prepareForSoftwareDraw();
    platformContext()->canvas()->clipRect(r);
}

void GraphicsContext::clip(const Path& path)
{
    if (paintingDisabled())
        return;

    const SkPath& p = *path.platformPath();
    if (!isPathSkiaSafe(getCTM(), p))
        return;

    platformContext()->prepareForSoftwareDraw();
    platformContext()->clipPathAntiAliased(p);
}

void GraphicsContext::canvasClip(const Path& path)
{
    if (paintingDisabled())
        return;

    const SkPath& p = *path.platformPath();
    if (!isPathSkiaSafe(getCTM(), p))
        return;

    platformContext()->canvasClipPath(p);
}

void GraphicsContext::clipOut(const IntRect& rect)
{
    if (paintingDisabled())
        return;

    SkRect r(rect);
    if (!isRectSkiaSafe(getCTM(), r))
        return;

    platformContext()->canvas()->clipRect(r, SkRegion::kDifference_Op);
}

void GraphicsContext::clipOut(const Path& p)
{
    if (paintingDisabled())
        return;

    const SkPath& path = *p.platformPath();
    if (!isPathSkiaSafe(getCTM(), path))
        return;

    platformContext()->canvas()->clipPath(path, SkRegion::kDifference_Op);
}

void GraphicsContext::clipPath(const Path& pathToClip, WindRule clipRule)
{
    if (paintingDisabled())
        return;

    // FIXME: Be smarter about this.
    beginPath();
    addPath(pathToClip);

    SkPath path = platformContext()->currentPathInLocalCoordinates();
    if (!isPathSkiaSafe(getCTM(), path))
        return;

    path.setFillType(clipRule == RULE_EVENODD ? SkPath::kEvenOdd_FillType : SkPath::kWinding_FillType);
    platformContext()->clipPathAntiAliased(path);
}

void GraphicsContext::concatCTM(const AffineTransform& affine)
{
    if (paintingDisabled())
        return;

    if (platformContext()->useGPU())
        platformContext()->gpuCanvas()->concatCTM(affine);

    platformContext()->canvas()->concat(affine);
}

void GraphicsContext::drawConvexPolygon(size_t numPoints,
                                        const FloatPoint* points,
                                        bool shouldAntialias)
{
    if (paintingDisabled())
        return;

    if (numPoints <= 1)
        return;

    platformContext()->prepareForSoftwareDraw();

    SkPath path;

    path.incReserve(numPoints);
    path.moveTo(WebCoreFloatToSkScalar(points[0].x()),
                WebCoreFloatToSkScalar(points[0].y()));
    for (size_t i = 1; i < numPoints; i++) {
        path.lineTo(WebCoreFloatToSkScalar(points[i].x()),
                    WebCoreFloatToSkScalar(points[i].y()));
    }

    if (!isPathSkiaSafe(getCTM(), path))
        return;

    SkPaint paint;
    platformContext()->setupPaintForFilling(&paint);
    platformContext()->canvas()->drawPath(path, paint);

    if (strokeStyle() != NoStroke) {
        paint.reset();
        platformContext()->setupPaintForStroking(&paint, 0, 0);
        platformContext()->canvas()->drawPath(path, paint);
    }
}

void GraphicsContext::clipConvexPolygon(size_t numPoints, const FloatPoint* points, bool antialiased)
{
    if (paintingDisabled())
        return;

    if (numPoints <= 1)
        return;

    // FIXME: IMPLEMENT!!
}

// This method is only used to draw the little circles used in lists.
void GraphicsContext::drawEllipse(const IntRect& elipseRect)
{
    if (paintingDisabled())
        return;

    SkRect rect = elipseRect;
    if (!isRectSkiaSafe(getCTM(), rect))
        return;

    platformContext()->prepareForSoftwareDraw();
    SkPaint paint;
    platformContext()->setupPaintForFilling(&paint);
    platformContext()->canvas()->drawOval(rect, paint);

    if (strokeStyle() != NoStroke) {
        paint.reset();
        platformContext()->setupPaintForStroking(&paint, &rect, 0);
        platformContext()->canvas()->drawOval(rect, paint);
    }
}

void GraphicsContext::drawFocusRing(const Path& path, int width, int offset, const Color& color)
{
    // FIXME: implement
}

void GraphicsContext::drawFocusRing(const Vector<IntRect>& rects, int /* width */, int /* offset */, const Color& color)
{
    if (paintingDisabled())
        return;

    unsigned rectCount = rects.size();
    if (!rectCount)
        return;

    platformContext()->prepareForSoftwareDraw();
    SkRegion focusRingRegion;
    const SkScalar focusRingOutset = WebCoreFloatToSkScalar(0.5);
    for (unsigned i = 0; i < rectCount; i++) {
        SkIRect r = rects[i];
        r.inset(-focusRingOutset, -focusRingOutset);
        focusRingRegion.op(r, SkRegion::kUnion_Op);
    }

    SkPath path;
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);

    paint.setColor(color.rgb());
    paint.setStrokeWidth(focusRingOutset * 2);
    paint.setPathEffect(new SkCornerPathEffect(focusRingOutset * 2))->unref();
    focusRingRegion.getBoundaryPath(&path);
    platformContext()->canvas()->drawPath(path, paint);
}

// This is only used to draw borders.
void GraphicsContext::drawLine(const IntPoint& point1, const IntPoint& point2)
{
    if (paintingDisabled())
        return;

    StrokeStyle penStyle = strokeStyle();
    if (penStyle == NoStroke)
        return;

    SkPaint paint;
    if (!isPointSkiaSafe(getCTM(), point1) || !isPointSkiaSafe(getCTM(), point2))
        return;

    platformContext()->prepareForSoftwareDraw();

    FloatPoint p1 = point1;
    FloatPoint p2 = point2;
    bool isVerticalLine = (p1.x() == p2.x());
    int width = roundf(strokeThickness());

    // We know these are vertical or horizontal lines, so the length will just
    // be the sum of the displacement component vectors give or take 1 -
    // probably worth the speed up of no square root, which also won't be exact.
    FloatSize disp = p2 - p1;
    int length = SkScalarRound(disp.width() + disp.height());
    platformContext()->setupPaintForStroking(&paint, 0, length);

    if (strokeStyle() == DottedStroke || strokeStyle() == DashedStroke) {
        // Do a rect fill of our endpoints.  This ensures we always have the
        // appearance of being a border.  We then draw the actual dotted/dashed line.

        SkRect r1, r2;
        r1.set(p1.x(), p1.y(), p1.x() + width, p1.y() + width);
        r2.set(p2.x(), p2.y(), p2.x() + width, p2.y() + width);

        if (isVerticalLine) {
            r1.offset(-width / 2, 0);
            r2.offset(-width / 2, -width);
        } else {
            r1.offset(0, -width / 2);
            r2.offset(-width, -width / 2);
        }
        SkPaint fillPaint;
        fillPaint.setColor(paint.getColor());
        platformContext()->canvas()->drawRect(r1, fillPaint);
        platformContext()->canvas()->drawRect(r2, fillPaint);
    }

    adjustLineToPixelBoundaries(p1, p2, width, penStyle);
    SkPoint pts[2] = { (SkPoint)p1, (SkPoint)p2 };

    platformContext()->canvas()->drawPoints(SkCanvas::kLines_PointMode, 2, pts, paint);
}

void GraphicsContext::drawLineForTextChecking(const IntPoint& pt, int width, TextCheckingLineStyle style)
{
    if (paintingDisabled())
        return;

    platformContext()->prepareForSoftwareDraw();

    // Create the pattern we'll use to draw the underline.
    static SkBitmap* misspellBitmap = 0;
    if (!misspellBitmap) {
        // We use a 2-pixel-high misspelling indicator because that seems to be
        // what WebKit is designed for, and how much room there is in a typical
        // page for it.
        const int rowPixels = 32;  // Must be multiple of 4 for pattern below.
        const int colPixels = 2;
        misspellBitmap = new SkBitmap;
        misspellBitmap->setConfig(SkBitmap::kARGB_8888_Config,
                                   rowPixels, colPixels);
        misspellBitmap->allocPixels();

        misspellBitmap->eraseARGB(0, 0, 0, 0);
        const uint32_t lineColor = 0xFFFF0000;  // Opaque red.
        const uint32_t antiColor = 0x60600000;  // Semitransparent red.

        // Pattern:  X o   o X o   o X
        //             o X o   o X o
        uint32_t* row1 = misspellBitmap->getAddr32(0, 0);
        uint32_t* row2 = misspellBitmap->getAddr32(0, 1);
        for (int x = 0; x < rowPixels; x++) {
            switch (x % 4) {
            case 0:
                row1[x] = lineColor;
                break;
            case 1:
                row1[x] = antiColor;
                row2[x] = antiColor;
                break;
            case 2:
                row2[x] = lineColor;
                break;
            case 3:
                row1[x] = antiColor;
                row2[x] = antiColor;
                break;
            }
        }
    }

    // Offset it vertically by 1 so that there's some space under the text.
    SkScalar originX = SkIntToScalar(pt.x());
    SkScalar originY = SkIntToScalar(pt.y()) + 1;

    // Make a shader for the bitmap with an origin of the box we'll draw. This
    // shader is refcounted and will have an initial refcount of 1.
    SkShader* shader = SkShader::CreateBitmapShader(
        *misspellBitmap, SkShader::kRepeat_TileMode,
        SkShader::kRepeat_TileMode);
    SkMatrix matrix;
    matrix.reset();
    matrix.postTranslate(originX, originY);
    shader->setLocalMatrix(matrix);

    // Assign the shader to the paint & release our reference. The paint will
    // now own the shader and the shader will be destroyed when the paint goes
    // out of scope.
    SkPaint paint;
    paint.setShader(shader);
    shader->unref();

    SkRect rect;
    rect.set(originX,
             originY,
             originX + SkIntToScalar(width),
             originY + SkIntToScalar(misspellBitmap->height()));
    platformContext()->canvas()->drawRect(rect, paint);
}

void GraphicsContext::drawLineForText(const IntPoint& pt,
                                      int width,
                                      bool printing)
{
    if (paintingDisabled())
        return;

    if (width <= 0)
        return;

    platformContext()->prepareForSoftwareDraw();

    int thickness = SkMax32(static_cast<int>(strokeThickness()), 1);
    SkRect r;
    r.fLeft = SkIntToScalar(pt.x());
    r.fTop = SkIntToScalar(pt.y());
    r.fRight = r.fLeft + SkIntToScalar(width);
    r.fBottom = r.fTop + SkIntToScalar(thickness);

    SkPaint paint;
    platformContext()->setupPaintForFilling(&paint);
    // Text lines are drawn using the stroke color.
    paint.setColor(platformContext()->effectiveStrokeColor());
    platformContext()->canvas()->drawRect(r, paint);
}

// Draws a filled rectangle with a stroked border.
void GraphicsContext::drawRect(const IntRect& rect)
{
    if (paintingDisabled())
        return;

    platformContext()->prepareForSoftwareDraw();

    SkRect r = rect;
    if (!isRectSkiaSafe(getCTM(), r)) {
        // See the fillRect below.
        ClipRectToCanvas(*platformContext()->canvas(), r, &r);
    }

    platformContext()->drawRect(r);
}

void GraphicsContext::fillPath(const Path& pathToFill)
{
    if (paintingDisabled())
        return;

    // FIXME: Be smarter about this.
    beginPath();
    addPath(pathToFill);

    SkPath path = platformContext()->currentPathInLocalCoordinates();
    if (!isPathSkiaSafe(getCTM(), path))
      return;

    platformContext()->prepareForSoftwareDraw();

    const GraphicsContextState& state = m_state;
    path.setFillType(state.fillRule == RULE_EVENODD ?
        SkPath::kEvenOdd_FillType : SkPath::kWinding_FillType);

    SkPaint paint;
    platformContext()->setupPaintForFilling(&paint);

    platformContext()->canvas()->drawPath(path, paint);
}

void GraphicsContext::fillRect(const FloatRect& rect)
{
    if (paintingDisabled())
        return;

    SkRect r = rect;
    if (!isRectSkiaSafe(getCTM(), r)) {
        // See the other version of fillRect below.
        ClipRectToCanvas(*platformContext()->canvas(), r, &r);
    }

    if (platformContext()->useGPU() && platformContext()->canAccelerate()) {
        platformContext()->prepareForHardwareDraw();
        platformContext()->gpuCanvas()->fillRect(rect);
        return;
    }

    platformContext()->save();

    platformContext()->prepareForSoftwareDraw();

    SkPaint paint;
    platformContext()->setupPaintForFilling(&paint);
    platformContext()->canvas()->drawRect(r, paint);

    platformContext()->restore();
}

void GraphicsContext::fillRect(const FloatRect& rect, const Color& color, ColorSpace colorSpace)
{
    if (paintingDisabled())
        return;

    if (platformContext()->useGPU() && platformContext()->canAccelerate()) {
        platformContext()->prepareForHardwareDraw();
        platformContext()->gpuCanvas()->fillRect(rect, color, colorSpace);
        return;
    }

    platformContext()->prepareForSoftwareDraw();

    SkRect r = rect;
    if (!isRectSkiaSafe(getCTM(), r)) {
        // Special case when the rectangle overflows fixed point. This is a
        // workaround to fix bug 1212844. When the input rectangle is very
        // large, it can overflow Skia's internal fixed point rect. This
        // should be fixable in Skia (since the output bitmap isn't that
        // large), but until that is fixed, we try to handle it ourselves.
        //
        // We manually clip the rectangle to the current clip rect. This
        // will prevent overflow. The rectangle will be transformed to the
        // canvas' coordinate space before it is converted to fixed point
        // so we are guaranteed not to overflow after doing this.
        ClipRectToCanvas(*platformContext()->canvas(), r, &r);
    }

    SkPaint paint;
    platformContext()->setupPaintCommon(&paint);
    paint.setColor(color.rgb());
    platformContext()->canvas()->drawRect(r, paint);
}

void GraphicsContext::fillRoundedRect(const IntRect& rect,
                                      const IntSize& topLeft,
                                      const IntSize& topRight,
                                      const IntSize& bottomLeft,
                                      const IntSize& bottomRight,
                                      const Color& color,
                                      ColorSpace colorSpace)
{
    if (paintingDisabled())
        return;

    platformContext()->prepareForSoftwareDraw();

    SkRect r = rect;
    if (!isRectSkiaSafe(getCTM(), r))
        // See fillRect().
        ClipRectToCanvas(*platformContext()->canvas(), r, &r);

    if (topLeft.width() + topRight.width() > rect.width()
            || bottomLeft.width() + bottomRight.width() > rect.width()
            || topLeft.height() + bottomLeft.height() > rect.height()
            || topRight.height() + bottomRight.height() > rect.height()) {
        // Not all the radii fit, return a rect. This matches the behavior of
        // Path::createRoundedRectangle. Without this we attempt to draw a round
        // shadow for a square box.
        fillRect(rect, color, colorSpace);
        return;
    }

    SkPath path;
    addCornerArc(&path, r, topRight, 270);
    addCornerArc(&path, r, bottomRight, 0);
    addCornerArc(&path, r, bottomLeft, 90);
    addCornerArc(&path, r, topLeft, 180);

    SkPaint paint;
    platformContext()->setupPaintForFilling(&paint);
    platformContext()->canvas()->drawPath(path, paint);
}

AffineTransform GraphicsContext::getCTM() const
{
    const SkMatrix& m = platformContext()->canvas()->getTotalMatrix();
    return AffineTransform(SkScalarToDouble(m.getScaleX()),
                           SkScalarToDouble(m.getSkewY()),
                           SkScalarToDouble(m.getSkewX()),
                           SkScalarToDouble(m.getScaleY()),
                           SkScalarToDouble(m.getTranslateX()),
                           SkScalarToDouble(m.getTranslateY()));
}

FloatRect GraphicsContext::roundToDevicePixels(const FloatRect& rect)
{
    // This logic is copied from GraphicsContextCG, eseidel 5/05/08

    // It is not enough just to round to pixels in device space. The rotation
    // part of the affine transform matrix to device space can mess with this
    // conversion if we have a rotating image like the hands of the world clock
    // widget. We just need the scale, so we get the affine transform matrix and
    // extract the scale.

    const SkMatrix& deviceMatrix = platformContext()->canvas()->getTotalMatrix();
    if (deviceMatrix.isIdentity())
        return rect;

    float deviceScaleX = sqrtf(square(deviceMatrix.getScaleX())
        + square(deviceMatrix.getSkewY()));
    float deviceScaleY = sqrtf(square(deviceMatrix.getSkewX())
        + square(deviceMatrix.getScaleY()));

    FloatPoint deviceOrigin(rect.x() * deviceScaleX, rect.y() * deviceScaleY);
    FloatPoint deviceLowerRight((rect.x() + rect.width()) * deviceScaleX,
        (rect.y() + rect.height()) * deviceScaleY);

    deviceOrigin.setX(roundf(deviceOrigin.x()));
    deviceOrigin.setY(roundf(deviceOrigin.y()));
    deviceLowerRight.setX(roundf(deviceLowerRight.x()));
    deviceLowerRight.setY(roundf(deviceLowerRight.y()));

    // Don't let the height or width round to 0 unless either was originally 0
    if (deviceOrigin.y() == deviceLowerRight.y() && rect.height())
        deviceLowerRight.move(0, 1);
    if (deviceOrigin.x() == deviceLowerRight.x() && rect.width())
        deviceLowerRight.move(1, 0);

    FloatPoint roundedOrigin(deviceOrigin.x() / deviceScaleX,
        deviceOrigin.y() / deviceScaleY);
    FloatPoint roundedLowerRight(deviceLowerRight.x() / deviceScaleX,
        deviceLowerRight.y() / deviceScaleY);
    return FloatRect(roundedOrigin, roundedLowerRight - roundedOrigin);
}

void GraphicsContext::scale(const FloatSize& size)
{
    if (paintingDisabled())
        return;

    if (platformContext()->useGPU())
        platformContext()->gpuCanvas()->scale(size);

    platformContext()->canvas()->scale(WebCoreFloatToSkScalar(size.width()),
        WebCoreFloatToSkScalar(size.height()));
}

void GraphicsContext::setAlpha(float alpha)
{
    if (paintingDisabled())
        return;

    if (platformContext()->useGPU())
        platformContext()->gpuCanvas()->setAlpha(alpha);

    platformContext()->setAlpha(alpha);
}

void GraphicsContext::setPlatformCompositeOperation(CompositeOperator op)
{
    if (paintingDisabled())
        return;

    if (platformContext()->useGPU())
        platformContext()->gpuCanvas()->setCompositeOperation(op);

    platformContext()->setXfermodeMode(WebCoreCompositeToSkiaComposite(op));
}

InterpolationQuality GraphicsContext::imageInterpolationQuality() const
{
    return platformContext()->interpolationQuality();
}

void GraphicsContext::setImageInterpolationQuality(InterpolationQuality q)
{
    platformContext()->setInterpolationQuality(q);
}

void GraphicsContext::setLineCap(LineCap cap)
{
    if (paintingDisabled())
        return;
    switch (cap) {
    case ButtCap:
        platformContext()->setLineCap(SkPaint::kButt_Cap);
        break;
    case RoundCap:
        platformContext()->setLineCap(SkPaint::kRound_Cap);
        break;
    case SquareCap:
        platformContext()->setLineCap(SkPaint::kSquare_Cap);
        break;
    default:
        ASSERT(0);
        break;
    }
}

void GraphicsContext::setLineDash(const DashArray& dashes, float dashOffset)
{
    if (paintingDisabled())
        return;

    // FIXME: This is lifted directly off SkiaSupport, lines 49-74
    // so it is not guaranteed to work correctly.
    size_t dashLength = dashes.size();
    if (!dashLength) {
        // If no dash is set, revert to solid stroke
        // FIXME: do we need to set NoStroke in some cases?
        platformContext()->setStrokeStyle(SolidStroke);
        platformContext()->setDashPathEffect(0);
        return;
    }

    size_t count = !(dashLength % 2) ? dashLength : dashLength * 2;
    SkScalar* intervals = new SkScalar[count];

    for (unsigned int i = 0; i < count; i++)
        intervals[i] = dashes[i % dashLength];

    platformContext()->setDashPathEffect(new SkDashPathEffect(intervals, count, dashOffset));

    delete[] intervals;
}

void GraphicsContext::setLineJoin(LineJoin join)
{
    if (paintingDisabled())
        return;
    switch (join) {
    case MiterJoin:
        platformContext()->setLineJoin(SkPaint::kMiter_Join);
        break;
    case RoundJoin:
        platformContext()->setLineJoin(SkPaint::kRound_Join);
        break;
    case BevelJoin:
        platformContext()->setLineJoin(SkPaint::kBevel_Join);
        break;
    default:
        ASSERT(0);
        break;
    }
}

void GraphicsContext::setMiterLimit(float limit)
{
    if (paintingDisabled())
        return;
    platformContext()->setMiterLimit(limit);
}

void GraphicsContext::setPlatformFillColor(const Color& color, ColorSpace colorSpace)
{
    if (paintingDisabled())
        return;

    if (platformContext()->useGPU())
        platformContext()->gpuCanvas()->setFillColor(color, colorSpace);

    platformContext()->setFillColor(color.rgb());
}

void GraphicsContext::setPlatformFillGradient(Gradient* gradient)
{
    if (paintingDisabled())
        return;

    platformContext()->setFillShader(gradient->platformGradient());
}

void GraphicsContext::setPlatformFillPattern(Pattern* pattern)
{
    if (paintingDisabled())
        return;

    platformContext()->setFillShader(pattern->platformPattern(getCTM()));
}

void GraphicsContext::setPlatformShadow(const FloatSize& size,
                                        float blurFloat,
                                        const Color& color,
                                        ColorSpace colorSpace)
{
    if (paintingDisabled())
        return;

    // Detect when there's no effective shadow and clear the looper.
    if (!size.width() && !size.height() && !blurFloat) {
        platformContext()->setDrawLooper(0);
        return;
    }

    double width = size.width();
    double height = size.height();
    double blur = blurFloat;

    SkBlurDrawLooper::BlurFlags blurFlags = SkBlurDrawLooper::kNone_BlurFlag;

    if (m_state.shadowsIgnoreTransforms)  {
        // Currently only the GraphicsContext associated with the
        // CanvasRenderingContext for HTMLCanvasElement have shadows ignore
        // Transforms. So with this flag set, we know this state is associated
        // with a CanvasRenderingContext.
        blurFlags = SkBlurDrawLooper::kIgnoreTransform_BlurFlag;
        
        // CG uses natural orientation for Y axis, but the HTML5 canvas spec
        // does not.
        // So we now flip the height since it was flipped in
        // CanvasRenderingContext in order to work with CG.
        height = -height;
    }

    SkColor c;
    if (color.isValid())
        c = color.rgb();
    else
        c = SkColorSetARGB(0xFF/3, 0, 0, 0);    // "std" apple shadow color.

    // TODO(tc): Should we have a max value for the blur?  CG clamps at 1000.0
    // for perf reasons.
    SkDrawLooper* dl = new SkBlurDrawLooper(blur / 2, width, height, c, blurFlags);
    platformContext()->setDrawLooper(dl);
    dl->unref();
}

void GraphicsContext::setPlatformStrokeColor(const Color& strokecolor, ColorSpace colorSpace)
{
    if (paintingDisabled())
        return;

    platformContext()->setStrokeColor(strokecolor.rgb());
}

void GraphicsContext::setPlatformStrokeStyle(StrokeStyle stroke)
{
    if (paintingDisabled())
        return;

    platformContext()->setStrokeStyle(stroke);
}

void GraphicsContext::setPlatformStrokeThickness(float thickness)
{
    if (paintingDisabled())
        return;

    platformContext()->setStrokeThickness(thickness);
}

void GraphicsContext::setPlatformStrokeGradient(Gradient* gradient)
{
    if (paintingDisabled())
        return;

    platformContext()->setStrokeShader(gradient->platformGradient());
}

void GraphicsContext::setPlatformStrokePattern(Pattern* pattern)
{
    if (paintingDisabled())
        return;

    platformContext()->setStrokeShader(pattern->platformPattern(getCTM()));
}

void GraphicsContext::setPlatformTextDrawingMode(TextDrawingModeFlags mode)
{
    if (paintingDisabled())
        return;

    platformContext()->setTextDrawingMode(mode);
}

void GraphicsContext::setURLForRect(const KURL& link, const IntRect& destRect)
{
}

void GraphicsContext::setPlatformShouldAntialias(bool enable)
{
    if (paintingDisabled())
        return;

    platformContext()->setUseAntialiasing(enable);
}

void GraphicsContext::strokeArc(const IntRect& r, int startAngle, int angleSpan)
{
    if (paintingDisabled())
        return;

    platformContext()->prepareForSoftwareDraw();

    SkPaint paint;
    SkRect oval = r;
    if (strokeStyle() == NoStroke) {
        // Stroke using the fill color.
        // TODO(brettw) is this really correct? It seems unreasonable.
        platformContext()->setupPaintForFilling(&paint);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(WebCoreFloatToSkScalar(strokeThickness()));
    } else
        platformContext()->setupPaintForStroking(&paint, 0, 0);

    // We do this before converting to scalar, so we don't overflow SkFixed.
    startAngle = fastMod(startAngle, 360);
    angleSpan = fastMod(angleSpan, 360);

    SkPath path;
    path.addArc(oval, SkIntToScalar(-startAngle), SkIntToScalar(-angleSpan));
    if (!isPathSkiaSafe(getCTM(), path))
        return;
    platformContext()->canvas()->drawPath(path, paint);
}

void GraphicsContext::strokePath(const Path& pathToStroke)
{
    if (paintingDisabled())
        return;

    // FIXME: Be smarter about this.
    beginPath();
    addPath(pathToStroke);

    SkPath path = platformContext()->currentPathInLocalCoordinates();
    if (!isPathSkiaSafe(getCTM(), path))
        return;

    platformContext()->prepareForSoftwareDraw();

    SkPaint paint;
    platformContext()->setupPaintForStroking(&paint, 0, 0);
    platformContext()->canvas()->drawPath(path, paint);
}

void GraphicsContext::strokeRect(const FloatRect& rect, float lineWidth)
{
    if (paintingDisabled())
        return;

    if (!isRectSkiaSafe(getCTM(), rect))
        return;

    platformContext()->prepareForSoftwareDraw();

    SkPaint paint;
    platformContext()->setupPaintForStroking(&paint, 0, 0);
    paint.setStrokeWidth(WebCoreFloatToSkScalar(lineWidth));
    platformContext()->canvas()->drawRect(rect, paint);
}

void GraphicsContext::rotate(float angleInRadians)
{
    if (paintingDisabled())
        return;

    if (platformContext()->useGPU())
        platformContext()->gpuCanvas()->rotate(angleInRadians);

    platformContext()->canvas()->rotate(WebCoreFloatToSkScalar(
        angleInRadians * (180.0f / 3.14159265f)));
}

void GraphicsContext::translate(float w, float h)
{
    if (paintingDisabled())
        return;

    if (platformContext()->useGPU())
        platformContext()->gpuCanvas()->translate(w, h);

    platformContext()->canvas()->translate(WebCoreFloatToSkScalar(w),
                                           WebCoreFloatToSkScalar(h));
}

void GraphicsContext::syncSoftwareCanvas()
{
    platformContext()->syncSoftwareCanvas();
}

void GraphicsContext::setSharedGraphicsContext3D(SharedGraphicsContext3D* context, DrawingBuffer* framebuffer, const IntSize& size)
{
    platformContext()->setSharedGraphicsContext3D(context, framebuffer, size);
}

void GraphicsContext::markDirtyRect(const IntRect& rect)
{
    platformContext()->markDirtyRect(rect);
}

}  // namespace WebCore
