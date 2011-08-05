#ifndef EditorClientNetflix_h__
#define EditorClientNetflix_h__

#include "EditorClient.h"
#include "TextCheckerClient.h"

#include "text/WTFString.h"

namespace WebKit {

class WebViewNetflix;

class EditorClientNetflix : public WebCore::EditorClient
{
public:
    EditorClientNetflix(WebViewNetflix* webview);
    virtual ~EditorClientNetflix();

    virtual void pageDestroyed();
    
    virtual bool shouldDeleteRange(WebCore::Range*) { breakpoint(); return false; }
    virtual bool shouldShowDeleteInterface(WebCore::HTMLElement*) { breakpoint(); return false; }
    virtual bool smartInsertDeleteEnabled() { breakpoint(); return false; }
    virtual bool isSelectTrailingWhitespaceEnabled() { breakpoint(); return false; }
    virtual bool isContinuousSpellCheckingEnabled() { breakpoint(); return false; }
    virtual void toggleContinuousSpellChecking() { breakpoint(); }
    virtual bool isGrammarCheckingEnabled() { breakpoint(); return false; }
    virtual void toggleGrammarChecking() { breakpoint(); }
    virtual int spellCheckerDocumentTag() { breakpoint(); return -1; }

    virtual bool isEditable() { return false; }

    virtual bool shouldBeginEditing(WebCore::Range*) { breakpoint(); return false; }
    virtual bool shouldEndEditing(WebCore::Range*) { breakpoint(); return false; }
    virtual bool shouldInsertNode(WebCore::Node*, WebCore::Range*, WebCore::EditorInsertAction) { breakpoint(); return false; }
    virtual bool shouldInsertText(const WTF::String&, WebCore::Range*, WebCore::EditorInsertAction);
    virtual bool shouldChangeSelectedRange(WebCore::Range*, WebCore::Range*, WebCore::EAffinity, bool) { breakpoint(); return true; }

    virtual bool shouldApplyStyle(WebCore::CSSStyleDeclaration*, WebCore::Range*) { breakpoint(); return false; }
    virtual bool shouldMoveRangeAfterDelete(WebCore::Range*, WebCore::Range*) { breakpoint(); return false; }

    virtual void didBeginEditing() { breakpoint(); }
    virtual void respondToChangedContents() { breakpoint(); }
    virtual void respondToChangedSelection() { breakpoint(); }
    virtual void didEndEditing() { breakpoint(); }
    virtual void didWriteSelectionToPasteboard() { breakpoint(); }
    virtual void didSetSelectionTypesForPasteboard() { breakpoint(); }

    virtual void registerCommandForUndo(PassRefPtr<WebCore::EditCommand>) { breakpoint(); }
    virtual void registerCommandForRedo(PassRefPtr<WebCore::EditCommand>) { breakpoint(); }
    virtual void clearUndoRedoOperations();

    virtual bool canCopyCut(WebCore::Frame*, bool) const { breakpoint(); return false; }
    virtual bool canPaste(WebCore::Frame*, bool) const { breakpoint(); return false; }
    virtual bool canUndo() const { breakpoint(); return false; }
    virtual bool canRedo() const { breakpoint(); return false; }

    virtual WebCore::TextCheckerClient* textChecker() { breakpoint(); return 0; }

    virtual void undo() { breakpoint(); }
    virtual void redo() { breakpoint(); }

    virtual void handleKeyboardEvent(WebCore::KeyboardEvent*);
    virtual void handleInputMethodKeydown(WebCore::KeyboardEvent*);

    virtual void textFieldDidBeginEditing(WebCore::Element*) { breakpoint(); }
    virtual void textFieldDidEndEditing(WebCore::Element*) { breakpoint(); }
    virtual void textDidChangeInTextField(WebCore::Element*) { breakpoint(); }
    virtual bool doTextFieldCommandFromEvent(WebCore::Element*, WebCore::KeyboardEvent*) { breakpoint(); return false; }
    virtual void textWillBeDeletedInTextField(WebCore::Element*) { breakpoint(); }
    virtual void textDidChangeInTextArea(WebCore::Element*) { breakpoint(); }

    virtual void ignoreWordInSpellDocument(const WTF::String&) { breakpoint(); }
    virtual void learnWord(const WTF::String&) { breakpoint(); }
    virtual void checkSpellingOfString(const UChar*, int, int*, int*) { breakpoint(); }
    virtual WTF::String getAutoCorrectSuggestionForMisspelledWord(const WTF::String&) { breakpoint(); return WTF::String(); }
    virtual void checkGrammarOfString(const UChar*, int, Vector<WebCore::GrammarDetail>&, int*, int*) { breakpoint(); }
    virtual void updateSpellingUIWithGrammarString(const WTF::String&, const WebCore::GrammarDetail&) { breakpoint(); }
    virtual void updateSpellingUIWithMisspelledWord(const WTF::String&) { breakpoint(); }
    virtual void showSpellingUI(bool) { breakpoint(); }
    virtual bool spellingUIIsShowing() { breakpoint(); return false; }
    virtual void getGuessesForWord(const WTF::String&, Vector<WTF::String>&) { breakpoint(); }
    virtual void willSetInputMethodState() { breakpoint(); }
    virtual void setInputMethodState(bool);

private:
    void breakpoint() const { }

private:
    WebViewNetflix* m_webview;
};

}

#endif // EditorClientNetflix_h__
