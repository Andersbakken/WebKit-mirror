/*
 *  Copyright (C) 2008 INdT - Instituto Nokia de Tecnologia
 *  Copyright (C) 2009-2010 ProFUSION embedded systems
 *  Copyright (C) 2009-2010 Samsung Electronics
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"
#include "ClipboardNetflix.h"

#include "Editor.h"
#include "FileList.h"
#include "NotImplemented.h"
#include <wtf/text/StringHash.h>

namespace WebCore {
PassRefPtr<Clipboard> Editor::newGeneralClipboard(ClipboardAccessPolicy policy, Frame*)
{
    return ClipboardNetflix::create(policy, Clipboard::CopyAndPaste);
}

PassRefPtr<Clipboard> Clipboard::create(ClipboardAccessPolicy, DragData*, Frame*)
{
    return 0;
}

ClipboardNetflix::ClipboardNetflix(ClipboardAccessPolicy policy, ClipboardType clipboardType)
    : Clipboard(policy, clipboardType)
{
    notImplemented();
}

ClipboardNetflix::~ClipboardNetflix()
{
    notImplemented();
}

void ClipboardNetflix::clearData(const String&)
{
    notImplemented();
}

void ClipboardNetflix::writePlainText(const WTF::String&)
{
    notImplemented();
}

void ClipboardNetflix::clearAllData()
{
    notImplemented();
}

String ClipboardNetflix::getData(const String&, bool &success) const
{
    notImplemented();
    success = false;
    return String();
}

bool ClipboardNetflix::setData(const String&, const String&)
{
    notImplemented();
    return false;
}

HashSet<String> ClipboardNetflix::types() const
{
    notImplemented();
    return HashSet<String>();
}

PassRefPtr<FileList> ClipboardNetflix::files() const
{
    notImplemented();
    return 0;
}

IntPoint ClipboardNetflix::dragLocation() const
{
    notImplemented();
    return IntPoint(0, 0);
}

CachedImage* ClipboardNetflix::dragImage() const
{
    notImplemented();
    return 0;
}

void ClipboardNetflix::setDragImage(CachedImage*, const IntPoint&)
{
    notImplemented();
}

Node* ClipboardNetflix::dragImageElement()
{
    notImplemented();
    return 0;
}

void ClipboardNetflix::setDragImageElement(Node*, const IntPoint&)
{
    notImplemented();
}

DragImageRef ClipboardNetflix::createDragImage(IntPoint&) const
{
    notImplemented();
    return 0;
}

void ClipboardNetflix::declareAndWriteDragImage(Element*, const KURL&, const String&, Frame*)
{
    notImplemented();
}

void ClipboardNetflix::writeURL(const KURL&, const String&, Frame*)
{
    notImplemented();
}

void ClipboardNetflix::writeRange(Range*, Frame*)
{
    notImplemented();
}

bool ClipboardNetflix::hasData()
{
    notImplemented();
    return false;
}

}
