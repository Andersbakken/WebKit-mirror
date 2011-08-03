
#include "config.h"

#include "EditorClientNetflix.h"
#include "NotImplemented.h"

namespace WebKit {

EditorClientNetflix::EditorClientNetflix()
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
}

void EditorClientNetflix::handleInputMethodKeydown(WebCore::KeyboardEvent*)
{
}

void EditorClientNetflix::setInputMethodState(bool)
{
}

}
