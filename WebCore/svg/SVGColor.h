/*
 * Copyright (C) 2004, 2005 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006, 2007 Rob Buis <buis@kde.org>
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

#ifndef SVGColor_h
#define SVGColor_h
#if ENABLE(SVG)

#include "CSSValue.h"
#include "Color.h"
#include <wtf/PassRefPtr.h>

namespace WebCore {

class RGBColor;

class SVGColor : public CSSValue {
public:
    static PassRefPtr<SVGColor> create(const String& color)
    {
        return adoptRef(new SVGColor(color));
    }
    static PassRefPtr<SVGColor> create(const Color& color)
    {
        return adoptRef(new SVGColor(color));
    }
    static PassRefPtr<SVGColor> createCurrentColor()
    {
        return adoptRef(new SVGColor(SVG_COLORTYPE_CURRENTCOLOR));
    }

    enum SVGColorType {
        SVG_COLORTYPE_UNKNOWN = 0,
        SVG_COLORTYPE_RGBCOLOR = 1,
        SVG_COLORTYPE_RGBCOLOR_ICCCOLOR = 2,
        SVG_COLORTYPE_CURRENTCOLOR = 3
    };

    // 'SVGColor' functions
    unsigned short colorType() const { return m_colorType; }

    PassRefPtr<RGBColor> rgbColor() const;

    static Color colorFromRGBColorString(const String&);

    void setRGBColor(const String& rgbColor) { ExceptionCode ignored = 0; setRGBColor(rgbColor, ignored); }
    void setRGBColor(const String& rgbColor, ExceptionCode&);
    void setRGBColorICCColor(const String& rgbColor, const String& iccColor, ExceptionCode&);
    void setColor(unsigned short colorType, const String& rgbColor, const String& iccColor, ExceptionCode&);

    const Color& color() const { return m_color; }

protected:
    SVGColor();
    SVGColor(const String& color);
    SVGColor(const Color&);

    virtual String cssText() const;

private:
    SVGColor(SVGColorType);

    static void create(int); // compile-time guard 

    virtual bool isSVGColor() const { return true; }

    Color m_color;
    unsigned short m_colorType;
};

} // namespace WebCore

#endif // ENABLE(SVG)
#endif // SVGColor_h

// vim:ts=4:noet
