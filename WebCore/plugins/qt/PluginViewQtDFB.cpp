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

#include "config.h"
#include "PluginViewQtDFB.h"

#include "Frame.h"
#include "FrameView.h"
#include "GraphicsContext.h"
#include "HostWindow.h"
#include "KeyboardEvent.h"
#include "MouseEvent.h"
#include "PlatformKeyboardEvent.h"
#include "PlatformMouseEvent.h"
#include "PluginView.h"
#include "QWebPageClient.h"
#include "RenderObject.h"
#include <QGraphicsView>
#include <QGraphicsWebView>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QVariant>
#include <QWidget>

QT_BEGIN_NAMESPACE
extern Q_GUI_EXPORT IDirectFBSurface* qt_directfb_surface_for_pixmap(const QPixmap &pixmap);
QT_END_NAMESPACE

namespace WebCore {
void PluginViewQtDFB::clearWindowInfo(PluginViewQtDFB::DFBWindowInfo *windowInfo)
{
    if (windowInfo->window)
        windowInfo->window->Release(windowInfo->window);
    if (windowInfo->layer)
        windowInfo->layer->Release(windowInfo->layer);
    if (windowInfo->dfb)
        windowInfo->dfb->Release(windowInfo->dfb);
}

PluginViewQtDFB::PluginViewQtDFB(PluginView* pluginView,
                                         const DFBWindowInfo& windowInfo)
    : m_pluginView(pluginView), m_dfbSurface(0), m_installedEventFilter(false), m_dfbWindowInfo(windowInfo)
{
}

PluginViewQtDFB::~PluginViewQtDFB()
{
    clearWindowInfo(&m_dfbWindowInfo);
}

PluginViewQtDFB* PluginViewQtDFB::create(PluginView* pluginView)
{
    ASSERT(pluginView);
    QWebPageClient* client = pluginView->m_parentFrame->view()->hostWindow()->platformPageClient();
    if (!client || !client->ownerWidget())
        return 0;

    DFBWindowInfo windowInfo = { 0, 0, 0 };
    DFBResult result = DirectFBCreate(&windowInfo.dfb);
    if (result != DFB_OK) {
        DirectFBError("PluginViewQtDFB::create", result);
        return 0;
    }

    result = windowInfo.dfb->GetDisplayLayer(windowInfo.dfb, DLID_PRIMARY, &windowInfo.layer);
    if (result != DFB_OK) {
        DirectFBError("PluginViewQtDFB::create", result);
        clearWindowInfo(&windowInfo);
        return 0;
    }

    result = windowInfo.layer->GetWindow(windowInfo.layer,
                                         client->ownerWidget()->property("_q_DirectFBWindowID").toInt(),
                                         &windowInfo.window);
    if (result != DFB_OK) {
        DirectFBError("PluginViewQtDFB::create", result);
        clearWindowInfo(&windowInfo);
        return 0;
    }

    return new PluginViewQtDFB(pluginView, windowInfo);
}

void PluginViewQtDFB::paint(GraphicsContext* context, const IntRect& rect)
{
    DFBEvent dfbEvent;
    memset(&dfbEvent, 0, sizeof(DFBEvent));
    DFBUserEvent& userEvent = dfbEvent.user;
    userEvent.clazz = DFEC_USER;
    userEvent.type = 0xdfb;
    userEvent.data = &m_pluginView->m_npWindow;
    m_pluginView->dispatchNPEvent(dfbEvent);
    QPainter* painter = context->platformContext();
    IntRect exposedRect(rect);
    const IntRect frameRect = m_pluginView->frameRect();
    exposedRect.intersect(frameRect);
    exposedRect.move(-frameRect.x(), -frameRect.y());
    const QPainter::CompositionMode oldMode = painter->compositionMode();
    painter->setCompositionMode(QPainter::CompositionMode_Source);
    painter->drawPixmap(QPoint(frameRect.x() + exposedRect.x(), frameRect.y() + exposedRect.y()), m_pixmap,
                        exposedRect);
    painter->setCompositionMode(oldMode);
}

bool PluginViewQtDFB::dispatchEvent(const KeyboardEvent* event)
{
    const QKeyEvent* qtEvent = event->keyEvent()->qtEvent();
    DFBEvent dfbEvent;
    memset(&dfbEvent, 0, sizeof(DFBEvent));
    DFBInputEvent& input = dfbEvent.input;
    input.clazz = DFEC_INPUT;
    input.type = (qtEvent->type() == QEvent::KeyPress ? DIET_KEYPRESS : DIET_KEYRELEASE);
    unsigned int flags = (DIEF_KEYSYMBOL | DIEF_MODIFIERS);
    if (qtEvent->isAutoRepeat())
        flags |= DIEF_REPEAT;
    const Qt::KeyboardModifier modifiers[] = { Qt::ShiftModifier, Qt::ControlModifier, Qt::AltModifier, Qt::MetaModifier };
    const DFBInputDeviceModifierMask dfbModifiers[] = { DIMM_SHIFT, DIMM_CONTROL, DIMM_ALT, DIMM_META };
    for (int i = 0; i < 4; ++i) {
        if (qtEvent->modifiers() & modifiers[i])
            input.modifiers = DFBInputDeviceModifierMask(input.modifiers | dfbModifiers[i]);
    }
    switch (qtEvent->key()) {
    case Qt::Key_Backspace:
        input.key_symbol = DIKS_BACKSPACE;
        input.key_id = DIKI_BACKSPACE;
        break;
    case Qt::Key_Tab:
        input.key_symbol = DIKS_TAB;
        input.key_id = DIKI_TAB;
        break;
    case Qt::Key_Return:
        input.key_symbol = DIKS_RETURN;
        input.key_id = DIKI_KP_ENTER;
        break;
    case Qt::Key_Escape:
        input.key_symbol = DIKS_ESCAPE;
        input.key_id = DIKI_ESCAPE;
        break;
    case Qt::Key_Delete:
        input.key_symbol = DIKS_DELETE;
        input.key_id = DIKI_DELETE;
        break;
    case Qt::Key_Left:
        input.key_symbol = DIKS_CURSOR_LEFT;
        input.key_id = DIKI_LEFT;
        break;
    case Qt::Key_Right:
        input.key_symbol = DIKS_CURSOR_RIGHT;
        input.key_id = DIKI_RIGHT;
        break;
    case Qt::Key_Up:
        input.key_symbol = DIKS_CURSOR_UP;
        input.key_id = DIKI_UP;
        break;
    case Qt::Key_Down:
        input.key_symbol = DIKS_CURSOR_DOWN;
        input.key_id = DIKI_DOWN;
        break;
    case Qt::Key_Insert:
        input.key_symbol = DIKS_INSERT;
        input.key_id = DIKI_INSERT;
        break;
    case Qt::Key_Home:
        input.key_symbol = DIKS_HOME;
        input.key_id = DIKI_HOME;
        break;
    case Qt::Key_End:
        input.key_symbol = DIKS_END;
        input.key_id = DIKI_END;
        break;
    case Qt::Key_PageUp:
        input.key_symbol = DIKS_PAGE_UP;
        input.key_id = DIKI_PAGE_UP;
        break;
    case Qt::Key_PageDown:
        input.key_symbol = DIKS_PAGE_DOWN;
        input.key_id = DIKI_PAGE_DOWN;
        break;
    case Qt::Key_Print:
        input.key_symbol = DIKS_PRINT;
        break;
    case Qt::Key_Pause:
        input.key_symbol = DIKS_PAUSE;
        break;
    case Qt::Key_Select:
        input.key_symbol = DIKS_SELECT;
        break;
    case Qt::Key_OpenUrl:
        input.key_symbol = DIKS_GOTO;
        break;
    case Qt::Key_Clear:
        input.key_symbol = DIKS_CLEAR;
        break;
    case Qt::Key_Menu:
        input.key_symbol = DIKS_MENU;
        break;
    case Qt::Key_Help:
        input.key_symbol = DIKS_HELP;
        break;
    case Qt::Key_HomePage:
        input.key_symbol = DIKS_INTERNET;
        break;
    case Qt::Key_LaunchMail:
        input.key_symbol = DIKS_MAIL;
        break;
    case Qt::Key_Favorites:
        input.key_symbol = DIKS_FAVORITES;
        break;
    case Qt::Key_Back:
        input.key_symbol = DIKS_BACK;
        break;
    case Qt::Key_Forward:
        input.key_symbol = DIKS_FORWARD;
        break;
    case Qt::Key_VolumeUp:
        input.key_symbol = DIKS_VOLUME_UP;
        break;
    case Qt::Key_VolumeDown:
        input.key_symbol = DIKS_VOLUME_DOWN;
        break;
    case Qt::Key_VolumeMute:
        input.key_symbol = DIKS_MUTE;
        break;
    case Qt::Key_MediaPlay:
        input.key_symbol = DIKS_PLAY;
        break;
    case Qt::Key_MediaStop:
        input.key_symbol = DIKS_STOP;
        break;
    case Qt::Key_MediaRecord:
        input.key_symbol = DIKS_RECORD;
        break;
    case Qt::Key_MediaPrevious:
        input.key_symbol = DIKS_PREVIOUS;
        break;
    case Qt::Key_MediaNext:
        input.key_symbol = DIKS_NEXT;
        break;
    case Qt::Key_F1:
        input.key_symbol = DIKS_F1;
        input.key_id = DIKI_F1;
        break;
    case Qt::Key_F2:
        input.key_symbol = DIKS_F2;
        input.key_id = DIKI_F2;
        break;
    case Qt::Key_F3:
        input.key_symbol = DIKS_F3;
        input.key_id = DIKI_F3;
        break;
    case Qt::Key_F4:
        input.key_symbol = DIKS_F4;
        input.key_id = DIKI_F4;
        break;
    case Qt::Key_F5:
        input.key_symbol = DIKS_F5;
        input.key_id = DIKI_F5;
        break;
    case Qt::Key_F6:
        input.key_symbol = DIKS_F6;
        input.key_id = DIKI_F6;
        break;
    case Qt::Key_F7:
        input.key_symbol = DIKS_F7;
        input.key_id = DIKI_F7;
        break;
    case Qt::Key_F8:
        input.key_symbol = DIKS_F8;
        input.key_id = DIKI_F8;
        break;
    case Qt::Key_F9:
        input.key_symbol = DIKS_F9;
        input.key_id = DIKI_F9;
        break;
    case Qt::Key_F10:
        input.key_symbol = DIKS_F10;
        input.key_id = DIKI_F10;
        break;
    case Qt::Key_F11:
        input.key_symbol = DIKS_F11;
        input.key_id = DIKI_F11;
        break;
    case Qt::Key_F12:
        input.key_symbol = DIKS_F12;
        input.key_id = DIKI_F12;
        break;
    case Qt::Key_Shift:
        input.key_symbol = DIKS_SHIFT;
        input.key_id = DIKI_SHIFT_L;
        break;
    case Qt::Key_Control:
        input.key_symbol = DIKS_CONTROL;
        input.key_id = DIKI_CONTROL_L;
        break;
    case Qt::Key_Alt:
        input.key_symbol = DIKS_ALT;
        input.key_id = DIKI_ALT_L;
        break;
    case Qt::Key_AltGr:
        input.key_symbol = DIKS_ALTGR;
        break;
    case Qt::Key_Meta:
        input.key_symbol = DIKS_META;
        input.key_id = DIKI_META_L;
        break;
    case Qt::Key_Super_L:
        input.key_symbol = DIKS_SUPER;
        input.key_id = DIKI_SUPER_L;
        break;
    case Qt::Key_Hyper_L:
        input.key_symbol = DIKS_HYPER;
        input.key_id = DIKI_HYPER_L;
        break;
    case Qt::Key_CapsLock:
        input.key_symbol = DIKS_CAPS_LOCK;
        input.key_id = DIKI_CAPS_LOCK;
        break;
    case Qt::Key_NumLock:
        input.key_symbol = DIKS_NUM_LOCK;
        input.key_id = DIKI_NUM_LOCK;
        break;
    case Qt::Key_ScrollLock:
        input.key_symbol = DIKS_SCROLL_LOCK;
        input.key_id = DIKI_SCROLL_LOCK;
        break;
    case Qt::Key_Dead_Abovedot:
        input.key_symbol = DIKS_DEAD_ABOVEDOT;
        break;
    case Qt::Key_Dead_Abovering:
        input.key_symbol = DIKS_DEAD_ABOVERING;
        break;
    case Qt::Key_Dead_Acute:
        input.key_symbol = DIKS_DEAD_ACUTE;
        break;
    case Qt::Key_Dead_Breve:
        input.key_symbol = DIKS_DEAD_BREVE;
        break;
    case Qt::Key_Dead_Caron:
        input.key_symbol = DIKS_DEAD_CARON;
        break;
    case Qt::Key_Dead_Cedilla:
        input.key_symbol = DIKS_DEAD_CEDILLA;
        break;
    case Qt::Key_Dead_Circumflex:
        input.key_symbol = DIKS_DEAD_CIRCUMFLEX;
        break;
    case Qt::Key_Dead_Diaeresis:
        input.key_symbol = DIKS_DEAD_DIAERESIS;
        break;
    case Qt::Key_Dead_Doubleacute:
        input.key_symbol = DIKS_DEAD_DOUBLEACUTE;
        break;
    case Qt::Key_Dead_Grave:
        input.key_symbol = DIKS_DEAD_GRAVE;
        break;
    case Qt::Key_Dead_Iota:
        input.key_symbol = DIKS_DEAD_IOTA;
        break;
    case Qt::Key_Dead_Macron:
        input.key_symbol = DIKS_DEAD_MACRON;
        break;
    case Qt::Key_Dead_Ogonek:
        input.key_symbol = DIKS_DEAD_OGONEK;
        break;
    case Qt::Key_Dead_Semivoiced_Sound:
        input.key_symbol = DIKS_DEAD_SEMIVOICED_SOUND;
        break;
    case Qt::Key_Dead_Tilde:
        input.key_symbol = DIKS_DEAD_TILDE;
        break;
    case Qt::Key_Dead_Voiced_Sound:
        input.key_symbol = DIKS_DEAD_VOICED_SOUND;
        break;
    case Qt::Key_Space:
        input.key_symbol = DIKS_SPACE;
        input.key_id = DIKI_SPACE;
        break;
    case Qt::Key_Exclam:
        input.key_symbol = DIKS_EXCLAMATION_MARK;
        break;
    case Qt::Key_QuoteDbl:
        input.key_symbol = DIKS_QUOTATION;
        break;
    case Qt::Key_NumberSign:
        input.key_symbol = DIKS_NUMBER_SIGN;
        break;
    case Qt::Key_Dollar:
        input.key_symbol = DIKS_DOLLAR_SIGN;
        break;
    case Qt::Key_Percent:
        input.key_symbol = DIKS_PERCENT_SIGN;
        break;
    case Qt::Key_Ampersand:
        input.key_symbol = DIKS_AMPERSAND;
        break;
    case Qt::Key_Apostrophe:
        input.key_symbol = DIKS_APOSTROPHE;
        break;
    case Qt::Key_ParenLeft:
        input.key_symbol = DIKS_PARENTHESIS_LEFT;
        break;
    case Qt::Key_ParenRight:
        input.key_symbol = DIKS_PARENTHESIS_RIGHT;
        break;
    case Qt::Key_Asterisk:
        input.key_symbol = DIKS_ASTERISK;
        break;
    case Qt::Key_Plus:
        input.key_symbol = DIKS_PLUS_SIGN;
        if (qtEvent->modifiers() & Qt::KeypadModifier)
            input.key_id = DIKI_KP_PLUS;
        break;
    case Qt::Key_Comma:
        input.key_symbol = DIKS_COMMA;
        input.key_id = DIKI_COMMA;
        break;
    case Qt::Key_Minus:
        input.key_symbol = DIKS_MINUS_SIGN;
        input.key_id = DIKI_MINUS_SIGN;
        break;
    case Qt::Key_Period:
        input.key_symbol = DIKS_PERIOD;
        input.key_id = DIKI_PERIOD;
        break;
    case Qt::Key_Slash:
        input.key_symbol = DIKS_SLASH;
        input.key_id = DIKI_SLASH;
        break;
    case Qt::Key_0:
        input.key_symbol = DIKS_0;
        input.key_id = DIKI_0;
        break;
    case Qt::Key_1:
        input.key_symbol = DIKS_1;
        input.key_id = DIKI_1;
        break;
    case Qt::Key_2:
        input.key_symbol = DIKS_2;
        input.key_id = DIKI_2;
        break;
    case Qt::Key_3:
        input.key_symbol = DIKS_3;
        input.key_id = DIKI_3;
        break;
    case Qt::Key_4:
        input.key_symbol = DIKS_4;
        input.key_id = DIKI_4;
        break;
    case Qt::Key_5:
        input.key_symbol = DIKS_5;
        input.key_id = DIKI_5;
        break;
    case Qt::Key_6:
        input.key_symbol = DIKS_6;
        input.key_id = DIKI_6;
        break;
    case Qt::Key_7:
        input.key_symbol = DIKS_7;
        input.key_id = DIKI_7;
        break;
    case Qt::Key_8:
        input.key_symbol = DIKS_8;
        input.key_id = DIKI_8;
        break;
    case Qt::Key_9:
        input.key_symbol = DIKS_9;
        input.key_id = DIKI_9;
        break;
    case Qt::Key_Colon:
        input.key_symbol = DIKS_COLON;
        break;
    case Qt::Key_Semicolon:
        input.key_symbol = DIKS_SEMICOLON;
        input.key_id = DIKI_SEMICOLON;
        break;
    case Qt::Key_Less:
        input.key_symbol = DIKS_LESS_THAN_SIGN;
        input.key_id = DIKI_LESS_SIGN;
        break;
    case Qt::Key_Equal:
        input.key_symbol = DIKS_EQUALS_SIGN;
        input.key_id = DIKI_EQUALS_SIGN;
        break;
    case Qt::Key_Greater:
        input.key_symbol = DIKS_GREATER_THAN_SIGN;
        break;
    case Qt::Key_Question:
        input.key_symbol = DIKS_QUESTION_MARK;
        break;
    case Qt::Key_At:
        input.key_symbol = DIKS_AT;
        break;
    case Qt::Key_A:
        input.key_symbol = DIKS_SMALL_A;
        input.key_id = DIKI_A;
        break;
    case Qt::Key_B:
        input.key_symbol = DIKS_SMALL_B;
        input.key_id = DIKI_B;
        break;
    case Qt::Key_C:
        input.key_symbol = DIKS_SMALL_C;
        input.key_id = DIKI_C;
        break;
    case Qt::Key_D:
        input.key_symbol = DIKS_SMALL_D;
        input.key_id = DIKI_D;
        break;
    case Qt::Key_E:
        input.key_symbol = DIKS_SMALL_E;
        input.key_id = DIKI_E;
        break;
    case Qt::Key_F:
        input.key_symbol = DIKS_SMALL_F;
        input.key_id = DIKI_F;
        break;
    case Qt::Key_G:
        input.key_symbol = DIKS_SMALL_G;
        input.key_id = DIKI_G;
        break;
    case Qt::Key_H:
        input.key_symbol = DIKS_SMALL_H;
        input.key_id = DIKI_H;
        break;
    case Qt::Key_I:
        input.key_symbol = DIKS_SMALL_I;
        input.key_id = DIKI_I;
        break;
    case Qt::Key_J:
        input.key_symbol = DIKS_SMALL_J;
        input.key_id = DIKI_J;
        break;
    case Qt::Key_K:
        input.key_symbol = DIKS_SMALL_K;
        input.key_id = DIKI_K;
        break;
    case Qt::Key_L:
        input.key_symbol = DIKS_SMALL_L;
        input.key_id = DIKI_L;
        break;
    case Qt::Key_M:
        input.key_symbol = DIKS_SMALL_M;
        input.key_id = DIKI_M;
        break;
    case Qt::Key_N:
        input.key_symbol = DIKS_SMALL_N;
        input.key_id = DIKI_N;
        break;
    case Qt::Key_O:
        input.key_symbol = DIKS_SMALL_O;
        input.key_id = DIKI_O;
        break;
    case Qt::Key_P:
        input.key_symbol = DIKS_SMALL_P;
        input.key_id = DIKI_P;
        break;
    case Qt::Key_Q:
        input.key_symbol = DIKS_SMALL_Q;
        input.key_id = DIKI_Q;
        break;
    case Qt::Key_R:
        input.key_symbol = DIKS_SMALL_R;
        input.key_id = DIKI_R;
        break;
    case Qt::Key_S:
        input.key_symbol = DIKS_SMALL_S;
        input.key_id = DIKI_S;
        break;
    case Qt::Key_T:
        input.key_symbol = DIKS_SMALL_T;
        input.key_id = DIKI_T;
        break;
    case Qt::Key_U:
        input.key_symbol = DIKS_SMALL_U;
        input.key_id = DIKI_U;
        break;
    case Qt::Key_V:
        input.key_symbol = DIKS_SMALL_V;
        input.key_id = DIKI_V;
        break;
    case Qt::Key_W:
        input.key_symbol = DIKS_SMALL_W;
        input.key_id = DIKI_W;
        break;
    case Qt::Key_X:
        input.key_symbol = DIKS_SMALL_X;
        input.key_id = DIKI_X;
        break;
    case Qt::Key_Y:
        input.key_symbol = DIKS_SMALL_Y;
        input.key_id = DIKI_Y;
        break;
    case Qt::Key_Z:
        input.key_symbol = DIKS_SMALL_Z;
        input.key_id = DIKI_Z;
        break;
    case Qt::Key_BracketLeft:
        input.key_symbol = DIKS_SQUARE_BRACKET_LEFT;
        break;
    case Qt::Key_Backslash:
        input.key_symbol = DIKS_BACKSLASH;
        input.key_id = DIKI_BACKSLASH;
        break;
    case Qt::Key_BracketRight:
        input.key_symbol = DIKS_SQUARE_BRACKET_RIGHT;
        break;
    case Qt::Key_AsciiCircum:
        input.key_symbol = DIKS_CIRCUMFLEX_ACCENT;
        break;
    case Qt::Key_Underscore:
        input.key_symbol = DIKS_UNDERSCORE;
        break;
    case Qt::Key_BraceLeft:
        input.key_symbol = DIKS_CURLY_BRACKET_LEFT;
        break;
    case Qt::Key_Bar:
        input.key_symbol = DIKS_VERTICAL_BAR;
        break;
    case Qt::Key_BraceRight:
        input.key_symbol = DIKS_CURLY_BRACKET_RIGHT;
        break;
    case Qt::Key_AsciiTilde:
        input.key_symbol = DIKS_TILDE;
        break;
    }
    if ((qtEvent->modifiers() & Qt::ShiftModifier) && qtEvent->key() >= Qt::Key_A && qtEvent->key() <= Qt::Key_Z)
        input.key_symbol = DFBInputDeviceKeySymbol((input.key_symbol - DIKS_SMALL_A) + DIKS_CAPITAL_A);
    if (input.key_id != DIKI_UNKNOWN) {
        flags |= DIEF_KEYID;
        if (input.key_id >= DIKI_0 && input.key_id <= DIKI_9 && qtEvent->modifiers() & Qt::KeypadModifier)
            input.key_id = DFBInputDeviceKeyIdentifier(input.key_id - DIKI_0 + DIKI_KP_0);
    }
    input.flags = DFBInputEventFlags(flags);
    return m_pluginView->dispatchNPEvent(dfbEvent);
}

bool PluginViewQtDFB::dispatchEvent(const MouseEvent* event)
{
    DFBEvent dfbEvent;
    memset(&dfbEvent, 0, sizeof(DFBEvent));
    DFBWindowEvent& window = dfbEvent.window;
    m_dfbWindowInfo.window->GetID(m_dfbWindowInfo.window, &window.window_id);
    window.clazz = DFEC_WINDOW;
    IntPoint postZoomPos = roundedIntPoint(m_pluginView->m_element->renderer()->absoluteToLocal(event->absoluteLocation()));
    if (event->type() == eventNames().mouseoverEvent || event->type() == eventNames().mouseoutEvent) {
        // enter/leave
        window.type = (event->type() == eventNames().mouseoverEvent ? DWET_ENTER : DWET_LEAVE);
        window.x = dfbEvent.window.cx = postZoomPos.x();
        window.y = dfbEvent.window.cy = postZoomPos.y();
    } else {
        // mouse
        unsigned int modifiers = 0;
        if (event->ctrlKey())
            modifiers |= DIMM_CONTROL;
        if (event->shiftKey())
            modifiers |= DIMM_SHIFT;
        if (event->altKey())
            modifiers |= DIMM_ALT;
        if (event->metaKey())
            modifiers |= DIMM_META;
        window.modifiers = DFBInputDeviceModifierMask(modifiers);

        if (event->type() == eventNames().mousedownEvent || event->type() == eventNames().mouseupEvent) {
            window.type = (event->type() == eventNames().mousedownEvent ? DWET_BUTTONDOWN : DWET_BUTTONUP);
            switch (event->button()) {
            case LeftButton:
                window.button = DIBI_LEFT;
                break;
            case MiddleButton:
                window.button = DIBI_MIDDLE;
                break;
            case RightButton:
                window.button = DIBI_RIGHT;
                break;
            default:
                break;
            }
            window.buttons = DFBInputDeviceButtonMask(0);
        } else if (event->type() == eventNames().mousemoveEvent) {
            window.type = DWET_MOTION;
        } else {
            return false;
        }
    }

    window.x = postZoomPos.x();
    window.y = postZoomPos.y();
    const IntPoint absoluteLocation = event->absoluteLocation();
    window.cx = absoluteLocation.x();
    window.cy = absoluteLocation.y();

    return m_pluginView->dispatchNPEvent(dfbEvent);
}

bool PluginViewQtDFB::dispatchEvent(EventType type)
{
    DFBEvent dfbEvent;
    memset(&dfbEvent, 0, sizeof(DFBEvent));
    DFBWindowEvent& window = dfbEvent.window;
    window.clazz = DFEC_WINDOW;
    switch (type) {
    case FocusIn: window.type = DWET_GOTFOCUS; break;
    case FocusOut: window.type = DWET_LOSTFOCUS; break;
    default: return false;
    }
    return m_pluginView->dispatchNPEvent(dfbEvent);
}

bool PluginViewQtDFB::eventFilter(QObject* , QEvent* e)
{
    if (e->type() == QEvent::Move || e->type() == QEvent::Resize) {
        m_pluginView->m_hasPendingGeometryChange = true;
        m_timer.start(10, this);
    }
    return false;
}

void PluginViewQtDFB::timerEvent(QTimerEvent* e)
{
    if (e->timerId() == m_timer.timerId()) {
        m_timer.stop();
        m_pluginView->forceRedraw();
    }
}

void PluginViewQtDFB::setNPWindowIfNeeded()
{
    QPoint pluginViewGlobalPos = m_pluginView->windowClipRect().topLeft();
    QWebPageClient* client = m_pluginView->m_parentFrame->view()->hostWindow()->platformPageClient();
    if (QWidget* ownerWidget = client ? client->ownerWidget() : 0) {
        if (!m_installedEventFilter) {
            QWidget* watched = ownerWidget;
            while (true) {
                watched->installEventFilter(this);
                if (watched->isWindow())
                    break;
                watched = watched->parentWidget();
                if (!watched)
                    break;
            }
            m_installedEventFilter = true;
        }

        pluginViewGlobalPos = ownerWidget->mapToGlobal(pluginViewGlobalPos);
        if (const QGraphicsWebView* item = qobject_cast<const QGraphicsWebView*>(client->pluginParent())) {
            Q_ASSERT(item->scene() && !item->scene()->views().isEmpty());
            const QGraphicsView* view = item->scene()->views().value(0);
            const QPoint add = view->mapFromScene(item->pos());
            pluginViewGlobalPos += add;
        }
    }

    if ((QSize)(m_pluginView->m_windowRect.size()) != m_pixmap.size()) {
        m_pixmap = QPixmap(m_pluginView->m_windowRect.size());
        m_pixmap.fill(Qt::transparent);
        m_dfbSurface = QT_PREPEND_NAMESPACE(qt_directfb_surface_for_pixmap(m_pixmap));
    }

    m_pluginView->m_npWindow.ws_info = &m_dfbWindowInfo;
    m_pluginView->m_npWindow.type = NPWindowTypeDrawable;

    m_pluginView->m_npWindow.x = pluginViewGlobalPos.x();
    m_pluginView->m_npWindow.y = pluginViewGlobalPos.y();
    const IntRect frameRect = m_pluginView->frameRect();
    m_pluginView->m_npWindow.width = frameRect.width();
    m_pluginView->m_npWindow.height = frameRect.height();
    m_pluginView->m_npWindow.clipRect.top = m_pluginView->m_clipRect.y();
    m_pluginView->m_npWindow.clipRect.left = m_pluginView->m_clipRect.x();
    m_pluginView->m_npWindow.clipRect.right = m_pluginView->m_clipRect.width() + m_pluginView->m_clipRect.x() - 1;
    m_pluginView->m_npWindow.clipRect.bottom = m_pluginView->m_clipRect.height() + m_pluginView->m_clipRect.y() - 1;
    m_pluginView->m_npWindow.window = m_dfbSurface;
}

bool PluginViewQtDFB::platformGetValue(NPNVariable variable, void* value, NPError* result)
{
    switch (variable) {
    case NPNVxDisplay:
        *reinterpret_cast<void**>(value) = m_dfbWindowInfo.dfb;
        *result = NPERR_NO_ERROR;
        return true;

    case NPNVxtAppContext:
        *result = NPERR_GENERIC_ERROR;
        return true;

    case NPNVnetscapeWindow:
        *reinterpret_cast<void**>(value) = m_dfbWindowInfo.window;
        *result = NPERR_NO_ERROR;
        return true;

    default:
        return false;
    }
}
} // namespace WebCore
