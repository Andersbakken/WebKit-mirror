/*
 * Copyright (C) 2004, 2005 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006 Rob Buis <buis@kde.org>
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"

#if ENABLE(SVG) && ENABLE(SVG_ANIMATION)
#include "SVGAnimateElement.h"

#include "ColorDistance.h"
#include "FloatConversion.h"
#include "SVGColor.h"
#include "SVGNames.h"
#include "SVGParserUtilities.h"
#include "SVGPathParserFactory.h"
#include "SVGPathSegList.h"
#include "SVGPointList.h"

using namespace std;

namespace WebCore {

SVGAnimateElement::SVGAnimateElement(const QualifiedName& tagName, Document* document)
    : SVGAnimationElement(tagName, document)
    , m_propertyType(StringProperty)
    , m_fromNumber(0)
    , m_toNumber(0)
    , m_animatedNumber(numeric_limits<double>::infinity())
    , m_animatedPathPointer(0)
{
}

PassRefPtr<SVGAnimateElement> SVGAnimateElement::create(const QualifiedName& tagName, Document* document)
{
    return adoptRef(new SVGAnimateElement(tagName, document));
}

SVGAnimateElement::~SVGAnimateElement()
{
}

static bool parseNumberValueAndUnit(const String& in, double& value, String& unit)
{
    // FIXME: These are from top of my head, figure out all property types that can be animated as numbers.
    unsigned unitLength = 0;
    String parse = in.stripWhiteSpace();
    if (parse.endsWith("%"))
        unitLength = 1;
    else if (parse.endsWith("px") || parse.endsWith("pt") || parse.endsWith("em"))
        unitLength = 2;
    else if (parse.endsWith("deg") || parse.endsWith("rad"))
        unitLength = 3;
    else if (parse.endsWith("grad"))
        unitLength = 4;
    String newUnit = parse.right(unitLength);
    String number = parse.left(parse.length() - unitLength);
    if ((!unit.isEmpty() && newUnit != unit) || number.isEmpty())
        return false;
    UChar last = number[number.length() - 1];
    if (last < '0' || last > '9')
        return false;
    unit = newUnit;
    bool ok;
    value = number.toDouble(&ok);
    return ok;
}

SVGAnimateElement::PropertyType SVGAnimateElement::determinePropertyType(const String& attribute) const
{
    // FIXME: We need a full property table for figuring this out reliably.
    if (hasTagName(SVGNames::animateColorTag))
        return ColorProperty;
    if (attribute == "d")
        return PathProperty;
    if (attribute == "points")
        return PointsProperty;
    if (attribute == "color" || attribute == "fill" || attribute == "stroke")
        return ColorProperty;
    return NumberProperty;
}

void SVGAnimateElement::calculateAnimatedValue(float percentage, unsigned repeat, SVGSMILElement* resultElement)
{
    ASSERT(percentage >= 0.f && percentage <= 1.f);
    ASSERT(resultElement);
    if (hasTagName(SVGNames::setTag))
        percentage = 1.f;
    if (!resultElement->hasTagName(SVGNames::animateTag) && !resultElement->hasTagName(SVGNames::animateColorTag) 
        && !resultElement->hasTagName(SVGNames::setTag))
        return;
    SVGAnimateElement* results = static_cast<SVGAnimateElement*>(resultElement);
    // Can't accumulate over a string property.
    if (results->m_propertyType == StringProperty && m_propertyType != StringProperty)
        return;
    if (m_propertyType == NumberProperty) {
        // To animation uses contributions from the lower priority animations as the base value.
        if (animationMode() == ToAnimation)
            m_fromNumber = results->m_animatedNumber;
    
        double number = (m_toNumber - m_fromNumber) * percentage + m_fromNumber;

        // FIXME: This is not correct for values animation.
        if (isAccumulated() && repeat)
            number += m_toNumber * repeat;
        if (isAdditive() && animationMode() != ToAnimation)
            results->m_animatedNumber += number;
        else 
            results->m_animatedNumber = number;
        return;
    } 
    if (m_propertyType == ColorProperty) {
        if (animationMode() == ToAnimation)
            m_fromColor = results->m_animatedColor;
        Color color = ColorDistance(m_fromColor, m_toColor).scaledDistance(percentage).addToColorAndClamp(m_fromColor);
        // FIXME: Accumulate colors.
        if (isAdditive() && animationMode() != ToAnimation)
            results->m_animatedColor = ColorDistance::addColorsAndClamp(results->m_animatedColor, color);
        else
            results->m_animatedColor = color;
        return;
    }
    AnimationMode animationMode = this->animationMode();
    if (m_propertyType == PathProperty) {
        if (!percentage) {
            ASSERT(m_fromPath);
            ASSERT(percentage >= 0);
            results->m_animatedPathPointer = m_fromPath.get();
        } else if (percentage == 1.f) {
            ASSERT(m_toPath);
            results->m_animatedPathPointer = m_toPath.get();
        } else {
            if (m_fromPath && m_toPath) {
                SVGPathParserFactory* factory = SVGPathParserFactory::self();
                if (!factory->buildAnimatedSVGPathByteStream(m_fromPath.get(), m_toPath.get(), results->m_animatedPath, percentage)) {
                    results->m_animatedPath.clear();
                    results->m_animatedPathPointer = 0;
                } else
                    results->m_animatedPathPointer = results->m_animatedPath.get();
            } else
                results->m_animatedPathPointer = 0;
            // Fall back to discrete animation if the paths are not compatible
            if (!results->m_animatedPathPointer) {
                ASSERT(m_fromPath);
                ASSERT(m_toPath);
                ASSERT(!results->m_animatedPath);
                results->m_animatedPathPointer = ((animationMode == FromToAnimation && percentage > 0.5f) || animationMode == ToAnimation || percentage == 1.0f) 
                    ? m_toPath.get() : m_fromPath.get();
            }
        }
        return;
    }
    if (m_propertyType == PointsProperty) {
        if (!percentage)
            results->m_animatedPoints = m_fromPoints;
        else if (percentage == 1.f)
            results->m_animatedPoints = m_toPoints;
        else {
            if (!m_fromPoints.isEmpty() && !m_toPoints.isEmpty())
                SVGPointList::createAnimated(m_fromPoints, m_toPoints, results->m_animatedPoints, percentage);
            else
                results->m_animatedPoints.clear();
            // Fall back to discrete animation if the points are not compatible
            if (results->m_animatedPoints.isEmpty())
                results->m_animatedPoints = ((animationMode == FromToAnimation && percentage > 0.5f) || animationMode == ToAnimation || percentage == 1.0f) 
                    ? m_toPoints : m_fromPoints;
        }
        return;
    }
    ASSERT(animationMode == FromToAnimation || animationMode == ToAnimation || animationMode == ValuesAnimation);
    if ((animationMode == FromToAnimation && percentage > 0.5f) || animationMode == ToAnimation || percentage == 1.0f)
        results->m_animatedString = m_toString;
    else
        results->m_animatedString = m_fromString;
    // Higher priority replace animation overrides any additive results so far.
    results->m_propertyType = StringProperty;
}

bool SVGAnimateElement::calculateFromAndToValues(const String& fromString, const String& toString)
{
    // FIXME: Needs more solid way determine target attribute type.
    m_propertyType = determinePropertyType(attributeName());
    if (m_propertyType == ColorProperty) {
        m_fromColor = SVGColor::colorFromRGBColorString(fromString);
        m_toColor = SVGColor::colorFromRGBColorString(toString);
        if ((m_fromColor.isValid() && m_toColor.isValid()) || (m_toColor.isValid() && animationMode() == ToAnimation))
            return true;
    } else if (m_propertyType == NumberProperty) {
        m_numberUnit = String();
        if (parseNumberValueAndUnit(toString, m_toNumber, m_numberUnit)) {
            // For to-animations the from number is calculated later
            if (animationMode() == ToAnimation || parseNumberValueAndUnit(fromString, m_fromNumber, m_numberUnit))
                return true;
        }
    } else if (m_propertyType == PathProperty) {
        SVGPathParserFactory* factory = SVGPathParserFactory::self();
        if (factory->buildSVGPathByteStreamFromString(fromString, m_fromPath, UnalteredParsing)) {
            if (factory->buildSVGPathByteStreamFromString(toString, m_toPath, UnalteredParsing))
                return true;
        }
        m_fromPath.clear();
        m_toPath.clear();
    } else if (m_propertyType == PointsProperty) {
        m_fromPoints.clear();
        if (pointsListFromSVGData(m_fromPoints, fromString)) {
            m_toPoints.clear();
            if (pointsListFromSVGData(m_toPoints, toString))
                return true;
        }
    }
    m_fromString = fromString;
    m_toString = toString;
    m_propertyType = StringProperty;
    return true;
}

bool SVGAnimateElement::calculateFromAndByValues(const String& fromString, const String& byString)
{
    ASSERT(!hasTagName(SVGNames::setTag));
    m_propertyType = determinePropertyType(attributeName());
    if (m_propertyType == ColorProperty) {
        m_fromColor = fromString.isEmpty() ? Color() : SVGColor::colorFromRGBColorString(fromString);
        m_toColor = ColorDistance::addColorsAndClamp(m_fromColor, SVGColor::colorFromRGBColorString(byString));
        if (!m_fromColor.isValid() || !m_toColor.isValid())
            return false;
    } else {
        m_numberUnit = String();
        m_fromNumber = 0;
        if (!fromString.isEmpty() && !parseNumberValueAndUnit(fromString, m_fromNumber, m_numberUnit))
            return false;
        if (!parseNumberValueAndUnit(byString, m_toNumber, m_numberUnit))
            return false;
        m_toNumber += m_fromNumber;
    }
    return true;
}

void SVGAnimateElement::resetToBaseValue(const String& baseString)
{
    m_animatedString = baseString;
    PropertyType lastType = m_propertyType;
    m_propertyType = determinePropertyType(attributeName());
    if (m_propertyType == ColorProperty) {
        m_animatedColor = baseString.isEmpty() ? Color() : SVGColor::colorFromRGBColorString(baseString);
        if (isContributing(elapsed())) {
            m_propertyType = lastType;
            return;
        }
    } else if (m_propertyType == NumberProperty) {
        if (baseString.isEmpty()) {
            m_animatedNumber = 0;
            m_numberUnit = String();
            return;
        }
        if (parseNumberValueAndUnit(baseString, m_animatedNumber, m_numberUnit))
            return;
    } else if (m_propertyType == PathProperty) {
        m_animatedPath.clear();
        m_animatedPathPointer = 0;
        return;
    } else if (m_propertyType == PointsProperty) {
        m_animatedPoints.clear();
        return;
    }
    m_propertyType = StringProperty;
}
    
void SVGAnimateElement::applyResultsToTarget()
{
    String valueToApply;
    if (m_propertyType == ColorProperty)
        valueToApply = m_animatedColor.name();
    else if (m_propertyType == NumberProperty)
        valueToApply = String::number(m_animatedNumber) + m_numberUnit;
    else if (m_propertyType == PathProperty) {
        if (!m_animatedPathPointer || m_animatedPathPointer->isEmpty())
            valueToApply = m_animatedString;
        else {
            // We need to keep going to string and back because we are currently only able to paint
            // "processed" paths where complex shapes are replaced with simpler ones. Path 
            // morphing needs to be done with unprocessed paths.
            // FIXME: This could be optimized if paths were not processed at parse time.
            SVGPathParserFactory* factory = SVGPathParserFactory::self();
            factory->buildStringFromByteStream(m_animatedPathPointer, valueToApply, UnalteredParsing);
        }
    } else if (m_propertyType == PointsProperty)
        valueToApply = m_animatedPoints.isEmpty() ? m_animatedString : m_animatedPoints.valueAsString();
    else
        valueToApply = m_animatedString;
    
    setTargetAttributeAnimatedValue(valueToApply);
}
    
float SVGAnimateElement::calculateDistance(const String& fromString, const String& toString)
{
    m_propertyType = determinePropertyType(attributeName());
    if (m_propertyType == NumberProperty) {
        double from;
        double to;
        String unit;
        if (!parseNumberValueAndUnit(fromString, from, unit))
            return -1.f;
        if (!parseNumberValueAndUnit(toString, to, unit))
            return -1.f;
        return narrowPrecisionToFloat(fabs(to - from));
    }
    if (m_propertyType == ColorProperty) {
        Color from = SVGColor::colorFromRGBColorString(fromString);
        if (!from.isValid())
            return -1.f;
        Color to = SVGColor::colorFromRGBColorString(toString);
        if (!to.isValid())
            return -1.f;
        return ColorDistance(from, to).distance();
    }
    return -1.f;
}
   
}

// vim:ts=4:noet
#endif // ENABLE(SVG)

