/*
 * Copyright (C) 2006 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2006 Zack Rusin <zack@kde.org>
 * Copyright (C) 2006 Apple Computer, Inc.
 * Copyright (C) 2007 Ryan Leavengood <leavengood@gmail.com>
 *
 * All rights reserved.
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

#ifndef EditorClientHaiku_H
#define EditorClientHaiku_H

#include "EditorClient.h"
#include "RefCounted.h"
#include "Page.h"

#include <wtf/Forward.h>


namespace WebCore {

    class EditorClientHaiku : public EditorClient {
    public:
        EditorClientHaiku();
        void setPage( Page* page );

        virtual void pageDestroyed();

        virtual bool shouldDeleteRange(Range*);
        virtual bool shouldShowDeleteInterface(HTMLElement*);
        virtual bool smartInsertDeleteEnabled();
        virtual bool isSelectTrailingWhitespaceEnabled();
        virtual bool isContinuousSpellCheckingEnabled();
        virtual void toggleContinuousSpellChecking();
        virtual bool isGrammarCheckingEnabled();
        virtual void toggleGrammarChecking();
        virtual int spellCheckerDocumentTag();

        virtual bool isEditable();

        virtual bool shouldBeginEditing(Range*);
        virtual bool shouldEndEditing(Range*);
        virtual bool shouldInsertNode(Node*, Range*, EditorInsertAction);
        virtual bool shouldInsertText(const String&, Range*, EditorInsertAction);
        virtual bool shouldChangeSelectedRange(Range* fromRange, Range* toRange,
                                               EAffinity, bool stillSelecting);

        virtual bool shouldApplyStyle(CSSStyleDeclaration*, Range*);
        virtual bool shouldMoveRangeAfterDelete(Range*, Range*);

        virtual void didBeginEditing();
        virtual void respondToChangedContents();
        virtual void respondToChangedSelection();
        virtual void didEndEditing();
        virtual void didWriteSelectionToPasteboard();
        virtual void didSetSelectionTypesForPasteboard();

        virtual void registerCommandForUndo(PassRefPtr<EditCommand>);
        virtual void registerCommandForRedo(PassRefPtr<EditCommand>);
        virtual void clearUndoRedoOperations();

        virtual bool canUndo() const;
        virtual bool canRedo() const;

        virtual void undo();
        virtual void redo();

        virtual void handleKeyboardEvent(KeyboardEvent*);
        virtual void handleInputMethodKeydown(KeyboardEvent*);

        virtual void textFieldDidBeginEditing(Element*);
        virtual void textFieldDidEndEditing(Element*);
        virtual void textDidChangeInTextField(Element*);
        virtual bool doTextFieldCommandFromEvent(Element*, KeyboardEvent*);
        virtual void textWillBeDeletedInTextField(Element*);
        virtual void textDidChangeInTextArea(Element*);

        virtual void ignoreWordInSpellDocument(const String&);
        virtual void learnWord(const String&);
        virtual void checkSpellingOfString(const UChar*, int length, int* misspellingLocation,
                                           int* misspellingLength);
        virtual String getAutoCorrectSuggestionForMisspelledWord(const String& misspelledWord);
        virtual void checkGrammarOfString(const UChar*, int length, Vector<GrammarDetail>&,
                                          int* badGrammarLocation, int* badGrammarLength);
        virtual void updateSpellingUIWithGrammarString(const String&, const GrammarDetail&);
        virtual void updateSpellingUIWithMisspelledWord(const String&);
        virtual void showSpellingUI(bool show);
        virtual bool spellingUIIsShowing();
        virtual void getGuessesForWord(const String& word, const String& context, Vector<String>& guesses);
        virtual void willSetInputMethodState();
        virtual void setInputMethodState(bool enabled);
        virtual void requestCheckingOfString(SpellChecker*, int, const String&) {}

        bool isEditing() const;

    private:
        Page* m_page;
        bool m_editing;
        bool m_inUndoRedo; // our undo stack works differently - don't re-enter!
    };

} // namespace WebCore

#endif // EditorClientHaiku_h
