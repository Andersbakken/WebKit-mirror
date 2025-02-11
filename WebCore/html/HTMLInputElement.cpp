/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010 Apple Inc. All rights reserved.
 *           (C) 2006 Alexey Proskuryakov (ap@nypop.com)
 * Copyright (C) 2007 Samuel Weinig (sam@webkit.org)
 * Copyright (C) 2010 Google Inc. All rights reserved.
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
#include "HTMLInputElement.h"

#include "AXObjectCache.h"
#include "Attribute.h"
#include "BeforeTextInsertedEvent.h"
#include "CSSPropertyNames.h"
#include "ChromeClient.h"
#include "DateComponents.h"
#include "Document.h"
#include "Editor.h"
#include "Event.h"
#include "EventHandler.h"
#include "EventNames.h"
#include "ExceptionCode.h"
#include "File.h"
#include "FileList.h"
#include "FileSystem.h"
#include "FocusController.h"
#include "FormDataList.h"
#include "Frame.h"
#include "HTMLDataListElement.h"
#include "HTMLFormElement.h"
#include "HTMLImageLoader.h"
#include "HTMLNames.h"
#include "HTMLOptionElement.h"
#include "HTMLParserIdioms.h"
#include "InputType.h"
#include "KeyboardEvent.h"
#include "LocalizedStrings.h"
#include "MouseEvent.h"
#include "Page.h"
#include "RenderFileUploadControl.h"
#include "RenderImage.h"
#include "RenderSlider.h"
#include "RenderTextControlSingleLine.h"
#include "RenderTheme.h"
#include "RuntimeEnabledFeatures.h"
#include "ScriptEventListener.h"
#include "Settings.h"
#include "StepRange.h"
#include "TextEvent.h"
#include "WheelEvent.h"
#include <wtf/HashMap.h>
#include <wtf/MathExtras.h>
#include <wtf/StdLibExtras.h>
#include <wtf/text/StringHash.h>

using namespace std;

namespace WebCore {

using namespace HTMLNames;

const int maxSavedResults = 256;

HTMLInputElement::HTMLInputElement(const QualifiedName& tagName, Document* document, HTMLFormElement* form)
    : HTMLTextFormControlElement(tagName, document, form)
    , m_maxResults(-1)
    , m_deprecatedTypeNumber(TEXT)
    , m_checked(false)
    , m_defaultChecked(false)
    , m_useDefaultChecked(true)
    , m_indeterminate(false)
    , m_haveType(false)
    , m_activeSubmit(false)
    , m_autocomplete(Uninitialized)
    , m_autofilled(false)
    , m_inited(false)
    , m_inputType(InputType::createText(this))
{
    ASSERT(hasTagName(inputTag) || hasTagName(isindexTag));
}

PassRefPtr<HTMLInputElement> HTMLInputElement::create(const QualifiedName& tagName, Document* document, HTMLFormElement* form)
{
    return adoptRef(new HTMLInputElement(tagName, document, form));
}

HTMLInputElement::~HTMLInputElement()
{
    if (needsActivationCallback())
        document()->unregisterForDocumentActivationCallbacks(this);

    document()->checkedRadioButtons().removeButton(this);

    // Need to remove this from the form while it is still an HTMLInputElement,
    // so can't wait for the base class's destructor to do it.
    removeFromForm();
}

const AtomicString& HTMLInputElement::formControlName() const
{
    return m_data.name();
}

bool HTMLInputElement::autoComplete() const
{
    if (m_autocomplete != Uninitialized)
        return m_autocomplete == On;
    return HTMLTextFormControlElement::autoComplete();
}

void HTMLInputElement::updateCheckedRadioButtons()
{
    if (attached() && checked())
        checkedRadioButtons().addButton(this);

    if (form()) {
        const Vector<FormAssociatedElement*>& controls = form()->associatedElements();
        for (unsigned i = 0; i < controls.size(); ++i) {
            if (!controls[i]->isFormControlElement())
                continue;
            HTMLFormControlElement* control = static_cast<HTMLFormControlElement*>(controls[i]);
            if (control->name() != name())
                continue;
            if (control->type() != type())
                continue;
            control->setNeedsValidityCheck();
        }
    } else {
        // FIXME: Traversing the document is inefficient.
        for (Node* node = document()->body(); node; node = node->traverseNextNode()) {
            if (!node->isElementNode())
                continue;
            Element* element = static_cast<Element*>(node);
            if (element->formControlName() != name())
                continue;
            if (element->formControlType() != type())
                continue;
            HTMLFormControlElement* control = static_cast<HTMLFormControlElement*>(element);
            if (control->form())
                continue;
            control->setNeedsValidityCheck();
        }
    }
   
    if (renderer() && renderer()->style()->hasAppearance())
        renderer()->theme()->stateChanged(renderer(), CheckedState);
}

bool HTMLInputElement::isValidValue(const String& value) const
{
    // Should not call isValidValue() for the following types because
    // we can't set string values for these types.
    if (deprecatedInputType() == CHECKBOX || deprecatedInputType() == FILE || deprecatedInputType() == RADIO) {
        ASSERT_NOT_REACHED();
        return false;
    }
    return !m_inputType->typeMismatchFor(value)
        && !stepMismatch(value)
        && !rangeUnderflow(value)
        && !rangeOverflow(value)
        && !tooLong(value, IgnoreDirtyFlag)
        && !patternMismatch(value)
        && !valueMissing(value);
}

bool HTMLInputElement::typeMismatch() const
{
    return m_inputType->typeMismatch();
}

bool HTMLInputElement::valueMissing(const String& value) const
{
    if (!isRequiredFormControl() || readOnly() || disabled())
        return false;
    return m_inputType->valueMissing(value);
}

bool HTMLInputElement::patternMismatch(const String& value) const
{
    return m_inputType->patternMismatch(value);
}

bool HTMLInputElement::tooLong(const String& value, NeedsToCheckDirtyFlag check) const
{
    // We use isTextType() instead of supportsMaxLength() because of the
    // 'virtual' overhead.
    if (!isTextType())
        return false;
    int max = maxLength();
    if (max < 0)
        return false;
    if (check == CheckDirtyFlag) {
        // Return false for the default value even if it is longer than maxLength.
        bool userEdited = !m_data.value().isNull();
        if (!userEdited)
            return false;
    }
    return numGraphemeClusters(value) > static_cast<unsigned>(max);
}

bool HTMLInputElement::rangeUnderflow(const String& value) const
{
    return m_inputType->rangeUnderflow(value);
}

bool HTMLInputElement::rangeOverflow(const String& value) const
{
    return m_inputType->rangeOverflow(value);
}

double HTMLInputElement::minimum() const
{
    return m_inputType->minimum();
}

double HTMLInputElement::maximum() const
{
    return m_inputType->maximum();
}

bool HTMLInputElement::stepMismatch(const String& value) const
{
    double step;
    if (!getAllowedValueStep(&step))
        return false;
    return m_inputType->stepMismatch(value, step);
}

String HTMLInputElement::minimumString() const
{
    return m_inputType->serialize(minimum());
}

String HTMLInputElement::maximumString() const
{
    return m_inputType->serialize(maximum());
}

String HTMLInputElement::stepBaseString() const
{
    return m_inputType->serialize(m_inputType->stepBase());
}

String HTMLInputElement::stepString() const
{
    double step;
    if (!getAllowedValueStep(&step)) {
        // stepString() should be called only if stepMismatch() can be true.
        ASSERT_NOT_REACHED();
        return String();
    }
    return serializeForNumberType(step / m_inputType->stepScaleFactor());
}

String HTMLInputElement::typeMismatchText() const
{
    return m_inputType->typeMismatchText();
}

String HTMLInputElement::valueMissingText() const
{
    return m_inputType->valueMissingText();
}

bool HTMLInputElement::getAllowedValueStep(double* step) const
{
    return getAllowedValueStepWithDecimalPlaces(step, 0);
}

bool HTMLInputElement::getAllowedValueStepWithDecimalPlaces(double* step, unsigned* decimalPlaces) const
{
    ASSERT(step);
    double defaultStep = m_inputType->defaultStep();
    double stepScaleFactor = m_inputType->stepScaleFactor();
    if (!isfinite(defaultStep) || !isfinite(stepScaleFactor))
        return false;
    const AtomicString& stepString = getAttribute(stepAttr);
    if (stepString.isEmpty()) {
        *step = defaultStep * stepScaleFactor;
        if (decimalPlaces)
            *decimalPlaces = 0;
        return true;
    }
    if (equalIgnoringCase(stepString, "any"))
        return false;
    double parsed;
    if (!decimalPlaces) {
        if (!parseToDoubleForNumberType(stepString, &parsed) || parsed <= 0.0) {
            *step = defaultStep * stepScaleFactor;
            return true;
        }
    } else {
        if (!parseToDoubleForNumberTypeWithDecimalPlaces(stepString, &parsed, decimalPlaces) || parsed <= 0.0) {
            *step = defaultStep * stepScaleFactor;
            *decimalPlaces = 0;
            return true;
        }
    }
    // For date, month, week, the parsed value should be an integer for some types.
    if (m_inputType->parsedStepValueShouldBeInteger())
        parsed = max(round(parsed), 1.0);
    double result = parsed * stepScaleFactor;
    // For datetime, datetime-local, time, the result should be an integer.
    if (m_inputType->scaledStepValeuShouldBeInteger())
        result = max(round(result), 1.0);
    ASSERT(result > 0);
    *step = result;
    return true;
}

void HTMLInputElement::applyStep(double count, ExceptionCode& ec)
{
    double step;
    unsigned stepDecimalPlaces, currentDecimalPlaces;
    if (!getAllowedValueStepWithDecimalPlaces(&step, &stepDecimalPlaces)) {
        ec = INVALID_STATE_ERR;
        return;
    }
    const double nan = numeric_limits<double>::quiet_NaN();
    double current = m_inputType->parseToDoubleWithDecimalPlaces(value(), nan, &currentDecimalPlaces);
    if (!isfinite(current)) {
        ec = INVALID_STATE_ERR;
        return;
    }
    double newValue = current + step * count;
    if (isinf(newValue)) {
        ec = INVALID_STATE_ERR;
        return;
    }
    double acceptableError = m_inputType->acceptableError(step);
    if (newValue - m_inputType->minimum() < -acceptableError) {
        ec = INVALID_STATE_ERR;
        return;
    }
    if (newValue < m_inputType->minimum())
        newValue = m_inputType->minimum();
    unsigned baseDecimalPlaces;
    double base = m_inputType->stepBaseWithDecimalPlaces(&baseDecimalPlaces);
    baseDecimalPlaces = min(baseDecimalPlaces, 16u);
    if (newValue < pow(10.0, 21.0)) {
      if (stepMismatch(value())) {
            double scale = pow(10.0, static_cast<double>(max(stepDecimalPlaces, currentDecimalPlaces)));
            newValue = round(newValue * scale) / scale;
        } else {
            double scale = pow(10.0, static_cast<double>(max(stepDecimalPlaces, baseDecimalPlaces)));
            newValue = round((base + round((newValue - base) / step) * step) * scale) / scale;
        }
    }
    if (newValue - m_inputType->maximum() > acceptableError) {
        ec = INVALID_STATE_ERR;
        return;
    }
    if (newValue > m_inputType->maximum())
        newValue = m_inputType->maximum();
    setValueAsNumber(newValue, ec);
    
    if (AXObjectCache::accessibilityEnabled())
         document()->axObjectCache()->postNotification(renderer(), AXObjectCache::AXValueChanged, true);
}

void HTMLInputElement::stepUp(int n, ExceptionCode& ec)
{
    applyStep(n, ec);
}

void HTMLInputElement::stepDown(int n, ExceptionCode& ec)
{
    applyStep(-n, ec);
}

bool HTMLInputElement::isKeyboardFocusable(KeyboardEvent* event) const
{
    // If text fields can be focused, then they should always be keyboard focusable
    if (isTextField())
        return HTMLFormControlElementWithState::isFocusable();
        
    // If the base class says we can't be focused, then we can stop now.
    if (!HTMLFormControlElementWithState::isKeyboardFocusable(event))
        return false;

    if (deprecatedInputType() == RADIO) {
        // When using Spatial Navigation, every radio button should be focusable.
        if (isSpatialNavigationEnabled(document()->frame()))
            return true;

        // Never allow keyboard tabbing to leave you in the same radio group.  Always
        // skip any other elements in the group.
        Node* currentFocusedNode = document()->focusedNode();
        if (currentFocusedNode && currentFocusedNode->hasTagName(inputTag)) {
            HTMLInputElement* focusedInput = static_cast<HTMLInputElement*>(currentFocusedNode);
            if (focusedInput->deprecatedInputType() == RADIO && focusedInput->form() == form() && focusedInput->name() == name())
                return false;
        }
        
        // Allow keyboard focus if we're checked or if nothing in the group is checked.
        return checked() || !checkedRadioButtons().checkedButtonForGroup(name());
    }
    
    return true;
}

bool HTMLInputElement::isMouseFocusable() const
{
    if (isTextField())
        return HTMLFormControlElementWithState::isFocusable();
    return HTMLFormControlElementWithState::isMouseFocusable();
}

void HTMLInputElement::updateFocusAppearance(bool restorePreviousSelection)
{        
    if (isTextField())
        InputElement::updateFocusAppearance(m_data, this, this, restorePreviousSelection);
    else
        HTMLFormControlElementWithState::updateFocusAppearance(restorePreviousSelection);
}

void HTMLInputElement::aboutToUnload()
{
    InputElement::aboutToUnload(this, this);
}

bool HTMLInputElement::shouldUseInputMethod() const
{
    // The reason IME's are disabled for the password field is because IMEs 
    // can access the underlying password and display it in clear text --
    // e.g. you can use it to access the stored password for any site 
    // with only trivial effort.
    return isTextField() && deprecatedInputType() != PASSWORD;
}

void HTMLInputElement::handleFocusEvent()
{
    InputElement::dispatchFocusEvent(this, this);
}

void HTMLInputElement::handleBlurEvent()
{
    if (deprecatedInputType() == NUMBER) {
        // Reset the renderer value, which might be unmatched with the element value.
        setFormControlValueMatchesRenderer(false);
        // We need to reset the renderer value explicitly because an unacceptable
        // renderer value should be purged before style calculation.
        if (renderer())
            renderer()->updateFromElement();
    }
    InputElement::dispatchBlurEvent(this, this);
}

void HTMLInputElement::setType(const String& t)
{
    // FIXME: This should just call setAttribute. No reason to handle the empty string specially.
    // We should write a test case to show that setting to the empty string does not remove the
    // attribute in other browsers and then fix this. Note that setting to null *does* remove
    // the attribute and setAttribute implements that.
    if (t.isEmpty()) {
        int exccode;
        removeAttribute(typeAttr, exccode);
    } else
        setAttribute(typeAttr, t);
}

PassOwnPtr<HTMLInputElement::InputTypeMap> HTMLInputElement::createTypeMap()
{
    OwnPtr<InputTypeMap> map = adoptPtr(new InputTypeMap);
    map->add("button", BUTTON);
    map->add("checkbox", CHECKBOX);
    map->add("color", COLOR);
    map->add("date", DATE);
    map->add("datetime", DATETIME);
    map->add("datetime-local", DATETIMELOCAL);
    map->add("email", EMAIL);
    map->add("file", FILE);
    map->add("hidden", HIDDEN);
    map->add("image", IMAGE);
    map->add("khtml_isindex", ISINDEX);
    map->add("month", MONTH);
    map->add("number", NUMBER);
    map->add("password", PASSWORD);
    map->add("radio", RADIO);
    map->add("range", RANGE);
    map->add("reset", RESET);
    map->add("search", SEARCH);
    map->add("submit", SUBMIT);
    map->add("tel", TELEPHONE);
    map->add("time", TIME);
    map->add("url", URL);
    map->add("week", WEEK);
    // No need to register "text" because it is the default type.
    return map.release();
}

void HTMLInputElement::updateType()
{
    static const InputTypeMap* typeMap = createTypeMap().leakPtr();
    const AtomicString& typeString = fastGetAttribute(typeAttr);
    DeprecatedInputType newType = typeString.isEmpty() ? TEXT : typeMap->get(typeString);

    // IMPORTANT: Don't allow the type to be changed to FILE after the first
    // type change, otherwise a JavaScript programmer would be able to set a text
    // field's value to something like /etc/passwd and then change it to a file field.
    if (deprecatedInputType() != newType) {
        if (newType == FILE && m_haveType)
            // Set the attribute back to the old value.
            // Useful in case we were called from inside parseMappedAttribute.
            setAttribute(typeAttr, type());
        else {
            checkedRadioButtons().removeButton(this);

            if (newType == FILE && !m_fileList)
                m_fileList = FileList::create();

            bool wasAttached = attached();
            if (wasAttached)
                detach();

            bool didStoreValue = storesValueSeparateFromAttribute();
            bool wasPasswordField = deprecatedInputType() == PASSWORD;
            bool didRespectHeightAndWidth = respectHeightAndWidthAttrs();
            m_deprecatedTypeNumber = newType;
            m_inputType = InputType::create(this, typeString);
            setNeedsWillValidateCheck();
            bool willStoreValue = storesValueSeparateFromAttribute();
            bool isPasswordField = deprecatedInputType() == PASSWORD;
            bool willRespectHeightAndWidth = respectHeightAndWidthAttrs();

            if (didStoreValue && !willStoreValue && !m_data.value().isNull()) {
                setAttribute(valueAttr, m_data.value());
                m_data.setValue(String());
            }
            if (!didStoreValue && willStoreValue)
                m_data.setValue(sanitizeValue(getAttribute(valueAttr)));
            else
                InputElement::updateValueIfNeeded(m_data, this);

            if (wasPasswordField && !isPasswordField)
                unregisterForActivationCallbackIfNeeded();
            else if (!wasPasswordField && isPasswordField)
                registerForActivationCallbackIfNeeded();

            if (didRespectHeightAndWidth != willRespectHeightAndWidth) {
                NamedNodeMap* map = attributeMap();
                ASSERT(map);
                if (Attribute* height = map->getAttributeItem(heightAttr))
                    attributeChanged(height, false);
                if (Attribute* width = map->getAttributeItem(widthAttr))
                    attributeChanged(width, false);
                if (Attribute* align = map->getAttributeItem(alignAttr))
                    attributeChanged(align, false);
            }

            if (wasAttached) {
                attach();
                if (document()->focusedNode() == this)
                    updateFocusAppearance(true);
            }

            checkedRadioButtons().addButton(this);
        }

        setNeedsValidityCheck();
        InputElement::notifyFormStateChanged(this);
    }
    m_haveType = true;

    if (deprecatedInputType() != IMAGE && m_imageLoader)
        m_imageLoader.clear();
}

const AtomicString& HTMLInputElement::formControlType() const
{
    return m_inputType->formControlType();
}

bool HTMLInputElement::saveFormControlState(String& result) const
{
    return m_inputType->saveFormControlState(result);
}

void HTMLInputElement::restoreFormControlState(const String& state)
{
    m_inputType->restoreFormControlState(state);
}

bool HTMLInputElement::canStartSelection() const
{
    if (!isTextField())
        return false;
    return HTMLFormControlElementWithState::canStartSelection();
}

bool HTMLInputElement::canHaveSelection() const
{
    return isTextField();
}

void HTMLInputElement::accessKeyAction(bool sendToAnyElement)
{
    switch (deprecatedInputType()) {
    case BUTTON:
    case CHECKBOX:
    case FILE:
    case IMAGE:
    case RADIO:
    case RANGE:
    case RESET:
    case SUBMIT:
        focus(false);
        // send the mouse button events iff the caller specified sendToAnyElement
        dispatchSimulatedClick(0, sendToAnyElement);
        break;
    case HIDDEN:
        // a no-op for this type
        break;
    case COLOR:
    case DATE:
    case DATETIME:
    case DATETIMELOCAL:
    case EMAIL:
    case ISINDEX:
    case MONTH:
    case NUMBER:
    case PASSWORD:
    case SEARCH:
    case TELEPHONE:
    case TEXT:
    case TIME:
    case URL:
    case WEEK:
        // should never restore previous selection here
        focus(false);
         break;
    }
}

bool HTMLInputElement::mapToEntry(const QualifiedName& attrName, MappedAttributeEntry& result) const
{
    if (((attrName == heightAttr || attrName == widthAttr) && respectHeightAndWidthAttrs())
        || attrName == vspaceAttr 
        || attrName == hspaceAttr) {
        result = eUniversal;
        return false;
    } 

    if (attrName == alignAttr) {
        if (deprecatedInputType() == IMAGE) {
            // Share with <img> since the alignment behavior is the same.
            result = eReplaced;
            return false;
        }
    }

    return HTMLElement::mapToEntry(attrName, result);
}

void HTMLInputElement::parseMappedAttribute(Attribute* attr)
{
    if (attr->name() == nameAttr) {
        checkedRadioButtons().removeButton(this);
        m_data.setName(attr->value());
        checkedRadioButtons().addButton(this);
        HTMLFormControlElementWithState::parseMappedAttribute(attr);
    } else if (attr->name() == autocompleteAttr) {
        if (equalIgnoringCase(attr->value(), "off")) {
            m_autocomplete = Off;
            registerForActivationCallbackIfNeeded();
        } else {
            bool needsToUnregister = m_autocomplete == Off;

            if (attr->isEmpty())
                m_autocomplete = Uninitialized;
            else
                m_autocomplete = On;

            if (needsToUnregister)
                unregisterForActivationCallbackIfNeeded();
        }
    } else if (attr->name() == typeAttr) {
        updateType();
    } else if (attr->name() == valueAttr) {
        // We only need to setChanged if the form is looking at the default value right now.
        if (m_data.value().isNull())
            setNeedsStyleRecalc();
        setFormControlValueMatchesRenderer(false);
        setNeedsValidityCheck();
    } else if (attr->name() == checkedAttr) {
        m_defaultChecked = !attr->isNull();
        if (m_useDefaultChecked) {
            setChecked(m_defaultChecked);
            m_useDefaultChecked = true;
        }
        setNeedsValidityCheck();
    } else if (attr->name() == maxlengthAttr) {
        InputElement::parseMaxLengthAttribute(m_data, this, this, attr);
        setNeedsValidityCheck();
    } else if (attr->name() == sizeAttr)
        InputElement::parseSizeAttribute(m_data, this, attr);
    else if (attr->name() == altAttr) {
        if (renderer() && deprecatedInputType() == IMAGE)
            toRenderImage(renderer())->updateAltText();
    } else if (attr->name() == srcAttr) {
        if (renderer() && deprecatedInputType() == IMAGE) {
            if (!m_imageLoader)
                m_imageLoader = adoptPtr(new HTMLImageLoader(this));
            m_imageLoader->updateFromElementIgnoringPreviousError();
        }
    } else if (attr->name() == usemapAttr || attr->name() == accesskeyAttr) {
        // FIXME: ignore for the moment
    } else if (attr->name() == vspaceAttr) {
        addCSSLength(attr, CSSPropertyMarginTop, attr->value());
        addCSSLength(attr, CSSPropertyMarginBottom, attr->value());
    } else if (attr->name() == hspaceAttr) {
        addCSSLength(attr, CSSPropertyMarginLeft, attr->value());
        addCSSLength(attr, CSSPropertyMarginRight, attr->value());
    } else if (attr->name() == alignAttr) {
        if (deprecatedInputType() == IMAGE)
            addHTMLAlignment(attr);
    } else if (attr->name() == widthAttr) {
        if (respectHeightAndWidthAttrs())
            addCSSLength(attr, CSSPropertyWidth, attr->value());
    } else if (attr->name() == heightAttr) {
        if (respectHeightAndWidthAttrs())
            addCSSLength(attr, CSSPropertyHeight, attr->value());
    } else if (attr->name() == onsearchAttr) {
        // Search field and slider attributes all just cause updateFromElement to be called through style recalcing.
        setAttributeEventListener(eventNames().searchEvent, createAttributeEventListener(this, attr));
    } else if (attr->name() == resultsAttr) {
        int oldResults = m_maxResults;
        m_maxResults = !attr->isNull() ? std::min(attr->value().toInt(), maxSavedResults) : -1;
        // FIXME: Detaching just for maxResults change is not ideal.  We should figure out the right
        // time to relayout for this change.
        if (m_maxResults != oldResults && (m_maxResults <= 0 || oldResults <= 0) && attached()) {
            detach();
            attach();
        }
        setNeedsStyleRecalc();
    } else if (attr->name() == autosaveAttr
               || attr->name() == incrementalAttr)
        setNeedsStyleRecalc();
    else if (attr->name() == minAttr
             || attr->name() == maxAttr) {
        if (deprecatedInputType() == RANGE) {
            // Sanitize the value.
            setValue(value());
            setNeedsStyleRecalc();
        }
        setNeedsValidityCheck();
    } else if (attr->name() == multipleAttr
             || attr->name() == patternAttr
             || attr->name() == precisionAttr
             || attr->name() == stepAttr)
        setNeedsValidityCheck();
#if ENABLE(DATALIST)
    else if (attr->name() == listAttr)
        m_hasNonEmptyList = !attr->isEmpty();
        // FIXME: we need to tell this change to a renderer if the attribute affects the appearance.
#endif
#if ENABLE(INPUT_SPEECH)
    else if (attr->name() == webkitspeechAttr) {
      if (renderer())
          toRenderTextControlSingleLine(renderer())->speechAttributeChanged();
      setNeedsStyleRecalc();
    } else if (attr->name() == onwebkitspeechchangeAttr)
        setAttributeEventListener(eventNames().webkitspeechchangeEvent, createAttributeEventListener(this, attr));
#endif
    else
        HTMLTextFormControlElement::parseMappedAttribute(attr);
}

bool HTMLInputElement::rendererIsNeeded(RenderStyle *style)
{
    if (deprecatedInputType() == HIDDEN)
        return false;
    return HTMLFormControlElementWithState::rendererIsNeeded(style);
}

RenderObject* HTMLInputElement::createRenderer(RenderArena* arena, RenderStyle* style)
{
    return m_inputType->createRenderer(arena, style);
}

void HTMLInputElement::attach()
{
    if (!m_inited) {
        if (!m_haveType)
            updateType();
        m_inited = true;
    }

    HTMLFormControlElementWithState::attach();

    if (deprecatedInputType() == IMAGE) {
        if (!m_imageLoader)
            m_imageLoader = adoptPtr(new HTMLImageLoader(this));
        m_imageLoader->updateFromElement();
        if (renderer() && m_imageLoader->haveFiredBeforeLoadEvent()) {
            RenderImage* renderImage = toRenderImage(renderer());
            RenderImageResource* renderImageResource = renderImage->imageResource();
            renderImageResource->setCachedImage(m_imageLoader->image()); 

            // If we have no image at all because we have no src attribute, set
            // image height and width for the alt text instead.
            if (!m_imageLoader->image() && !renderImageResource->cachedImage())
                renderImage->setImageSizeForAltText();
        }
    }

    if (deprecatedInputType() == RADIO)
        updateCheckedRadioButtons();

    if (document()->focusedNode() == this)
        document()->updateFocusAppearanceSoon(true /* restore selection */);
}

void HTMLInputElement::detach()
{
    HTMLFormControlElementWithState::detach();
    setFormControlValueMatchesRenderer(false);
}

String HTMLInputElement::altText() const
{
    // http://www.w3.org/TR/1998/REC-html40-19980424/appendix/notes.html#altgen
    // also heavily discussed by Hixie on bugzilla
    // note this is intentionally different to HTMLImageElement::altText()
    String alt = getAttribute(altAttr);
    // fall back to title attribute
    if (alt.isNull())
        alt = getAttribute(titleAttr);
    if (alt.isNull())
        alt = getAttribute(valueAttr);
    if (alt.isEmpty())
        alt = inputElementAltText();
    return alt;
}

bool HTMLInputElement::isSuccessfulSubmitButton() const
{
    // HTML spec says that buttons must have names to be considered successful.
    // However, other browsers do not impose this constraint. So we do likewise.
    return !disabled() && (deprecatedInputType() == IMAGE || deprecatedInputType() == SUBMIT);
}

bool HTMLInputElement::isActivatedSubmit() const
{
    return m_activeSubmit;
}

void HTMLInputElement::setActivatedSubmit(bool flag)
{
    m_activeSubmit = flag;
}

bool HTMLInputElement::appendFormData(FormDataList& encoding, bool multipart)
{
    return m_inputType->isFormDataAppendable() && m_inputType->appendFormData(encoding, multipart);
}

void HTMLInputElement::reset()
{
    if (storesValueSeparateFromAttribute())
        setValue(String());

    setChecked(m_defaultChecked);
    m_useDefaultChecked = true;
}

bool HTMLInputElement::isTextField() const
{
    return m_inputType->isTextField();
}

bool HTMLInputElement::isTextType() const
{
    return m_inputType->isTextType();
}

void HTMLInputElement::setChecked(bool nowChecked, bool sendChangeEvent)
{
    if (checked() == nowChecked)
        return;

    checkedRadioButtons().removeButton(this);

    m_useDefaultChecked = false;
    m_checked = nowChecked;
    setNeedsStyleRecalc();

    updateCheckedRadioButtons();

    // Ideally we'd do this from the render tree (matching
    // RenderTextView), but it's not possible to do it at the moment
    // because of the way the code is structured.
    if (renderer() && AXObjectCache::accessibilityEnabled())
        renderer()->document()->axObjectCache()->postNotification(renderer(), AXObjectCache::AXCheckedStateChanged, true);

    // Only send a change event for items in the document (avoid firing during
    // parsing) and don't send a change event for a radio button that's getting
    // unchecked to match other browsers. DOM is not a useful standard for this
    // because it says only to fire change events at "lose focus" time, which is
    // definitely wrong in practice for these types of elements.
    if (sendChangeEvent && inDocument() && (deprecatedInputType() != RADIO || nowChecked))
        dispatchFormControlChangeEvent();
}

void HTMLInputElement::setIndeterminate(bool newValue)
{
    // Only checkboxes and radio buttons honor indeterminate.
    if (!allowsIndeterminate() || indeterminate() == newValue)
        return;

    m_indeterminate = newValue;

    setNeedsStyleRecalc();

    if (renderer() && renderer()->style()->hasAppearance())
        renderer()->theme()->stateChanged(renderer(), CheckedState);
}

int HTMLInputElement::size() const
{
    return m_data.size();
}

void HTMLInputElement::copyNonAttributeProperties(const Element* source)
{
    const HTMLInputElement* sourceElement = static_cast<const HTMLInputElement*>(source);

    m_data.setValue(sourceElement->m_data.value());
    setChecked(sourceElement->m_checked);
    m_defaultChecked = sourceElement->m_defaultChecked;
    m_useDefaultChecked = sourceElement->m_useDefaultChecked;
    m_indeterminate = sourceElement->m_indeterminate;

    HTMLFormControlElementWithState::copyNonAttributeProperties(source);
}

String HTMLInputElement::value() const
{
    if (deprecatedInputType() == FILE) {
        if (!m_fileList->isEmpty()) {
            // HTML5 tells us that we're supposed to use this goofy value for
            // file input controls.  Historically, browsers reveals the real
            // file path, but that's a privacy problem.  Code on the web
            // decided to try to parse the value by looking for backslashes
            // (because that's what Windows file paths use).  To be compatible
            // with that code, we make up a fake path for the file.
            return "C:\\fakepath\\" + m_fileList->item(0)->fileName();
        }
        return String();
    }

    String value = m_data.value();
    if (value.isNull()) {
        value = sanitizeValue(fastGetAttribute(valueAttr));
        
        // If no attribute exists, extra handling may be necessary.
        // For Checkbox Types just use "on" or "" based off the checked() state of the control.
        // For a Range Input use the calculated default value.
        if (value.isNull()) {
            if (deprecatedInputType() == CHECKBOX || deprecatedInputType() == RADIO)
                return checked() ? "on" : "";
            if (deprecatedInputType() == RANGE)
                return serializeForNumberType(StepRange(this).defaultValue());
        }
    }

    return value;
}

String HTMLInputElement::valueWithDefault() const
{
    String v = value();
    if (v.isNull()) {
        switch (deprecatedInputType()) {
        case BUTTON:
        case CHECKBOX:
        case COLOR:
        case DATE:
        case DATETIME:
        case DATETIMELOCAL:
        case EMAIL:
        case FILE:
        case HIDDEN:
        case IMAGE:
        case ISINDEX:
        case MONTH:
        case NUMBER:
        case PASSWORD:
        case RADIO:
        case RANGE:
        case SEARCH:
        case TELEPHONE:
        case TEXT:
        case TIME:
        case URL:
        case WEEK:
            break;
        case RESET:
            v = resetButtonDefaultLabel();
            break;
        case SUBMIT:
            v = submitButtonDefaultLabel();
            break;
        }
    }
    return v;
}

void HTMLInputElement::setValueForUser(const String& value)
{
    // Call setValue and make it send a change event.
    setValue(value, true);
}

const String& HTMLInputElement::suggestedValue() const
{
    return m_data.suggestedValue();
}

void HTMLInputElement::setSuggestedValue(const String& value)
{
    if (deprecatedInputType() != TEXT)
        return;
    setFormControlValueMatchesRenderer(false);
    m_data.setSuggestedValue(sanitizeValue(value));
    updatePlaceholderVisibility(false);
    if (renderer())
        renderer()->updateFromElement();
    setNeedsStyleRecalc();
}

void HTMLInputElement::setValue(const String& value, bool sendChangeEvent)
{
    // For security reasons, we don't allow setting the filename, but we do allow clearing it.
    // The HTML5 spec (as of the 10/24/08 working draft) says that the value attribute isn't applicable to the file upload control
    // but we don't want to break existing websites, who may be relying on this method to clear things.
    if (deprecatedInputType() == FILE && !value.isEmpty())
        return;

    setFormControlValueMatchesRenderer(false);
    if (storesValueSeparateFromAttribute()) {
        if (deprecatedInputType() == FILE)
            m_fileList->clear();
        else {
            m_data.setValue(sanitizeValue(value));
            if (isTextField())
                updatePlaceholderVisibility(false);
        }
        setNeedsStyleRecalc();
    } else
        setAttribute(valueAttr, sanitizeValue(value));

    setNeedsValidityCheck();

    if (isTextField()) {
        unsigned max = m_data.value().length();
        if (document()->focusedNode() == this)
            InputElement::updateSelectionRange(this, this, max, max);
        else
            cacheSelection(max, max);
        m_data.setSuggestedValue(String());
    }

    // Don't dispatch the change event when focused, it will be dispatched
    // when the control loses focus.
    if (sendChangeEvent && document()->focusedNode() != this)
        dispatchFormControlChangeEvent();

    InputElement::notifyFormStateChanged(this);
}

double HTMLInputElement::valueAsDate() const
{
    return m_inputType->valueAsDate();
}

void HTMLInputElement::setValueAsDate(double value, ExceptionCode& ec)
{
    m_inputType->setValueAsDate(value, ec);
}

double HTMLInputElement::valueAsNumber() const
{
    return m_inputType->valueAsNumber();
}

void HTMLInputElement::setValueAsNumber(double newValue, ExceptionCode& ec)
{
    if (!isfinite(newValue)) {
        ec = NOT_SUPPORTED_ERR;
        return;
    }
    m_inputType->setValueAsNumber(newValue, ec);
}

String HTMLInputElement::placeholder() const
{
    return getAttribute(placeholderAttr).string();
}

void HTMLInputElement::setPlaceholder(const String& value)
{
    setAttribute(placeholderAttr, value);
}

bool HTMLInputElement::searchEventsShouldBeDispatched() const
{
    return hasAttribute(incrementalAttr);
}

void HTMLInputElement::setValueFromRenderer(const String& value)
{
    // File upload controls will always use setFileListFromRenderer.
    ASSERT(deprecatedInputType() != FILE);
    m_data.setSuggestedValue(String());
    InputElement::setValueFromRenderer(m_data, this, this, value);
    updatePlaceholderVisibility(false);
    setNeedsValidityCheck();

    // Clear autofill flag (and yellow background) on user edit.
    setAutofilled(false);
}

void HTMLInputElement::setFileListFromRenderer(const Vector<String>& paths)
{
    m_fileList->clear();
    int size = paths.size();

#if ENABLE(DIRECTORY_UPLOAD)
    // If a directory is being selected, the UI allows a directory to be chosen
    // and the paths provided here share a root directory somewhere up the tree;
    // we want to store only the relative paths from that point.
    if (webkitdirectory() && size > 0) {
        String rootPath = directoryName(paths[0]);
        // Find the common root path.
        for (int i = 1; i < size; i++) {
            while (!paths[i].startsWith(rootPath))
                rootPath = directoryName(rootPath);
        }
        rootPath = directoryName(rootPath);
        ASSERT(rootPath.length());
        for (int i = 0; i < size; i++) {
            // Normalize backslashes to slashes before exposing the relative path to script.
            String relativePath = paths[i].substring(1 + rootPath.length()).replace('\\','/');
            m_fileList->append(File::create(relativePath, paths[i]));
        }
    } else {
        for (int i = 0; i < size; i++)
            m_fileList->append(File::create(paths[i]));
    }
#else
    for (int i = 0; i < size; i++)
        m_fileList->append(File::create(paths[i]));
#endif

    setFormControlValueMatchesRenderer(true);
    InputElement::notifyFormStateChanged(this);
    setNeedsValidityCheck();
}

bool HTMLInputElement::storesValueSeparateFromAttribute() const
{
    switch (deprecatedInputType()) {
    case BUTTON:
    case CHECKBOX:
    case HIDDEN:
    case IMAGE:
    case RADIO:
    case RESET:
    case SUBMIT:
        return false;
    case COLOR:
    case DATE:
    case DATETIME:
    case DATETIMELOCAL:
    case EMAIL:
    case FILE:
    case ISINDEX:
    case MONTH:
    case NUMBER:
    case PASSWORD:
    case RANGE:
    case SEARCH:
    case TELEPHONE:
    case TEXT:
    case TIME:
    case URL:
    case WEEK:
        return true;
    }
    return false;
}

struct EventHandlingState : FastAllocBase {
    RefPtr<HTMLInputElement> m_currRadio;
    bool m_indeterminate;
    bool m_checked;
    
    EventHandlingState(bool indeterminate, bool checked)
        : m_indeterminate(indeterminate)
        , m_checked(checked) { }
};

void* HTMLInputElement::preDispatchEventHandler(Event* evt)
{
    // preventDefault or "return false" are used to reverse the automatic checking/selection we do here.
    // This result gives us enough info to perform the "undo" in postDispatch of the action we take here.
    void* result = 0; 
    if ((deprecatedInputType() == CHECKBOX || deprecatedInputType() == RADIO) && evt->type() == eventNames().clickEvent) {
        OwnPtr<EventHandlingState> state = adoptPtr(new EventHandlingState(indeterminate(), checked()));
        if (deprecatedInputType() == CHECKBOX) {
            if (indeterminate())
                setIndeterminate(false);
            else
                setChecked(!checked(), true);
        } else {
            // For radio buttons, store the current selected radio object.
            // We really want radio groups to end up in sane states, i.e., to have something checked.
            // Therefore if nothing is currently selected, we won't allow this action to be "undone", since
            // we want some object in the radio group to actually get selected.
            HTMLInputElement* currRadio = checkedRadioButtons().checkedButtonForGroup(name());
            if (currRadio) {
                // We have a radio button selected that is not us.  Cache it in our result field and ref it so
                // that it can't be destroyed.
                state->m_currRadio = currRadio;
            }
            if (indeterminate())
                setIndeterminate(false);
            setChecked(true, true);
        }
        result = state.leakPtr(); // FIXME: Check whether this actually ends up leaking.
    }
    return result;
}

void HTMLInputElement::postDispatchEventHandler(Event *evt, void* data)
{
    if ((deprecatedInputType() == CHECKBOX || deprecatedInputType() == RADIO) && evt->type() == eventNames().clickEvent) {
        
        if (EventHandlingState* state = reinterpret_cast<EventHandlingState*>(data)) {
            if (deprecatedInputType() == CHECKBOX) {
                // Reverse the checking we did in preDispatch.
                if (evt->defaultPrevented() || evt->defaultHandled()) {
                    setIndeterminate(state->m_indeterminate);
                    setChecked(state->m_checked);
                }
            } else {
                HTMLInputElement* input = state->m_currRadio.get();
                if (evt->defaultPrevented() || evt->defaultHandled()) {
                    // Restore the original selected radio button if possible.
                    // Make sure it is still a radio button and only do the restoration if it still
                    // belongs to our group.

                    if (input && input->form() == form() && input->deprecatedInputType() == RADIO && input->name() == name()) {
                        // Ok, the old radio button is still in our form and in our group and is still a 
                        // radio button, so it's safe to restore selection to it.
                        input->setChecked(true);
                    }
                    setIndeterminate(state->m_indeterminate);
                }
            }
            delete state;
        }

        // Left clicks on radio buttons and check boxes already performed default actions in preDispatchEventHandler(). 
        evt->setDefaultHandled();
    }
}

void HTMLInputElement::defaultEventHandler(Event* evt)
{
    if (evt->isMouseEvent() && evt->type() == eventNames().clickEvent) {
        m_inputType->handleClickEvent(static_cast<MouseEvent*>(evt));
        if (evt->defaultHandled())
            return;
    }

    if (evt->isKeyboardEvent() && evt->type() == eventNames().keydownEvent) {
        m_inputType->handleKeydownEvent(static_cast<KeyboardEvent*>(evt));
        if (evt->defaultHandled())
            return;
    }

    // Call the base event handler before any of our own event handling for almost all events in text fields.
    // Makes editing keyboard handling take precedence over the keydown and keypress handling in this function.
    bool callBaseClassEarly = isTextField() && (evt->type() == eventNames().keydownEvent || evt->type() == eventNames().keypressEvent);
    if (callBaseClassEarly) {
        HTMLFormControlElementWithState::defaultEventHandler(evt);
        if (evt->defaultHandled())
            return;
    }

    // DOMActivate events cause the input to be "activated" - in the case of image and submit inputs, this means
    // actually submitting the form. For reset inputs, the form is reset. These events are sent when the user clicks
    // on the element, or presses enter while it is the active element. JavaScript code wishing to activate the element
    // must dispatch a DOMActivate event - a click event will not do the job.
    if (evt->type() == eventNames().DOMActivateEvent) {
        m_inputType->handleDOMActivateEvent(evt);
        if (evt->defaultHandled())
            return;
    }

    // Use key press event here since sending simulated mouse events
    // on key down blocks the proper sending of the key press event.
    if (evt->isKeyboardEvent() && evt->type() == eventNames().keypressEvent) {
        m_inputType->handleKeypressEvent(static_cast<KeyboardEvent*>(evt));
        if (evt->defaultHandled())
            return;
    }

    if (evt->isKeyboardEvent() && evt->type() == eventNames().keyupEvent) {
        m_inputType->handleKeyupEvent(static_cast<KeyboardEvent*>(evt));
        if (evt->defaultHandled())
            return;
    }

    if (m_inputType->shouldSubmitImplicitly(evt)) {
        if (isSearchField()) {
            addSearchResult();
            onSearch();
        }
        // Fire onChange for text fields.
        RenderObject* r = renderer();
        if (r && r->isTextField() && toRenderTextControl(r)->wasChangedSinceLastChangeEvent()) {
            dispatchFormControlChangeEvent();
            // Refetch the renderer since arbitrary JS code run during onchange can do anything, including destroying it.
            r = renderer();
            if (r && r->isTextField())
                toRenderTextControl(r)->setChangedSinceLastChangeEvent(false);
        }

        RefPtr<HTMLFormElement> formForSubmission = m_inputType->formForSubmission();
        // Form may never have been present, or may have been destroyed by code responding to the change event.
        if (formForSubmission)
            formForSubmission->submitImplicitly(evt, canTriggerImplicitSubmission());

        evt->setDefaultHandled();
        return;
    }

    if (evt->isBeforeTextInsertedEvent())
        m_inputType->handleBeforeTextInsertedEvent(static_cast<BeforeTextInsertedEvent*>(evt));

    if (evt->isWheelEvent()) {
        m_inputType->handleWheelEvent(static_cast<WheelEvent*>(evt));
        if (evt->defaultHandled())
            return;
    }

    m_inputType->forwardEvent(evt);

    if (!callBaseClassEarly && !evt->defaultHandled())
        HTMLFormControlElementWithState::defaultEventHandler(evt);
}

bool HTMLInputElement::isURLAttribute(Attribute *attr) const
{
    return (attr->name() == srcAttr || attr->name() == formactionAttr);
}

String HTMLInputElement::defaultValue() const
{
    return getAttribute(valueAttr);
}

void HTMLInputElement::setDefaultValue(const String &value)
{
    setAttribute(valueAttr, value);
}

bool HTMLInputElement::defaultChecked() const
{
    return fastHasAttribute(checkedAttr);
}

void HTMLInputElement::setDefaultName(const AtomicString& name)
{
    m_data.setName(name);
}

String HTMLInputElement::accept() const
{
    return getAttribute(acceptAttr);
}

String HTMLInputElement::alt() const
{
    return getAttribute(altAttr);
}

int HTMLInputElement::maxLength() const
{
    return m_data.maxLength();
}

void HTMLInputElement::setMaxLength(int maxLength, ExceptionCode& ec)
{
    if (maxLength < 0)
        ec = INDEX_SIZE_ERR;
    else
        setAttribute(maxlengthAttr, String::number(maxLength));
}

bool HTMLInputElement::multiple() const
{
    return fastHasAttribute(multipleAttr);
}

#if ENABLE(DIRECTORY_UPLOAD)
bool HTMLInputElement::webkitdirectory() const
{
    return fastHasAttribute(webkitdirectoryAttr);
}
#endif

void HTMLInputElement::setSize(unsigned size)
{
    setAttribute(sizeAttr, String::number(size));
}

KURL HTMLInputElement::src() const
{
    return document()->completeURL(getAttribute(srcAttr));
}

void HTMLInputElement::setAutofilled(bool b)
{
    if (b == m_autofilled)
        return;
        
    m_autofilled = b;
    setNeedsStyleRecalc();
}

FileList* HTMLInputElement::files()
{
    if (deprecatedInputType() != FILE)
        return 0;
    return m_fileList.get();
}

bool HTMLInputElement::isAcceptableValue(const String& proposedValue) const
{
    if (deprecatedInputType() != NUMBER)
        return true;
    return proposedValue.isEmpty() || parseToDoubleForNumberType(proposedValue, 0);
}

String HTMLInputElement::sanitizeValue(const String& proposedValue) const
{
    if (deprecatedInputType() == NUMBER)
        return parseToDoubleForNumberType(proposedValue, 0) ? proposedValue : String();

    if (isTextField())
        return InputElement::sanitizeValueForTextField(this, proposedValue);

    // If the proposedValue is null than this is a reset scenario and we
    // want the range input's value attribute to take priority over the
    // calculated default (middle) value.
    if (deprecatedInputType() == RANGE && !proposedValue.isNull())
        return serializeForNumberType(StepRange(this).clampValue(proposedValue));

    return proposedValue;
}

bool HTMLInputElement::hasUnacceptableValue() const
{
    return deprecatedInputType() == NUMBER && renderer() && !isAcceptableValue(toRenderTextControl(renderer())->text());
}

bool HTMLInputElement::isInRange() const
{
    return m_inputType->supportsRangeLimitation() && !rangeUnderflow(value()) && !rangeOverflow(value());
}

bool HTMLInputElement::isOutOfRange() const
{
    return m_inputType->supportsRangeLimitation() && (rangeUnderflow(value()) || rangeOverflow(value()));
}

bool HTMLInputElement::needsActivationCallback()
{
    return deprecatedInputType() == PASSWORD || m_autocomplete == Off;
}

void HTMLInputElement::registerForActivationCallbackIfNeeded()
{
    if (needsActivationCallback())
        document()->registerForDocumentActivationCallbacks(this);
}

void HTMLInputElement::unregisterForActivationCallbackIfNeeded()
{
    if (!needsActivationCallback())
        document()->unregisterForDocumentActivationCallbacks(this);
}

bool HTMLInputElement::isRequiredFormControl() const
{
    return m_inputType->supportsRequired() && required();
}

void HTMLInputElement::cacheSelection(int start, int end)
{
    m_data.setCachedSelectionStart(start);
    m_data.setCachedSelectionEnd(end);
}

void HTMLInputElement::addSearchResult()
{
    ASSERT(isSearchField());
    if (renderer())
        toRenderTextControlSingleLine(renderer())->addSearchResult();
}

void HTMLInputElement::onSearch()
{
    ASSERT(isSearchField());
    if (renderer())
        toRenderTextControlSingleLine(renderer())->stopSearchEventTimer();
    dispatchEvent(Event::create(eventNames().searchEvent, true, false));
}

void HTMLInputElement::documentDidBecomeActive()
{
    ASSERT(needsActivationCallback());
    reset();
}

void HTMLInputElement::willMoveToNewOwnerDocument()
{
    if (m_imageLoader)
        m_imageLoader->elementWillMoveToNewOwnerDocument();

    // Always unregister for cache callbacks when leaving a document, even if we would otherwise like to be registered
    if (needsActivationCallback())
        document()->unregisterForDocumentActivationCallbacks(this);
        
    document()->checkedRadioButtons().removeButton(this);
    
    HTMLFormControlElementWithState::willMoveToNewOwnerDocument();
}

void HTMLInputElement::didMoveToNewOwnerDocument()
{
    registerForActivationCallbackIfNeeded();
        
    HTMLFormControlElementWithState::didMoveToNewOwnerDocument();
}
    
void HTMLInputElement::addSubresourceAttributeURLs(ListHashSet<KURL>& urls) const
{
    HTMLFormControlElementWithState::addSubresourceAttributeURLs(urls);

    addSubresourceURL(urls, src());
}

bool HTMLInputElement::recalcWillValidate() const
{
    return m_inputType->supportsValidation() && HTMLFormControlElementWithState::recalcWillValidate();
}

#if ENABLE(DATALIST)

HTMLElement* HTMLInputElement::list() const
{
    return dataList();
}

HTMLDataListElement* HTMLInputElement::dataList() const
{
    if (!m_hasNonEmptyList)
        return 0;

    switch (deprecatedInputType()) {
    case COLOR:
    case DATE:
    case DATETIME:
    case DATETIMELOCAL:
    case EMAIL:
    case MONTH:
    case NUMBER:
    case RANGE:
    case SEARCH:
    case TELEPHONE:
    case TEXT:
    case TIME:
    case URL:
    case WEEK: {
        Element* element = document()->getElementById(getAttribute(listAttr));
        if (element && element->hasTagName(datalistTag))
            return static_cast<HTMLDataListElement*>(element);
        break;
    }
    case BUTTON:
    case CHECKBOX:
    case FILE:
    case HIDDEN:
    case IMAGE:
    case ISINDEX:
    case PASSWORD:
    case RADIO:
    case RESET:
    case SUBMIT:
        break;
    }
    return 0;
}

HTMLOptionElement* HTMLInputElement::selectedOption() const
{
    String currentValue = value();
    // The empty value never matches to a datalist option because it
    // doesn't represent a suggestion according to the standard.
    if (currentValue.isEmpty())
        return 0;

    HTMLDataListElement* sourceElement = dataList();
    if (!sourceElement)
        return 0;
    RefPtr<HTMLCollection> options = sourceElement->options();
    for (unsigned i = 0; options && i < options->length(); ++i) {
        HTMLOptionElement* option = static_cast<HTMLOptionElement*>(options->item(i));
        if (!option->disabled() && currentValue == option->value())
            return option;
    }
    return 0;
}

#endif // ENABLE(DATALIST)

void HTMLInputElement::stepUpFromRenderer(int n)
{
    // The differences from stepUp()/stepDown():
    //
    // Difference 1: the current value
    // If the current value is not a number, including empty, the current value is assumed as 0.
    //   * If 0 is in-range, and matches to step value
    //     - The value should be the +step if n > 0
    //     - The value should be the -step if n < 0
    //     If -step or +step is out of range, new value should be 0.
    //   * If 0 is smaller than the minimum value
    //     - The value should be the minimum value for any n
    //   * If 0 is larger than the maximum value
    //     - The value should be the maximum value for any n
    //   * If 0 is in-range, but not matched to step value
    //     - The value should be the larger matched value nearest to 0 if n > 0
    //       e.g. <input type=number min=-100 step=3> -> 2
    //     - The value should be the smaler matched value nearest to 0 if n < 0
    //       e.g. <input type=number min=-100 step=3> -> -1
    //   As for date/datetime-local/month/time/week types, the current value is assumed as "the current local date/time".
    //   As for datetime type, the current value is assumed as "the current date/time in UTC".
    // If the current value is smaller than the minimum value:
    //  - The value should be the minimum value if n > 0
    //  - Nothing should happen if n < 0
    // If the current value is larger than the maximum value:
    //  - The value should be the maximum value if n < 0
    //  - Nothing should happen if n > 0
    //
    // Difference 2: clamping steps
    // If the current value is not matched to step value:
    // - The value should be the larger matched value nearest to 0 if n > 0
    //   e.g. <input type=number value=3 min=-100 step=3> -> 5
    // - The value should be the smaler matched value nearest to 0 if n < 0
    //   e.g. <input type=number value=3 min=-100 step=3> -> 2
    //
    // n is assumed as -n if step < 0.

    ASSERT(hasSpinButton() || m_inputType->isRangeControl());
    if (!hasSpinButton() && !m_inputType->isRangeControl())
        return;
    ASSERT(n);
    if (!n)
        return;

    unsigned stepDecimalPlaces, baseDecimalPlaces;
    double step, base;
    // The value will be the default value after stepping for <input value=(empty/invalid) step="any" />
    // FIXME: Not any changes after stepping, even if it is an invalid value, may be better.
    // (e.g. Stepping-up for <input type="number" value="foo" step="any" /> => "foo")
    if (equalIgnoringCase(getAttribute(stepAttr), "any"))
        step = 0;
    else if (!getAllowedValueStepWithDecimalPlaces(&step, &stepDecimalPlaces))
        return;
    base = m_inputType->stepBaseWithDecimalPlaces(&baseDecimalPlaces);
    baseDecimalPlaces = min(baseDecimalPlaces, 16u);

    int sign;
    if (step > 0)
        sign = n;
    else if (step < 0)
        sign = -n;
    else
        sign = 0;

    const double nan = numeric_limits<double>::quiet_NaN();
    String currentStringValue = value();
    double current = m_inputType->parseToDouble(currentStringValue, nan);
    if (!isfinite(current)) {
        ExceptionCode ec;
        current = m_inputType->defaultValueForStepUp();
        setValueAsNumber(current, ec);
    }
    if ((sign > 0 && current < m_inputType->minimum()) || (sign < 0 && current > m_inputType->maximum()))
        setValue(m_inputType->serialize(sign > 0 ? m_inputType->minimum() : m_inputType->maximum()));
    else {
        ExceptionCode ec;
        if (stepMismatch(currentStringValue)) {
            ASSERT(step);
            double newValue;
            double scale = pow(10.0, static_cast<double>(max(stepDecimalPlaces, baseDecimalPlaces)));

            if (sign < 0)
                newValue = round((base + floor((current - base) / step) * step) * scale) / scale;
            else if (sign > 0)
                newValue = round((base + ceil((current - base) / step) * step) * scale) / scale;
            else
                newValue = current;

            if (newValue < m_inputType->minimum())
                newValue = m_inputType->minimum();
            if (newValue > m_inputType->maximum())
                newValue = m_inputType->maximum();

            setValueAsNumber(newValue, ec);
            current = newValue;
            if (n > 1)
                applyStep(n - 1, ec);
            else if (n < -1)
                applyStep(n + 1, ec);
        } else
            applyStep(n, ec);
    }

    if (currentStringValue != value()) {
        if (renderer() && renderer()->isTextField())
            toRenderTextControl(renderer())->setChangedSinceLastChangeEvent(true);
        if (m_inputType->isRangeControl())
            dispatchFormControlChangeEvent();
        else
            dispatchEvent(Event::create(eventNames().inputEvent, true, false));
    }
}

#if ENABLE(WCSS)
void HTMLInputElement::setWapInputFormat(String& mask)
{
    String validateMask = validateInputMask(m_data, mask);
    if (!validateMask.isEmpty())
        m_data.setInputFormatMask(validateMask);
}
#endif

#if ENABLE(INPUT_SPEECH)
bool HTMLInputElement::isSpeechEnabled() const
{
    switch (deprecatedInputType()) {
    // FIXME: Add support for RANGE, EMAIL, URL, COLOR and DATE/TIME input types.
    case NUMBER:
    case PASSWORD:
    case SEARCH:
    case TELEPHONE:
    case TEXT:
        return RuntimeEnabledFeatures::speechInputEnabled() && hasAttribute(webkitspeechAttr);
    case BUTTON:
    case CHECKBOX:
    case COLOR:
    case DATE:
    case DATETIME:
    case DATETIMELOCAL:
    case EMAIL:
    case FILE:
    case HIDDEN:
    case IMAGE:
    case ISINDEX:
    case MONTH:
    case RADIO:
    case RANGE:
    case RESET:
    case SUBMIT:
    case TIME:
    case URL:
    case WEEK:
        return false;
    }
    return false;
}

#endif

} // namespace
