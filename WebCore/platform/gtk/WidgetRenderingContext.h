/*
 * Copyright (C) 2010 Igalia S.L.
 * All rights reserved.
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
 *
 */

#ifndef WidgetRenderingContext_h
#define WidgetRenderingContext_h

#include "IntRect.h"
#include "gtkdrawing.h"

namespace WebCore {

class GraphicsContext;
class RenderThemeGtk;

class WidgetRenderingContext {
public:
    WidgetRenderingContext(GraphicsContext*, const IntRect&);
    ~WidgetRenderingContext();

    bool paintMozillaWidget(GtkThemeWidgetType, GtkWidgetState*, int flags, GtkTextDirection = GTK_TEXT_DIR_NONE);
    void gtkPaintBox(const IntRect&, GtkWidget*, GtkStateType, GtkShadowType, const gchar*);
    void gtkPaintFocus(const IntRect&, GtkWidget*, GtkStateType, const gchar*);
    void gtkPaintSlider(const IntRect&, GtkWidget*, GtkStateType, GtkShadowType, const gchar*, GtkOrientation);

private:
    GraphicsContext* m_graphicsContext;
    IntRect m_targetRect;
    GdkRectangle m_paintRect;
    IntSize m_extraSpace;
    bool m_hadError;

#ifdef GTK_API_VERSION_2
    GdkDrawable* m_target;
#else
    cairo_t* m_target;
#endif

};

}

#endif
