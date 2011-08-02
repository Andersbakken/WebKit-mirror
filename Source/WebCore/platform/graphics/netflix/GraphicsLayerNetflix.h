/*
    Copyright (C) 2009-2011 Samsung Electronics

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

#ifndef GraphicsLayerNetflix_h
#define GraphicsLayerNetflix_h

#if USE(ACCELERATED_COMPOSITING)

#include "GraphicsLayer.h"

namespace WebCore {

class GraphicsLayerNetflix : public GraphicsLayer {
public:
    GraphicsLayerNetflix(GraphicsLayerClient*);
    virtual ~GraphicsLayerNetflix();

    virtual void setNeedsDisplay();
    virtual void setNeedsDisplayInRect(const FloatRect&);
};

} // namespace WebCore

#endif // USE(ACCELERATED_COMPOSITING)

#endif // GraphicsLayerNetflix_h
