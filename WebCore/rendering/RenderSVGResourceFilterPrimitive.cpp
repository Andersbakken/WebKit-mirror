/*
 * Copyright (C) 2010 University of Szeged
 * Copyright (C) 2010 Zoltan Herczeg
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
 * THIS SOFTWARE IS PROVIDED BY UNIVERSITY OF SZEGED ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL UNIVERSITY OF SZEGED OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#if ENABLE(SVG) && ENABLE(FILTERS)
#include "RenderSVGResourceFilterPrimitive.h"
#include "SVGFEImage.h"

namespace WebCore {

FloatRect RenderSVGResourceFilterPrimitive::determineFilterPrimitiveSubregion(FilterEffect* effect, SVGFilter* filter)
{
    FloatRect uniteRect;
    FloatRect subregionBoundingBox = effect->effectBoundaries();
    FloatRect subregion = subregionBoundingBox;

    if (effect->filterEffectType() != FilterEffectTypeTile) {
        // FETurbulence, FEImage and FEFlood don't have input effects, take the filter region as unite rect.
        if (unsigned numberOfInputEffects = effect->inputEffects().size()) {
            for (unsigned i = 0; i < numberOfInputEffects; ++i)
                uniteRect.unite(determineFilterPrimitiveSubregion(effect->inputEffect(i), filter));
        } else
            uniteRect = filter->filterRegionInUserSpace();
    } else {
        determineFilterPrimitiveSubregion(effect->inputEffect(0), filter);
        uniteRect = filter->filterRegionInUserSpace();
    }

    if (filter->effectBoundingBoxMode()) {
        subregion = uniteRect;
        // Avoid the calling of a virtual method several times.
        FloatRect targetBoundingBox = filter->targetBoundingBox();

        if (effect->hasX())
            subregion.setX(targetBoundingBox.x() + subregionBoundingBox.x() * targetBoundingBox.width());

        if (effect->hasY())
            subregion.setY(targetBoundingBox.y() + subregionBoundingBox.y() * targetBoundingBox.height());

        if (effect->hasWidth())
            subregion.setWidth(subregionBoundingBox.width() * targetBoundingBox.width());

        if (effect->hasHeight())
            subregion.setHeight(subregionBoundingBox.height() * targetBoundingBox.height());
    } else {
        if (!effect->hasX())
            subregion.setX(uniteRect.x());

        if (!effect->hasY())
            subregion.setY(uniteRect.y());

        if (!effect->hasWidth())
            subregion.setWidth(uniteRect.width());

        if (!effect->hasHeight())
            subregion.setHeight(uniteRect.height());
    }

    effect->setFilterPrimitiveSubregion(subregion);

    FloatRect absoluteSubregion = filter->mapLocalRectToAbsoluteRect(subregion);
    FloatSize filterResolution = filter->filterResolution();
    absoluteSubregion.scale(filterResolution.width(), filterResolution.height());

    // FEImage needs the unclipped subregion in absolute coordinates to determine the correct
    // destination rect in combination with preserveAspectRatio.
    if (effect->filterEffectType() == FilterEffectTypeImage)
        reinterpret_cast<FEImage*>(effect)->setAbsoluteSubregion(absoluteSubregion);

    // Clip every filter effect to the filter region.
    FloatRect absoluteScaledFilterRegion = filter->filterRegion();
    absoluteScaledFilterRegion.scale(filterResolution.width(), filterResolution.height());
    absoluteSubregion.intersect(absoluteScaledFilterRegion);

    effect->setMaxEffectRect(enclosingIntRect(absoluteSubregion));
    return subregion;
}

} // namespace WebCore

#endif // ENABLE(SVG) && ENABLE(FILTERS)
