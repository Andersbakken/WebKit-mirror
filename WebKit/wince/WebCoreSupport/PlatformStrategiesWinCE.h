/*
 * Copyright (C) 2010 Patrick Gansterer <paroga@paroga.com>
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
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PlatformStrategiesWinCE_h
#define PlatformStrategiesWinCE_h

#include "LocalizationStrategy.h"
#include "PlatformStrategies.h"
#include "PluginStrategy.h"
#include "VisitedLinkStrategy.h"

class PlatformStrategiesWinCE : public WebCore::PlatformStrategies, private WebCore::PluginStrategy, private WebCore::LocalizationStrategy, private WebCore::VisitedLinkStrategy {
public:
    static void initialize();

private:
    PlatformStrategiesWinCE();

    // WebCore::PlatformStrategies
    virtual WebCore::PluginStrategy* createPluginStrategy();
    virtual WebCore::LocalizationStrategy* createLocalizationStrategy();
    virtual WebCore::VisitedLinkStrategy* createVisitedLinkStrategy();

    // WebCore::PluginStrategy
    virtual void refreshPlugins();
    virtual void getPluginInfo(const WebCore::Page*, Vector<WebCore::PluginInfo>&);

    // WebCore::LocalizationStrategy    
    virtual WTF::String inputElementAltText();
    virtual WTF::String resetButtonDefaultLabel();
    virtual WTF::String searchableIndexIntroduction();
    virtual WTF::String submitButtonDefaultLabel();
    virtual WTF::String fileButtonChooseFileLabel();
    virtual WTF::String fileButtonNoFileSelectedLabel();
#if ENABLE(CONTEXT_MENUS)
    virtual WTF::String contextMenuItemTagOpenLinkInNewWindow();
    virtual WTF::String contextMenuItemTagDownloadLinkToDisk();
    virtual WTF::String contextMenuItemTagCopyLinkToClipboard();
    virtual WTF::String contextMenuItemTagOpenImageInNewWindow();
    virtual WTF::String contextMenuItemTagDownloadImageToDisk();
    virtual WTF::String contextMenuItemTagCopyImageToClipboard();
    virtual WTF::String contextMenuItemTagOpenFrameInNewWindow();
    virtual WTF::String contextMenuItemTagCopy();
    virtual WTF::String contextMenuItemTagGoBack();
    virtual WTF::String contextMenuItemTagGoForward();
    virtual WTF::String contextMenuItemTagStop();
    virtual WTF::String contextMenuItemTagReload();
    virtual WTF::String contextMenuItemTagCut();
    virtual WTF::String contextMenuItemTagPaste();
    virtual WTF::String contextMenuItemTagNoGuessesFound();
    virtual WTF::String contextMenuItemTagIgnoreSpelling();
    virtual WTF::String contextMenuItemTagLearnSpelling();
    virtual WTF::String contextMenuItemTagSearchWeb();
    virtual WTF::String contextMenuItemTagLookUpInDictionary();
    virtual WTF::String contextMenuItemTagOpenLink();
    virtual WTF::String contextMenuItemTagIgnoreGrammar();
    virtual WTF::String contextMenuItemTagSpellingMenu();
    virtual WTF::String contextMenuItemTagShowSpellingPanel(bool show);
    virtual WTF::String contextMenuItemTagCheckSpelling();
    virtual WTF::String contextMenuItemTagCheckSpellingWhileTyping();
    virtual WTF::String contextMenuItemTagCheckGrammarWithSpelling();
    virtual WTF::String contextMenuItemTagFontMenu();
    virtual WTF::String contextMenuItemTagBold();
    virtual WTF::String contextMenuItemTagItalic();
    virtual WTF::String contextMenuItemTagUnderline();
    virtual WTF::String contextMenuItemTagOutline();
    virtual WTF::String contextMenuItemTagWritingDirectionMenu();
    virtual WTF::String contextMenuItemTagTextDirectionMenu();
    virtual WTF::String contextMenuItemTagDefaultDirection();
    virtual WTF::String contextMenuItemTagLeftToRight();
    virtual WTF::String contextMenuItemTagRightToLeft();
    virtual WTF::String contextMenuItemTagInspectElement();
    virtual WTF::String contextMenuItemTagOpenVideoInNewWindow();
    virtual WTF::String contextMenuItemTagOpenAudioInNewWindow();
    virtual WTF::String contextMenuItemTagCopyVideoLinkToClipboard();
    virtual WTF::String contextMenuItemTagCopyAudioLinkToClipboard();
    virtual WTF::String contextMenuItemTagToggleMediaControls();
    virtual WTF::String contextMenuItemTagToggleMediaLoop();
    virtual WTF::String contextMenuItemTagEnterVideoFullscreen();
    virtual WTF::String contextMenuItemTagMediaPlay();
    virtual WTF::String contextMenuItemTagMediaPause();
    virtual WTF::String contextMenuItemTagMediaMute();
#endif // ENABLE(CONTEXT_MENUS)
    virtual WTF::String searchMenuNoRecentSearchesText();
    virtual WTF::String searchMenuRecentSearchesText();
    virtual WTF::String searchMenuClearRecentSearchesText();
    virtual WTF::String AXWebAreaText();
    virtual WTF::String AXLinkText();
    virtual WTF::String AXListMarkerText();
    virtual WTF::String AXImageMapText();
    virtual WTF::String AXHeadingText();
    virtual WTF::String AXDefinitionListTermText();
    virtual WTF::String AXDefinitionListDefinitionText();
    virtual WTF::String AXButtonActionVerb();
    virtual WTF::String AXRadioButtonActionVerb();
    virtual WTF::String AXTextFieldActionVerb();
    virtual WTF::String AXCheckedCheckBoxActionVerb();
    virtual WTF::String AXUncheckedCheckBoxActionVerb();
    virtual WTF::String AXMenuListActionVerb();
    virtual WTF::String AXMenuListPopupActionVerb();
    virtual WTF::String AXLinkActionVerb();
    virtual WTF::String missingPluginText();
    virtual WTF::String crashedPluginText();
    virtual WTF::String multipleFileUploadText(unsigned numberOfFiles);
    virtual WTF::String unknownFileSizeText();
    virtual WTF::String uploadFileText();
    virtual WTF::String allFilesText();
    virtual WTF::String imageTitle(const WTF::String& filename, const WebCore::IntSize&);
    virtual WTF::String mediaElementLoadingStateText();
    virtual WTF::String mediaElementLiveBroadcastStateText();
    virtual WTF::String localizedMediaControlElementString(const WTF::String&);
    virtual WTF::String localizedMediaControlElementHelpText(const WTF::String&);
    virtual WTF::String localizedMediaTimeDescription(float);
    virtual WTF::String validationMessageValueMissingText();
    virtual WTF::String validationMessageTypeMismatchText();
    virtual WTF::String validationMessagePatternMismatchText();
    virtual WTF::String validationMessageTooLongText();
    virtual WTF::String validationMessageRangeUnderflowText();
    virtual WTF::String validationMessageRangeOverflowText();
    virtual WTF::String validationMessageStepMismatchText();

    // WebCore::VisitedLinkStrategy
    virtual bool isLinkVisited(WebCore::Page*, WebCore::LinkHash);
    virtual void addVisitedLink(WebCore::Page*, WebCore::LinkHash);
};

#endif // PlatformStrategiesWinCE_h
