/*
 * Copyright (C) 2010 Anders Bakken <agbakken@gmail.com>
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
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

#ifndef PluginViewQtDFB_h
#define PluginViewQtDFB_h

#include "npapi.h"
#include <QBasicTimer>
#include <QObject>
#include <QPixmap>
#include <directfb.h>

namespace WebCore {
class PluginView;
class IntRect;
class GraphicsContext;
class MouseEvent;
class KeyboardEvent;
class PluginViewQtDFB : public QObject {
public:
    static PluginViewQtDFB* create(PluginView* pluginView);
    ~PluginViewQtDFB();
    void paint(GraphicsContext* context, const IntRect& rect);
    void setNPWindowIfNeeded();
    bool platformGetValue(NPNVariable variable, void* value, NPError* result);
    bool eventFilter(QObject* object, QEvent* event);
    void timerEvent(QTimerEvent* e);

    bool dispatchEvent(const MouseEvent* event);
    bool dispatchEvent(const KeyboardEvent* event);
    enum EventType {
        FocusIn,
        FocusOut
    };

    bool dispatchEvent(EventType type);
private:
    PluginView* m_pluginView;
    QPixmap m_pixmap;
    IDirectFBSurface *m_dfbSurface;
    QBasicTimer m_timer;
    bool m_installedEventFilter;
    struct DFBWindowInfo {
        IDirectFB* dfb;
        IDirectFBDisplayLayer* layer;
        IDirectFBWindow* window;
    } m_dfbWindowInfo;

    PluginViewQtDFB(PluginView* pluginView, const DFBWindowInfo& windowInfo);
    static void clearWindowInfo(DFBWindowInfo *windowInfo);
};
} // namespace WebCore

#endif
