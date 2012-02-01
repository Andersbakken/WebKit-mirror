/*
 * Copyright (C) 2007 Apple Inc.
 * Copyright (C) 2007 Alp Toker <alp@atoker.com>
 * Copyright (C) 2008 Collabora Ltd.
 * Copyright (C) 2008 INdT - Instituto Nokia de Tecnologia
 * Copyright (C) 2009-2010 ProFUSION embedded systems
 * Copyright (C) 2009-2011 Samsung Electronics
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
 *
 */

#include "config.h"
#include "RenderThemeNetflix.h"

#include "CSSValueKeywords.h"
#include "FileSystem.h"
#include "Frame.h"
#include "FrameView.h"
#include "GraphicsContext.h"
#include "HTMLInputElement.h"
#include "NotImplemented.h"
#include "Page.h"
#include "PaintInfo.h"
#include "PlatformContextCairo.h"
#include "RenderBox.h"
#include "RenderObject.h"
#include "RenderProgress.h"
#include "RenderSlider.h"
#include "UserAgentStyleSheets.h"

#include <wtf/text/CString.h>
#include <wtf/text/WTFString.h>

#if ENABLE(VIDEO)
#include "HTMLMediaElement.h"
#include "HTMLNames.h"
#include "TimeRanges.h"
#endif

namespace WebCore {
#if ENABLE(VIDEO)
using namespace HTMLNames;
#endif

// TODO: change from object count to ecore_evas size (bytes)
// TODO: as objects are webpage/user defined and they can be very large.
#define RENDER_THEME_NETFLIX_PART_CACHE_MAX 32

// Initialize default font size.
float RenderThemeNetflix::defaultFontSize = 16.0f;

// Constants for progress tag animation.
// These values have been copied from RenderThemeGtk.cpp
static const int progressAnimationFrames = 10;
static const double progressAnimationInterval = 0.125;

static const int sliderThumbWidth = 12;
static const int sliderThumbHeight = 12;
#if ENABLE(VIDEO)
static const int mediaSliderHeight = 14;
static const int mediaSliderThumbWidth = 12;
static const int mediaSliderThumbHeight = 12;
#endif

PassRefPtr<RenderTheme> RenderThemeNetflix::create(Page* page)
{
    return adoptRef(new RenderThemeNetflix(page));
}

PassRefPtr<RenderTheme> RenderTheme::themeForPage(Page* page)
{
    if (page)
        return RenderThemeNetflix::create(page);

    static RenderTheme* fallback = RenderThemeNetflix::create(0).leakRef();
    return fallback;
}

void RenderThemeNetflix::themeChanged()
{
}

RenderThemeNetflix::RenderThemeNetflix(Page* page)
    : RenderTheme()
    , m_page(page)
    , m_activeSelectionBackgroundColor(0, 0, 255)
    , m_activeSelectionForegroundColor(Color::white)
    , m_inactiveSelectionBackgroundColor(0, 0, 128)
    , m_inactiveSelectionForegroundColor(200, 200, 200)
    , m_focusRingColor(32, 32, 224, 224)
    , m_buttonTextBackgroundColor(0, 0, 0, 0)
    , m_buttonTextForegroundColor(Color::black)
    , m_comboTextBackgroundColor(0, 0, 0, 0)
    , m_comboTextForegroundColor(Color::black)
    , m_entryTextBackgroundColor(0, 0, 0, 0)
    , m_entryTextForegroundColor(Color::black)
    , m_searchTextBackgroundColor(0, 0, 0, 0)
    , m_searchTextForegroundColor(Color::black)
    , m_sliderThumbColor(Color::darkGray)
#if ENABLE(VIDEO)
    , m_mediaPanelColor(220, 220, 195) // light tannish color.
    , m_mediaSliderColor(Color::white)
#endif
{
    if (page && page->mainFrame() && page->mainFrame()->view())
        themeChanged();
}

RenderThemeNetflix::~RenderThemeNetflix()
{
}

void RenderThemeNetflix::setActiveSelectionColor(int foreR, int foreG, int foreB, int foreA, int backR, int backG, int backB, int backA)
{
    m_activeSelectionForegroundColor = Color(foreR, foreG, foreB, foreA);
    m_activeSelectionBackgroundColor = Color(backR, backG, backB, backA);
    platformColorsDidChange();
}

void RenderThemeNetflix::setInactiveSelectionColor(int foreR, int foreG, int foreB, int foreA, int backR, int backG, int backB, int backA)
{
    m_inactiveSelectionForegroundColor = Color(foreR, foreG, foreB, foreA);
    m_inactiveSelectionBackgroundColor = Color(backR, backG, backB, backA);
    platformColorsDidChange();
}

void RenderThemeNetflix::setFocusRingColor(int r, int g, int b, int a)
{
    m_focusRingColor = Color(r, g, b, a);
    // webkit just use platformFocusRingColor() for default theme (without page)
    // this is ugly, but no other way to do it unless we change
    // it to use page themes as much as possible.
    RenderTheme::setCustomFocusRingColor(m_focusRingColor);
    platformColorsDidChange();
}

void RenderThemeNetflix::setButtonTextColor(int foreR, int foreG, int foreB, int foreA, int backR, int backG, int backB, int backA)
{
    m_buttonTextForegroundColor = Color(foreR, foreG, foreB, foreA);
    m_buttonTextBackgroundColor = Color(backR, backG, backB, backA);
    platformColorsDidChange();
}

void RenderThemeNetflix::setComboTextColor(int foreR, int foreG, int foreB, int foreA, int backR, int backG, int backB, int backA)
{
    m_comboTextForegroundColor = Color(foreR, foreG, foreB, foreA);
    m_comboTextBackgroundColor = Color(backR, backG, backB, backA);
    platformColorsDidChange();
}

void RenderThemeNetflix::setEntryTextColor(int foreR, int foreG, int foreB, int foreA, int backR, int backG, int backB, int backA)
{
    m_entryTextForegroundColor = Color(foreR, foreG, foreB, foreA);
    m_entryTextBackgroundColor = Color(backR, backG, backB, backA);
    platformColorsDidChange();
}

void RenderThemeNetflix::setSearchTextColor(int foreR, int foreG, int foreB, int foreA, int backR, int backG, int backB, int backA)
{
    m_searchTextForegroundColor = Color(foreR, foreG, foreB, foreA);
    m_searchTextBackgroundColor = Color(backR, backG, backB, backA);
    platformColorsDidChange();
}

static bool supportsFocus(ControlPart appearance)
{
    switch (appearance) {
    case PushButtonPart:
    case ButtonPart:
    case TextFieldPart:
    case TextAreaPart:
    case SearchFieldPart:
    case MenulistPart:
    case RadioPart:
    case CheckboxPart:
    case SliderVerticalPart:
    case SliderHorizontalPart:
        return true;
    default:
        return false;
    }
}

bool RenderThemeNetflix::supportsFocusRing(const RenderStyle* style) const
{
    return supportsFocus(style->appearance());
}

bool RenderThemeNetflix::controlSupportsTints(const RenderObject* object) const
{
    return isEnabled(object);
}

int RenderThemeNetflix::baselinePosition(const RenderObject* object) const
{
    return 0;
}

bool RenderThemeNetflix::paintSliderTrack(RenderObject* object, const PaintInfo& info, const IntRect& rect)
{
    notImplemented();
    return false;
}

void RenderThemeNetflix::adjustSliderTrackStyle(CSSStyleSelector* selector, RenderStyle* style, Element* element) const
{
}

void RenderThemeNetflix::adjustSliderThumbStyle(CSSStyleSelector* selector, RenderStyle* style, Element* element) const
{
}

void RenderThemeNetflix::adjustSliderThumbSize(RenderStyle* style) const
{
}

bool RenderThemeNetflix::paintSliderThumb(RenderObject* object, const PaintInfo& info, const IntRect& rect)
{
    notImplemented();
    return false;
}

void RenderThemeNetflix::adjustCheckboxStyle(CSSStyleSelector* selector, RenderStyle* style, Element* element) const
{
}

bool RenderThemeNetflix::paintCheckbox(RenderObject* object, const PaintInfo& info, const IntRect& rect)
{
    notImplemented();
    return false;
}

void RenderThemeNetflix::adjustRadioStyle(CSSStyleSelector* selector, RenderStyle* style, Element* element) const
{
}

bool RenderThemeNetflix::paintRadio(RenderObject* object, const PaintInfo& info, const IntRect& rect)
{
    notImplemented();
    return false;
}

void RenderThemeNetflix::adjustButtonStyle(CSSStyleSelector* selector, RenderStyle* style, Element* element) const
{
}

bool RenderThemeNetflix::paintButton(RenderObject* object, const PaintInfo& info, const IntRect& rect)
{
    notImplemented();
    return false;
}

void RenderThemeNetflix::adjustMenuListStyle(CSSStyleSelector* selector, RenderStyle* style, Element* element) const
{
}

bool RenderThemeNetflix::paintMenuList(RenderObject* object, const PaintInfo& info, const IntRect& rect)
{
    notImplemented();
    return false;
}

void RenderThemeNetflix::adjustMenuListButtonStyle(CSSStyleSelector* selector, RenderStyle* style, Element* element) const
{
}

bool RenderThemeNetflix::paintMenuListButton(RenderObject* object, const PaintInfo& info, const IntRect& rect)
{
    notImplemented();
    return false;
}

void RenderThemeNetflix::adjustTextFieldStyle(CSSStyleSelector* selector, RenderStyle* style, Element* element) const
{
}

bool RenderThemeNetflix::paintTextField(RenderObject* object, const PaintInfo& info, const IntRect& rect)
{
    notImplemented();
    return false;
}

void RenderThemeNetflix::adjustTextAreaStyle(CSSStyleSelector* selector, RenderStyle* style, Element* element) const
{
}

bool RenderThemeNetflix::paintTextArea(RenderObject* object, const PaintInfo& info, const IntRect& rect)
{
    notImplemented();
    return false;
}

void RenderThemeNetflix::adjustSearchFieldDecorationStyle(CSSStyleSelector* selector, RenderStyle* style, Element* element) const
{
}

bool RenderThemeNetflix::paintSearchFieldDecoration(RenderObject* object, const PaintInfo& info, const IntRect& rect)
{
    notImplemented();
    return false;
}

void RenderThemeNetflix::adjustSearchFieldResultsButtonStyle(CSSStyleSelector* selector, RenderStyle* style, Element* element) const
{
}

bool RenderThemeNetflix::paintSearchFieldResultsButton(RenderObject* object, const PaintInfo& info, const IntRect& rect)
{
    notImplemented();
    return false;
}

void RenderThemeNetflix::adjustSearchFieldResultsDecorationStyle(CSSStyleSelector* selector, RenderStyle* style, Element* element) const
{
}

bool RenderThemeNetflix::paintSearchFieldResultsDecoration(RenderObject* object, const PaintInfo& info, const IntRect& rect)
{
    notImplemented();
    return false;
}

void RenderThemeNetflix::adjustSearchFieldCancelButtonStyle(CSSStyleSelector* selector, RenderStyle* style, Element* element) const
{
}

bool RenderThemeNetflix::paintSearchFieldCancelButton(RenderObject* object, const PaintInfo& info, const IntRect& rect)
{
    notImplemented();
    return false;
}

void RenderThemeNetflix::adjustSearchFieldStyle(CSSStyleSelector* selector, RenderStyle* style, Element* element) const
{
}

bool RenderThemeNetflix::paintSearchField(RenderObject* object, const PaintInfo& info, const IntRect& rect)
{
    notImplemented();
    return false;
}

void RenderThemeNetflix::setDefaultFontSize(int size)
{
    defaultFontSize = size;
}

void RenderThemeNetflix::systemFont(int propId, FontDescription& fontDescription) const
{
    // It was called by RenderEmbeddedObject::paintReplaced to render alternative string.
    // To avoid cairo_error while rendering, fontDescription should be passed.
    DEFINE_STATIC_LOCAL(String, fontFace, ("Sans"));
    float fontSize = defaultFontSize;

    fontDescription.firstFamily().setFamily(fontFace);
    fontDescription.setSpecifiedSize(fontSize);
    fontDescription.setIsAbsoluteSize(true);
    fontDescription.setGenericFamily(FontDescription::NoFamily);
    fontDescription.setWeight(FontWeightNormal);
    fontDescription.setItalic(false);
}

#if ENABLE(PROGRESS_TAG)
void RenderThemeNetflix::adjustProgressBarStyle(CSSStyleSelector* selector, RenderStyle* style, Element* element) const
{
}

double RenderThemeNetflix::animationRepeatIntervalForProgressBar(RenderProgress*) const
{
    return progressAnimationInterval;
}

double RenderThemeNetflix::animationDurationForProgressBar(RenderProgress*) const
{
    return progressAnimationInterval * progressAnimationFrames * 2; // "2" for back and forth;
}

bool RenderThemeNetflix::paintProgressBar(RenderObject* object, const PaintInfo& info, const IntRect& rect)
{
    notImplemented();
    return false;
}
#endif

#if ENABLE(VIDEO)
bool RenderThemeNetflix::emitMediaButtonSignal(FormType formType, MediaControlElementType mediaElementType, const IntRect& rect)
{
    notImplemented();
    return false;
}

String RenderThemeNetflix::extraMediaControlsStyleSheet()
{
    return String(mediaControlsNetflixUserAgentStyleSheet, sizeof(mediaControlsNetflixUserAgentStyleSheet));
}

String RenderThemeNetflix::formatMediaControlsCurrentTime(float currentTime, float duration) const
{
    return formatMediaControlsTime(currentTime) + " / " + formatMediaControlsTime(duration);
}

bool RenderThemeNetflix::paintMediaFullscreenButton(RenderObject* object, const PaintInfo& info, const IntRect& rect)
{
    notImplemented();
    return false;
}

bool RenderThemeNetflix::paintMediaMuteButton(RenderObject* object, const PaintInfo& info, const IntRect& rect)
{
    notImplemented();
    return false;
}

bool RenderThemeNetflix::paintMediaPlayButton(RenderObject* object, const PaintInfo& info, const IntRect& rect)
{
    notImplemented();
    return false;
}

bool RenderThemeNetflix::paintMediaSeekBackButton(RenderObject* object, const PaintInfo& info, const IntRect& rect)
{
    notImplemented();
    return false;
}

bool RenderThemeNetflix::paintMediaSeekForwardButton(RenderObject* object, const PaintInfo& info, const IntRect& rect)
{
    notImplemented();
    return false;
}

bool RenderThemeNetflix::paintMediaSliderTrack(RenderObject* object, const PaintInfo& info, const IntRect& rect)
{
    notImplemented();
    return false;
}

bool RenderThemeNetflix::paintMediaSliderThumb(RenderObject* object, const PaintInfo& info, const IntRect& rect)
{
    notImplemented();
    return false;
}

bool RenderThemeNetflix::paintMediaVolumeSliderContainer(RenderObject*, const PaintInfo& info, const IntRect& rect)
{
    notImplemented();
    return false;
}

bool RenderThemeNetflix::paintMediaVolumeSliderTrack(RenderObject* object, const PaintInfo& info, const IntRect& rect)
{
    notImplemented();
    return false;
}

bool RenderThemeNetflix::paintMediaVolumeSliderThumb(RenderObject* object, const PaintInfo& info, const IntRect& rect)
{
    notImplemented();
    return false;
}

bool RenderThemeNetflix::paintMediaCurrentTime(RenderObject* object, const PaintInfo& info, const IntRect& rect)
{
    notImplemented();
    return false;
}
#endif
}
