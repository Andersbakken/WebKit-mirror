
#include "config.h"

#include "EditorClientNetflix.h"
#include "Frame.h"
#include "KeyboardEvent.h"
#include "Node.h"
#include "Page.h"
#include "PlatformKeyboardEvent.h"
#include "WebViewNetflix.h"
#include "NotImplemented.h"

namespace WebKit {

EditorClientNetflix::EditorClientNetflix(WebViewNetflix* webview)
    : m_webview(webview)
{
}

EditorClientNetflix::~EditorClientNetflix()
{
}

void EditorClientNetflix::pageDestroyed()
{
    delete this;
}

void EditorClientNetflix::clearUndoRedoOperations()
{
    notImplemented();
}

void EditorClientNetflix::handleKeyboardEvent(WebCore::KeyboardEvent* event)
{
    WebCore::Frame* frame = m_webview->m_page->mainFrame(); // ### FIX: this should use the active frame, not necessarily the main frame
    if (!frame)
        return;

    const WebCore::PlatformKeyboardEvent* kevent = event->keyEvent();
    if (!kevent || kevent->type() == WebCore::PlatformKeyboardEvent::KeyUp || kevent->type() == WebCore::PlatformKeyboardEvent::KeyDown)
        return;

    // ### FIXME not sure yet how to deal with raw events here
    if (kevent->type() == WebCore::PlatformKeyboardEvent::RawKeyDown)
        return;

    WebCore::Node* start = frame->selection()->start().containerNode();
    if (!start)
        return;

    if (!start->isContentEditable())
        return;

    frame->editor()->insertText(kevent->text(), event);
    event->setDefaultHandled();
}

bool EditorClientNetflix::shouldInsertText(const WTF::String &, WebCore::Range *, WebCore::EditorInsertAction)
{
    return true; // ### FIXME should this always return true?
}

void EditorClientNetflix::handleInputMethodKeydown(WebCore::KeyboardEvent*)
{
}

void EditorClientNetflix::setInputMethodState(bool)
{
}

}
