/*
 * Copyright (C) 2008 Holger Hans Peter Freyther
 * Copyright (C) 2011 Samsung Electronics
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
#include "GeolocationServiceNetflix.h"

#if ENABLE(GEOLOCATION)
#include "NotImplemented.h"
#include "PositionOptions.h"
#include <wtf/text/CString.h>

namespace WebCore {

GeolocationService::FactoryFunction* GeolocationService::s_factoryFunction = &GeolocationServiceNetflix::create;

PassOwnPtr<GeolocationService> GeolocationServiceNetflix::create(GeolocationServiceClient* client)
{
    return new GeolocationServiceNetflix(client);
}

GeolocationServiceNetflix::GeolocationServiceNetflix(GeolocationServiceClient* client)
    : GeolocationService(client)
{
}

GeolocationServiceNetflix::~GeolocationServiceNetflix()
{
    notImplemented();
}

bool GeolocationServiceNetflix::startUpdating(PositionOptions* options)
{
    notImplemented();
    return false;
}

void GeolocationServiceNetflix::stopUpdating()
{
    notImplemented();
}

void GeolocationServiceNetflix::suspend()
{
    notImplemented();
}

void GeolocationServiceNetflix::resume()
{
    notImplemented();
}

Geoposition* GeolocationServiceNetflix::lastPosition() const
{
    return m_lastPosition.get();
}

PositionError* GeolocationServiceNetflix::lastError() const
{
    return m_lastError.get();
}

}
#endif // ENABLE(GEOLOCATION)
