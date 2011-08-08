
#include "config.h"

#include "EditorClientNetflix.h"
#include "EventNames.h"
#include "Frame.h"
#include "KeyboardEvent.h"
#include "Node.h"
#include "Page.h"
#include "PlatformKeyboardEvent.h"
#include "WebViewNetflix.h"
#include "NotImplemented.h"

namespace WebKit {

EditorClientNetflix::EditorClientNetflix(WebViewNetflix* webview)
    : m_webview(webview), m_editing(false)
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

static const unsigned CtrlKey = 1 << 0;
static const unsigned AltKey = 1 << 1;
static const unsigned ShiftKey = 1 << 2;

struct KeyDownEntry {
    unsigned virtualKey;
    unsigned modifiers;
    const char* editorCommand;
};

static const KeyDownEntry keyDownEntries[] = {
    // Ones that do not have an associated QAction:
    { VK_DELETE, 0,                  "DeleteForward"                     },
    { VK_BACK,   ShiftKey,           "DeleteBackward"                    },
    { VK_BACK,   0,                  "DeleteBackward"                    },
    // Ones that need special handling for caret browsing:
    { VK_PRIOR,  0,                  "MovePageUp"                        },
    { VK_PRIOR,  ShiftKey,           "MovePageUpAndModifySelection"      },
    { VK_NEXT,   0,                  "MovePageDown"                      },
    { VK_NEXT,   ShiftKey,           "MovePageDownAndModifySelection"    },
    // Ones that need special handling for spatial navigation:
    { VK_LEFT,   0,                  "MoveLeft"                          },
    { VK_RIGHT,  0,                  "MoveRight"                         },
    { VK_UP,     0,                  "MoveUp"                            },
    { VK_DOWN,   0,                  "MoveDown"                          },
};

static inline const char* editorCommandForKeyDownEvent(const WebCore::KeyboardEvent* event)
{
    if (event->type() != WebCore::eventNames().keydownEvent)
        return "";

    static HashMap<int, const char*> keyDownCommandsMap;
    if (keyDownCommandsMap.isEmpty()) {

        unsigned numEntries = sizeof(keyDownEntries) / sizeof((keyDownEntries)[0]);
        for (unsigned i = 0; i < numEntries; i++)
            keyDownCommandsMap.set(keyDownEntries[i].modifiers << 16 | keyDownEntries[i].virtualKey, keyDownEntries[i].editorCommand);
    }

    unsigned modifiers = 0;
    if (event->shiftKey())
        modifiers |= ShiftKey;
    if (event->altKey())
        modifiers |= AltKey;
    if (event->ctrlKey())
        modifiers |= CtrlKey;

    int mapKey = modifiers << 16 | event->keyCode();
    return mapKey ? keyDownCommandsMap.get(mapKey) : 0;
}

void EditorClientNetflix::handleKeyboardEvent(WebCore::KeyboardEvent* event)
{
    WebCore::Frame* frame = m_webview->m_page->mainFrame(); // ### FIX: this should use the active frame, not necessarily the main frame
    if (!frame)
        return;

    const WebCore::PlatformKeyboardEvent* kevent = event->keyEvent();
    if (!kevent || kevent->type() == WebCore::PlatformKeyboardEvent::KeyUp)
        return;

    WebCore::Node* start = frame->selection()->start().containerNode();
    if (!start)
        return;

    if (!start->isContentEditable())
        return;

    String commandName = editorCommandForKeyDownEvent(event);
    if (!commandName.isEmpty()) {
        if (frame->editor()->command(commandName).execute()) // Event handled.
            event->setDefaultHandled();
        return;
    }

    // ### FIXME not sure yet how to deal with raw events here
    if (kevent->type() == WebCore::PlatformKeyboardEvent::RawKeyDown)
        return;

    // Only allow keypress events passed this point, and TAB events are handled elsewhere
    if (kevent->type() == WebCore::PlatformKeyboardEvent::KeyDown || kevent->windowsVirtualKeyCode() == VK_TAB)
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
