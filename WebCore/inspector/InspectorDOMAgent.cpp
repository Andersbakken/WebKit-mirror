/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
 * Copyright (C) 2009 Google Inc. All rights reserved.
 * Copyright (C) 2009 Joseph Pecoraro
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
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

#include "config.h"
#include "InspectorDOMAgent.h"

#if ENABLE(INSPECTOR)

#include "Attr.h"
#include "CSSComputedStyleDeclaration.h"
#include "CSSMutableStyleDeclaration.h"
#include "CSSPropertyNames.h"
#include "CSSPropertySourceData.h"
#include "CSSRule.h"
#include "CSSRuleList.h"
#include "CSSStyleRule.h"
#include "CSSStyleSelector.h"
#include "CSSStyleSheet.h"
#include "CharacterData.h"
#include "ContainerNode.h"
#include "Cookie.h"
#include "CookieJar.h"
#include "DOMWindow.h"
#include "Document.h"
#include "DocumentType.h"
#include "Event.h"
#include "EventContext.h"
#include "EventListener.h"
#include "EventNames.h"
#include "EventTarget.h"
#include "Frame.h"
#include "FrameTree.h"
#include "HTMLElement.h"
#include "HTMLFrameOwnerElement.h"
#include "InspectorFrontend.h"
#include "MutationEvent.h"
#include "Node.h"
#include "NodeList.h"
#include "Pasteboard.h"
#include "PlatformString.h"
#include "RenderStyle.h"
#include "RenderStyleConstants.h"
#include "ScriptEventListener.h"
#include "StyleSheetList.h"
#include "Text.h"

#if ENABLE(XPATH)
#include "XPathResult.h"
#endif

#include "markup.h"

#include <wtf/text/CString.h>
#include <wtf/text/StringConcatenate.h>
#include <wtf/HashSet.h>
#include <wtf/ListHashSet.h>
#include <wtf/OwnPtr.h>
#include <wtf/Vector.h>
#include <wtf/text/AtomicString.h>

namespace WebCore {

class MatchJob {
public:
    virtual void match(ListHashSet<Node*>& resultCollector) = 0;
    virtual ~MatchJob() { }

protected:
    MatchJob(Document* document, const String& query)
        : m_document(document)
        , m_query(query) { }

    void addNodesToResults(PassRefPtr<NodeList> nodes, ListHashSet<Node*>& resultCollector)
    {
        for (unsigned i = 0; nodes && i < nodes->length(); ++i)
            resultCollector.add(nodes->item(i));
    }

    RefPtr<Document> m_document;
    String m_query;
};

namespace {

class MatchExactIdJob : public WebCore::MatchJob {
public:
    MatchExactIdJob(Document* document, const String& query) : WebCore::MatchJob(document, query) { }
    virtual ~MatchExactIdJob() { }

protected:
    virtual void match(ListHashSet<Node*>& resultCollector)
    {
        if (m_query.isEmpty())
            return;

        Element* element = m_document->getElementById(m_query);
        if (element)
            resultCollector.add(element);
    }
};

class MatchExactClassNamesJob : public WebCore::MatchJob {
public:
    MatchExactClassNamesJob(Document* document, const String& query) : WebCore::MatchJob(document, query) { }
    virtual ~MatchExactClassNamesJob() { }

    virtual void match(ListHashSet<Node*>& resultCollector)
    {
        if (!m_query.isEmpty())
            addNodesToResults(m_document->getElementsByClassName(m_query), resultCollector);
    }
};

class MatchExactTagNamesJob : public WebCore::MatchJob {
public:
    MatchExactTagNamesJob(Document* document, const String& query) : WebCore::MatchJob(document, query) { }
    virtual ~MatchExactTagNamesJob() { }

    virtual void match(ListHashSet<Node*>& resultCollector)
    {
        if (!m_query.isEmpty())
            addNodesToResults(m_document->getElementsByName(m_query), resultCollector);
    }
};

class MatchQuerySelectorAllJob : public WebCore::MatchJob {
public:
    MatchQuerySelectorAllJob(Document* document, const String& query) : WebCore::MatchJob(document, query) { }
    virtual ~MatchQuerySelectorAllJob() { }

    virtual void match(ListHashSet<Node*>& resultCollector)
    {
        if (m_query.isEmpty())
            return;

        ExceptionCode ec = 0;
        RefPtr<NodeList> list = m_document->querySelectorAll(m_query, ec);
        if (!ec)
            addNodesToResults(list, resultCollector);
    }
};

class MatchXPathJob : public WebCore::MatchJob {
public:
    MatchXPathJob(Document* document, const String& query) : WebCore::MatchJob(document, query) { }
    virtual ~MatchXPathJob() { }

    virtual void match(ListHashSet<Node*>& resultCollector)
    {
#if ENABLE(XPATH)
        if (m_query.isEmpty())
            return;

        ExceptionCode ec = 0;
        RefPtr<XPathResult> result = m_document->evaluate(m_query, m_document.get(), 0, XPathResult::ORDERED_NODE_SNAPSHOT_TYPE, 0, ec);
        if (ec || !result)
            return;

        unsigned long size = result->snapshotLength(ec);
        for (unsigned long i = 0; !ec && i < size; ++i) {
            Node* node = result->snapshotItem(i, ec);
            if (!ec)
                resultCollector.add(node);
        }
#else
        UNUSED_PARAM(resultCollector);
#endif
    }
};

class MatchPlainTextJob : public MatchXPathJob {
public:
    MatchPlainTextJob(Document* document, const String& query) : MatchXPathJob(document, query)
    {
        m_query = "//text()[contains(., '" + m_query + "')] | //comment()[contains(., '" + m_query + "')]";
    }
    virtual ~MatchPlainTextJob() { }
};

enum DOMBreakpointType {
    SubtreeModified = 0,
    AttributeModified,
    NodeRemoved,
    DOMBreakpointTypesCount
};

const uint32_t inheritableDOMBreakpointTypesMask = (1 << SubtreeModified);
const int domBreakpointDerivedTypeShift = 16;

}

InspectorDOMAgent::InspectorDOMAgent(InspectorFrontend* frontend)
    : EventListener(InspectorDOMAgentType)
    , m_frontend(frontend)
    , m_domListener(0)
    , m_lastNodeId(1)
    , m_matchJobsTimer(this, &InspectorDOMAgent::onMatchJobsTimer)
{
}

InspectorDOMAgent::~InspectorDOMAgent()
{
    reset();
}

void InspectorDOMAgent::reset()
{
    searchCanceled();
    discardBindings();

    ListHashSet<RefPtr<Document> > copy = m_documents;
    for (ListHashSet<RefPtr<Document> >::iterator it = copy.begin(); it != copy.end(); ++it)
        stopListening((*it).get());

    ASSERT(!m_documents.size());
}

void InspectorDOMAgent::setDOMListener(DOMListener* listener)
{
    m_domListener = listener;
}

void InspectorDOMAgent::setDocument(Document* doc)
{
    if (doc == mainFrameDocument())
        return;

    reset();

    if (doc) {
        startListening(doc);
        if (doc->documentElement())
            pushDocumentToFrontend();
    } else
        m_frontend->setDocument(InspectorValue::null());
}

void InspectorDOMAgent::releaseDanglingNodes()
{
    deleteAllValues(m_danglingNodeToIdMaps);
    m_danglingNodeToIdMaps.clear();
}

void InspectorDOMAgent::startListening(Document* doc)
{
    if (m_documents.contains(doc))
        return;

    doc->addEventListener(eventNames().DOMContentLoadedEvent, this, false);
    doc->addEventListener(eventNames().loadEvent, this, true);
    m_documents.add(doc);
}

void InspectorDOMAgent::stopListening(Document* doc)
{
    if (!m_documents.contains(doc))
        return;

    doc->removeEventListener(eventNames().DOMContentLoadedEvent, this, false);
    doc->removeEventListener(eventNames().loadEvent, this, true);
    m_documents.remove(doc);
}

void InspectorDOMAgent::handleEvent(ScriptExecutionContext*, Event* event)
{
    AtomicString type = event->type();
    Node* node = event->target()->toNode();

    if (type == eventNames().DOMContentLoadedEvent) {
        // Re-push document once it is loaded.
        discardBindings();
        pushDocumentToFrontend();
    } else if (type == eventNames().loadEvent) {
        long frameOwnerId = m_documentNodeToIdMap.get(node);
        if (!frameOwnerId)
            return;

        if (!m_childrenRequested.contains(frameOwnerId)) {
            // No children are mapped yet -> only notify on changes of hasChildren.
            m_frontend->childNodeCountUpdated(frameOwnerId, innerChildNodeCount(node));
        } else {
            // Re-add frame owner element together with its new children.
            long parentId = m_documentNodeToIdMap.get(innerParentNode(node));
            m_frontend->childNodeRemoved(parentId, frameOwnerId);
            RefPtr<InspectorObject> value = buildObjectForNode(node, 0, &m_documentNodeToIdMap);
            Node* previousSibling = innerPreviousSibling(node);
            long prevId = previousSibling ? m_documentNodeToIdMap.get(previousSibling) : 0;
            m_frontend->childNodeInserted(parentId, prevId, value.release());
            // Invalidate children requested flag for the element.
            m_childrenRequested.remove(m_childrenRequested.find(frameOwnerId));
        }
    }
}

long InspectorDOMAgent::bind(Node* node, NodeToIdMap* nodesMap)
{
    long id = nodesMap->get(node);
    if (id)
        return id;
    id = m_lastNodeId++;
    nodesMap->set(node, id);
    m_idToNode.set(id, node);
    m_idToNodesMap.set(id, nodesMap);
    return id;
}

void InspectorDOMAgent::unbind(Node* node, NodeToIdMap* nodesMap)
{
    if (node->isFrameOwnerElement()) {
        const HTMLFrameOwnerElement* frameOwner = static_cast<const HTMLFrameOwnerElement*>(node);
        stopListening(frameOwner->contentDocument());
        if (m_domListener)
            m_domListener->didRemoveDocument(frameOwner->contentDocument());
    }

    long id = nodesMap->get(node);
    if (!id)
        return;
    m_idToNode.remove(id);
    nodesMap->remove(node);
    bool childrenRequested = m_childrenRequested.contains(id);
    if (childrenRequested) {
        // Unbind subtree known to client recursively.
        m_childrenRequested.remove(id);
        Node* child = innerFirstChild(node);
        while (child) {
            unbind(child, nodesMap);
            child = innerNextSibling(child);
        }
    }
}

bool InspectorDOMAgent::pushDocumentToFrontend()
{
    Document* document = mainFrameDocument();
    if (!document)
        return false;
    if (!m_documentNodeToIdMap.contains(document))
        m_frontend->setDocument(buildObjectForNode(document, 2, &m_documentNodeToIdMap));
    return true;
}

void InspectorDOMAgent::pushChildNodesToFrontend(long nodeId)
{
    Node* node = nodeForId(nodeId);
    if (!node || (node->nodeType() != Node::ELEMENT_NODE && node->nodeType() != Node::DOCUMENT_NODE && node->nodeType() != Node::DOCUMENT_FRAGMENT_NODE))
        return;
    if (m_childrenRequested.contains(nodeId))
        return;

    NodeToIdMap* nodeMap = m_idToNodesMap.get(nodeId);
    RefPtr<InspectorArray> children = buildArrayForContainerChildren(node, 1, nodeMap);
    m_childrenRequested.add(nodeId);
    m_frontend->setChildNodes(nodeId, children.release());
}

long InspectorDOMAgent::inspectedNode(unsigned long num)
{
    if (num < m_inspectedNodes.size())
        return m_inspectedNodes[num];
    return 0;
}

void InspectorDOMAgent::discardBindings()
{
    m_documentNodeToIdMap.clear();
    m_idToNode.clear();
    releaseDanglingNodes();
    m_childrenRequested.clear();
    m_inspectedNodes.clear();
    m_breakpoints.clear();
}

Node* InspectorDOMAgent::nodeForId(long id)
{
    if (!id)
        return 0;

    HashMap<long, Node*>::iterator it = m_idToNode.find(id);
    if (it != m_idToNode.end())
        return it->second;
    return 0;
}

void InspectorDOMAgent::getChildNodes(long nodeId)
{
    pushChildNodesToFrontend(nodeId);
}

long InspectorDOMAgent::pushNodePathToFrontend(Node* nodeToPush)
{
    ASSERT(nodeToPush);  // Invalid input

    // If we are sending information to the client that is currently being created. Send root node first.
    if (!pushDocumentToFrontend())
        return 0;

    // Return id in case the node is known.
    long result = m_documentNodeToIdMap.get(nodeToPush);
    if (result)
        return result;

    Node* node = nodeToPush;
    Vector<Node*> path;
    NodeToIdMap* danglingMap = 0;
    while (true) {
        Node* parent = innerParentNode(node);
        if (!parent) {
            // Node being pushed is detached -> push subtree root.
            danglingMap = new NodeToIdMap();
            m_danglingNodeToIdMaps.append(danglingMap);
            m_frontend->setDetachedRoot(buildObjectForNode(node, 0, danglingMap));
            break;
        } else {
            path.append(parent);
            if (m_documentNodeToIdMap.get(parent))
                break;
            else
                node = parent;
        }
    }

    NodeToIdMap* map = danglingMap ? danglingMap : &m_documentNodeToIdMap;
    for (int i = path.size() - 1; i >= 0; --i) {
        long nodeId = map->get(path.at(i));
        ASSERT(nodeId);
        pushChildNodesToFrontend(nodeId);
    }
    return map->get(nodeToPush);
}

void InspectorDOMAgent::setAttribute(long elementId, const String& name, const String& value, bool* success)
{
    Node* node = nodeForId(elementId);
    if (node && (node->nodeType() == Node::ELEMENT_NODE)) {
        Element* element = static_cast<Element*>(node);
        ExceptionCode ec = 0;
        element->setAttribute(name, value, ec);
        *success = !ec;
    }
}

void InspectorDOMAgent::removeAttribute(long elementId, const String& name, bool* success)
{
    Node* node = nodeForId(elementId);
    if (node && (node->nodeType() == Node::ELEMENT_NODE)) {
        Element* element = static_cast<Element*>(node);
        ExceptionCode ec = 0;
        element->removeAttribute(name, ec);
        *success = !ec;
    }
}

void InspectorDOMAgent::removeNode(long nodeId, long* outNodeId)
{
    Node* node = nodeForId(nodeId);
    if (!node)
        return;

    ContainerNode* parentNode = node->parentNode();
    if (!parentNode)
        return;

    ExceptionCode ec = 0;
    parentNode->removeChild(node, ec);
    if (ec)
        return;

    *outNodeId = nodeId;
}

void InspectorDOMAgent::changeTagName(long nodeId, const String& tagName, long* newId)
{
    Node* oldNode = nodeForId(nodeId);
    if (!oldNode || !oldNode->isElementNode())
        return;

    ExceptionCode ec = 0;
    RefPtr<Element> newElem = oldNode->document()->createElement(tagName, ec);
    if (ec)
        return;

    // Copy over the original node's attributes.
    Element* oldElem = static_cast<Element*>(oldNode);
    newElem->copyNonAttributeProperties(oldElem);
    if (oldElem->attributes())
        newElem->attributes()->setAttributes(*(oldElem->attributes(true)));

    // Copy over the original node's children.
    Node* child;
    while ((child = oldNode->firstChild()))
        newElem->appendChild(child, ec);

    // Replace the old node with the new node
    ContainerNode* parent = oldNode->parentNode();
    parent->insertBefore(newElem, oldNode->nextSibling(), ec);
    parent->removeChild(oldNode, ec);

    if (ec)
        return;

    *newId = pushNodePathToFrontend(newElem.get());
    if (m_childrenRequested.contains(nodeId))
        pushChildNodesToFrontend(*newId);
}

void InspectorDOMAgent::getOuterHTML(long nodeId, WTF::String* outerHTML)
{
    Node* node = nodeForId(nodeId);
    if (!node || !node->isHTMLElement())
        return;

    *outerHTML = static_cast<HTMLElement*>(node)->outerHTML();
}

void InspectorDOMAgent::setOuterHTML(long nodeId, const String& outerHTML, long* newId)
{
    Node* node = nodeForId(nodeId);
    if (!node || !node->isHTMLElement())
        return;

    bool requiresTotalUpdate = node->nodeName() == "HTML" || node->nodeName() == "BODY" || node->nodeName() == "HEAD";

    bool childrenRequested = m_childrenRequested.contains(nodeId);
    Node* previousSibling = node->previousSibling();
    ContainerNode* parentNode = node->parentNode();

    HTMLElement* htmlElement = static_cast<HTMLElement*>(node);
    ExceptionCode ec = 0;
    htmlElement->setOuterHTML(outerHTML, ec);
    if (ec)
        return;

    if (requiresTotalUpdate) {
        Document* document = mainFrameDocument();
        reset();
        setDocument(document);
        *newId = 0;
        return;
    }

    Node* newNode = previousSibling ? previousSibling->nextSibling() : parentNode->firstChild();
    if (!newNode) {
        // The only child node has been deleted.
        *newId = 0;
        return;
    }

    *newId = pushNodePathToFrontend(newNode);
    if (childrenRequested)
        pushChildNodesToFrontend(*newId);
}

void InspectorDOMAgent::setTextNodeValue(long nodeId, const String& value, bool* success)
{
    Node* node = nodeForId(nodeId);
    if (node && (node->nodeType() == Node::TEXT_NODE)) {
        Text* text_node = static_cast<Text*>(node);
        ExceptionCode ec = 0;
        text_node->replaceWholeText(value, ec);
        *success = !ec;
    }
}

void InspectorDOMAgent::getEventListenersForNode(long nodeId, long* outNodeId, RefPtr<InspectorArray>* listenersArray)
{
    Node* node = nodeForId(nodeId);
    *outNodeId = nodeId;
    EventTargetData* d;

    // Quick break if a null node or no listeners at all
    if (!node || !(d = node->eventTargetData()))
        return;

    // Get the list of event types this Node is concerned with
    Vector<AtomicString> eventTypes;
    const EventListenerMap& listenerMap = d->eventListenerMap;
    EventListenerMap::const_iterator end = listenerMap.end();
    for (EventListenerMap::const_iterator iter = listenerMap.begin(); iter != end; ++iter)
        eventTypes.append(iter->first);

    // Quick break if no useful listeners
    size_t eventTypesLength = eventTypes.size();
    if (!eventTypesLength)
        return;

    // The Node's Event Ancestors (not including self)
    Vector<EventContext> ancestors;
    node->getEventAncestors(ancestors, node);

    // Nodes and their Listeners for the concerned event types (order is top to bottom)
    Vector<EventListenerInfo> eventInformation;
    for (size_t i = ancestors.size(); i; --i) {
        Node* ancestor = ancestors[i - 1].node();
        for (size_t j = 0; j < eventTypesLength; ++j) {
            AtomicString& type = eventTypes[j];
            if (ancestor->hasEventListeners(type))
                eventInformation.append(EventListenerInfo(static_cast<Node*>(ancestor), type, ancestor->getEventListeners(type)));
        }
    }

    // Insert the Current Node at the end of that list (last in capturing, first in bubbling)
    for (size_t i = 0; i < eventTypesLength; ++i) {
        const AtomicString& type = eventTypes[i];
        eventInformation.append(EventListenerInfo(node, type, node->getEventListeners(type)));
    }

    // Get Capturing Listeners (in this order)
    size_t eventInformationLength = eventInformation.size();
    for (size_t i = 0; i < eventInformationLength; ++i) {
        const EventListenerInfo& info = eventInformation[i];
        const EventListenerVector& vector = info.eventListenerVector;
        for (size_t j = 0; j < vector.size(); ++j) {
            const RegisteredEventListener& listener = vector[j];
            if (listener.useCapture)
                (*listenersArray)->pushObject(buildObjectForEventListener(listener, info.eventType, info.node));
        }
    }

    // Get Bubbling Listeners (reverse order)
    for (size_t i = eventInformationLength; i; --i) {
        const EventListenerInfo& info = eventInformation[i - 1];
        const EventListenerVector& vector = info.eventListenerVector;
        for (size_t j = 0; j < vector.size(); ++j) {
            const RegisteredEventListener& listener = vector[j];
            if (!listener.useCapture)
                (*listenersArray)->pushObject(buildObjectForEventListener(listener, info.eventType, info.node));
        }
    }
}

void InspectorDOMAgent::addInspectedNode(long nodeId)
{
    m_inspectedNodes.prepend(nodeId);
    while (m_inspectedNodes.size() > 5)
        m_inspectedNodes.removeLast();
}

void InspectorDOMAgent::performSearch(const String& whitespaceTrimmedQuery, bool runSynchronously)
{
    // FIXME: Few things are missing here:
    // 1) Search works with node granularity - number of matches within node is not calculated.
    // 2) There is no need to push all search results to the front-end at a time, pushing next / previous result
    //    is sufficient.

    unsigned queryLength = whitespaceTrimmedQuery.length();
    bool startTagFound = !whitespaceTrimmedQuery.find('<');
    bool endTagFound = whitespaceTrimmedQuery.reverseFind('>') + 1 == queryLength;

    String tagNameQuery = whitespaceTrimmedQuery;
    if (startTagFound || endTagFound)
        tagNameQuery = tagNameQuery.substring(startTagFound ? 1 : 0, endTagFound ? queryLength - 1 : queryLength);
    if (!Document::isValidName(tagNameQuery))
        tagNameQuery = "";

    String attributeNameQuery = whitespaceTrimmedQuery;
    if (!Document::isValidName(attributeNameQuery))
        attributeNameQuery = "";

    String escapedQuery = whitespaceTrimmedQuery;
    escapedQuery.replace("'", "\\'");
    String escapedTagNameQuery = tagNameQuery;
    escapedTagNameQuery.replace("'", "\\'");

    // Clear pending jobs.
    searchCanceled();

    // Find all frames, iframes and object elements to search their documents.
    for (Frame* frame = mainFrameDocument()->frame(); frame; frame = frame->tree()->traverseNext()) {
        Document* document = frame->document();
        if (!document)
            continue;

        if (!tagNameQuery.isEmpty() && startTagFound && endTagFound) {
            m_pendingMatchJobs.append(new MatchExactTagNamesJob(document, tagNameQuery));
            m_pendingMatchJobs.append(new MatchPlainTextJob(document, escapedQuery));
            continue;
        }

        if (!tagNameQuery.isEmpty() && startTagFound) {
            m_pendingMatchJobs.append(new MatchXPathJob(document, "//*[starts-with(name(), '" + escapedTagNameQuery + "')]"));
            m_pendingMatchJobs.append(new MatchPlainTextJob(document, escapedQuery));
            continue;
        }

        if (!tagNameQuery.isEmpty() && endTagFound) {
            // FIXME: we should have a matchEndOfTagNames search function if endTagFound is true but not startTagFound.
            // This requires ends-with() support in XPath, WebKit only supports starts-with() and contains().
            m_pendingMatchJobs.append(new MatchXPathJob(document, "//*[contains(name(), '" + escapedTagNameQuery + "')]"));
            m_pendingMatchJobs.append(new MatchPlainTextJob(document, escapedQuery));
            continue;
        }

        bool matchesEveryNode = whitespaceTrimmedQuery == "//*" || whitespaceTrimmedQuery == "*";
        if (matchesEveryNode) {
            // These queries will match every node. Matching everything isn't useful and can be slow for large pages,
            // so limit the search functions list to plain text and attribute matching for these.
            m_pendingMatchJobs.append(new MatchXPathJob(document, "//*[contains(@*, '" + escapedQuery + "')]"));
            m_pendingMatchJobs.append(new MatchPlainTextJob(document, escapedQuery));
            continue;
        }

        m_pendingMatchJobs.append(new MatchExactIdJob(document, whitespaceTrimmedQuery));
        m_pendingMatchJobs.append(new MatchExactClassNamesJob(document, whitespaceTrimmedQuery));
        m_pendingMatchJobs.append(new MatchExactTagNamesJob(document, tagNameQuery));
        m_pendingMatchJobs.append(new MatchQuerySelectorAllJob(document, "[" + attributeNameQuery + "]"));
        m_pendingMatchJobs.append(new MatchQuerySelectorAllJob(document, whitespaceTrimmedQuery));
        m_pendingMatchJobs.append(new MatchXPathJob(document, "//*[contains(@*, '" + escapedQuery + "')]"));
        if (!tagNameQuery.isEmpty())
            m_pendingMatchJobs.append(new MatchXPathJob(document, "//*[contains(name(), '" + escapedTagNameQuery + "')]"));
        m_pendingMatchJobs.append(new MatchPlainTextJob(document, escapedQuery));
        m_pendingMatchJobs.append(new MatchXPathJob(document, whitespaceTrimmedQuery));
    }

    if (runSynchronously) {
        // For tests.
        ListHashSet<Node*> resultCollector;
        for (Deque<MatchJob*>::iterator it = m_pendingMatchJobs.begin(); it != m_pendingMatchJobs.end(); ++it)
            (*it)->match(resultCollector);
        reportNodesAsSearchResults(resultCollector);
        searchCanceled();
        return;
    }
    m_matchJobsTimer.startOneShot(0);
}

void InspectorDOMAgent::searchCanceled()
{
    if (m_matchJobsTimer.isActive())
        m_matchJobsTimer.stop();
    deleteAllValues(m_pendingMatchJobs);
    m_pendingMatchJobs.clear();
    m_searchResults.clear();
}

void InspectorDOMAgent::setDOMBreakpoint(long nodeId, long type)
{
    Node* node = nodeForId(nodeId);
    if (!node)
        return;

    uint32_t rootBit = 1 << type;
    m_breakpoints.set(node, m_breakpoints.get(node) | rootBit);
    if (rootBit & inheritableDOMBreakpointTypesMask) {
        for (Node* child = innerFirstChild(node); child; child = innerNextSibling(child))
            updateSubtreeBreakpoints(child, rootBit, true);
    }
}

void InspectorDOMAgent::removeDOMBreakpoint(long nodeId, long type)
{
    Node* node = nodeForId(nodeId);
    if (!node)
        return;

    uint32_t rootBit = 1 << type;
    uint32_t mask = m_breakpoints.get(node) & ~rootBit;
    if (mask)
        m_breakpoints.set(node, mask);
    else
        m_breakpoints.remove(node);

    if ((rootBit & inheritableDOMBreakpointTypesMask) && !(mask & (rootBit << domBreakpointDerivedTypeShift))) {
        for (Node* child = innerFirstChild(node); child; child = innerNextSibling(child))
            updateSubtreeBreakpoints(child, rootBit, false);
    }
}

bool InspectorDOMAgent::shouldBreakOnNodeInsertion(Node*, Node* parent, PassRefPtr<InspectorObject> details)
{
    if (hasBreakpoint(parent, SubtreeModified)) {
        descriptionForDOMEvent(parent, SubtreeModified, true, details);
        return true;
    }
    return false;
}

bool InspectorDOMAgent::shouldBreakOnNodeRemoval(Node* node, PassRefPtr<InspectorObject> details)
{
    if (hasBreakpoint(node, NodeRemoved)) {
        descriptionForDOMEvent(node, NodeRemoved, false, details);
        return true;
    }
    if (hasBreakpoint(innerParentNode(node), SubtreeModified)) {
        descriptionForDOMEvent(node, SubtreeModified, false, details);
        return true;
    }
    return false;
}

bool InspectorDOMAgent::shouldBreakOnAttributeModification(Element* element, PassRefPtr<InspectorObject> details)
{
    if (hasBreakpoint(element, AttributeModified)) {
        descriptionForDOMEvent(element, AttributeModified, false, details);
        return true;
    }
    return false;
}

void InspectorDOMAgent::descriptionForDOMEvent(Node* target, long breakpointType, bool insertion, PassRefPtr<InspectorObject> description)
{
    ASSERT(hasBreakpoint(target, breakpointType));

    Node* breakpointOwner = target;
    if ((1 << breakpointType) & inheritableDOMBreakpointTypesMask) {
        // For inheritable breakpoint types, target node isn't always the same as the node that owns a breakpoint.
        // Target node may be unknown to frontend, so we need to push it first.
        long targetNodeId = pushNodePathToFrontend(target);
        ASSERT(targetNodeId);
        description->setNumber("targetNodeId", targetNodeId);

        // Find breakpoint owner node.
        if (!insertion)
            breakpointOwner = innerParentNode(target);
        ASSERT(breakpointOwner);
        while (!(m_breakpoints.get(breakpointOwner) & (1 << breakpointType))) {
            breakpointOwner = innerParentNode(breakpointOwner);
            ASSERT(breakpointOwner);
        }

        if (breakpointType == SubtreeModified)
            description->setBoolean("insertion", insertion);
    }

    long breakpointOwnerNodeId = m_documentNodeToIdMap.get(breakpointOwner);
    ASSERT(breakpointOwnerNodeId);
    description->setNumber("nodeId", breakpointOwnerNodeId);
    description->setNumber("type", breakpointType);
}

String InspectorDOMAgent::documentURLString(Document* document) const
{
    if (!document || document->url().isNull())
        return "";
    return document->url().string();
}

PassRefPtr<InspectorObject> InspectorDOMAgent::buildObjectForNode(Node* node, int depth, NodeToIdMap* nodesMap)
{
    RefPtr<InspectorObject> value = InspectorObject::create();

    long id = bind(node, nodesMap);
    String nodeName;
    String localName;
    String nodeValue;

    switch (node->nodeType()) {
        case Node::TEXT_NODE:
        case Node::COMMENT_NODE:
        case Node::CDATA_SECTION_NODE:
            nodeValue = node->nodeValue();
            break;
        case Node::ATTRIBUTE_NODE:
            localName = node->localName();
            break;
        case Node::DOCUMENT_FRAGMENT_NODE:
            break;
        case Node::DOCUMENT_NODE:
        case Node::ELEMENT_NODE:
        default:
            nodeName = node->nodeName();
            localName = node->localName();
            break;
    }

    value->setNumber("id", id);
    value->setNumber("nodeType", node->nodeType());
    value->setString("nodeName", nodeName);
    value->setString("localName", localName);
    value->setString("nodeValue", nodeValue);

    if (node->nodeType() == Node::ELEMENT_NODE || node->nodeType() == Node::DOCUMENT_NODE || node->nodeType() == Node::DOCUMENT_FRAGMENT_NODE) {
        int nodeCount = innerChildNodeCount(node);
        value->setNumber("childNodeCount", nodeCount);
        RefPtr<InspectorArray> children = buildArrayForContainerChildren(node, depth, nodesMap);
        if (children->length() > 0)
            value->setArray("children", children.release());

        if (node->nodeType() == Node::ELEMENT_NODE) {
            Element* element = static_cast<Element*>(node);
            value->setArray("attributes", buildArrayForElementAttributes(element));
            if (node->isFrameOwnerElement()) {
                HTMLFrameOwnerElement* frameOwner = static_cast<HTMLFrameOwnerElement*>(node);
                value->setString("documentURL", documentURLString(frameOwner->contentDocument()));
            }
        } else if (node->nodeType() == Node::DOCUMENT_NODE) {
            Document* document = static_cast<Document*>(node);
            value->setString("documentURL", documentURLString(document));
        }
    } else if (node->nodeType() == Node::DOCUMENT_TYPE_NODE) {
        DocumentType* docType = static_cast<DocumentType*>(node);
        value->setString("publicId", docType->publicId());
        value->setString("systemId", docType->systemId());
        value->setString("internalSubset", docType->internalSubset());
    } else if (node->nodeType() == Node::ATTRIBUTE_NODE) {
        Attr* attribute = static_cast<Attr*>(node);
        value->setString("name", attribute->name());
        value->setString("value", attribute->value());
    }
    return value.release();
}

PassRefPtr<InspectorArray> InspectorDOMAgent::buildArrayForElementAttributes(Element* element)
{
    RefPtr<InspectorArray> attributesValue = InspectorArray::create();
    // Go through all attributes and serialize them.
    const NamedNodeMap* attrMap = element->attributes(true);
    if (!attrMap)
        return attributesValue.release();
    unsigned numAttrs = attrMap->length();
    for (unsigned i = 0; i < numAttrs; ++i) {
        // Add attribute pair
        const Attribute *attribute = attrMap->attributeItem(i);
        attributesValue->pushString(attribute->name().toString());
        attributesValue->pushString(attribute->value());
    }
    return attributesValue.release();
}

PassRefPtr<InspectorArray> InspectorDOMAgent::buildArrayForContainerChildren(Node* container, int depth, NodeToIdMap* nodesMap)
{
    RefPtr<InspectorArray> children = InspectorArray::create();
    if (depth == 0) {
        // Special case the_only text child.
        if (innerChildNodeCount(container) == 1) {
            Node *child = innerFirstChild(container);
            if (child->nodeType() == Node::TEXT_NODE)
                children->pushObject(buildObjectForNode(child, 0, nodesMap));
        }
        return children.release();
    } else if (depth > 0) {
        depth--;
    }

    for (Node *child = innerFirstChild(container); child; child = innerNextSibling(child))
        children->pushObject(buildObjectForNode(child, depth, nodesMap));
    return children.release();
}

PassRefPtr<InspectorObject> InspectorDOMAgent::buildObjectForEventListener(const RegisteredEventListener& registeredEventListener, const AtomicString& eventType, Node* node)
{
    RefPtr<EventListener> eventListener = registeredEventListener.listener;
    RefPtr<InspectorObject> value = InspectorObject::create();
    value->setString("type", eventType);
    value->setBoolean("useCapture", registeredEventListener.useCapture);
    value->setBoolean("isAttribute", eventListener->isAttribute());
    value->setNumber("nodeId", pushNodePathToFrontend(node));
    value->setString("listenerBody", eventListenerHandlerBody(node->document(), eventListener.get()));
    String sourceName;
    int lineNumber;
    if (eventListenerHandlerLocation(node->document(), eventListener.get(), sourceName, lineNumber)) {
        value->setString("sourceName", sourceName);
        value->setNumber("lineNumber", lineNumber);
    }
    return value.release();
}

Node* InspectorDOMAgent::innerFirstChild(Node* node)
{
    if (node->isFrameOwnerElement()) {
        HTMLFrameOwnerElement* frameOwner = static_cast<HTMLFrameOwnerElement*>(node);
        Document* doc = frameOwner->contentDocument();
        if (doc) {
            startListening(doc);
            return doc->firstChild();
        }
    }
    node = node->firstChild();
    while (isWhitespace(node))
        node = node->nextSibling();
    return node;
}

Node* InspectorDOMAgent::innerNextSibling(Node* node)
{
    do {
        node = node->nextSibling();
    } while (isWhitespace(node));
    return node;
}

Node* InspectorDOMAgent::innerPreviousSibling(Node* node)
{
    do {
        node = node->previousSibling();
    } while (isWhitespace(node));
    return node;
}

unsigned InspectorDOMAgent::innerChildNodeCount(Node* node)
{
    unsigned count = 0;
    Node* child = innerFirstChild(node);
    while (child) {
        count++;
        child = innerNextSibling(child);
    }
    return count;
}

Node* InspectorDOMAgent::innerParentNode(Node* node)
{
    ContainerNode* parent = node->parentNode();
    if (parent && parent->isDocumentNode())
        return static_cast<Document*>(parent)->ownerElement();
    return parent;
}

bool InspectorDOMAgent::isWhitespace(Node* node)
{
    //TODO: pull ignoreWhitespace setting from the frontend and use here.
    return node && node->nodeType() == Node::TEXT_NODE && node->nodeValue().stripWhiteSpace().length() == 0;
}

Document* InspectorDOMAgent::mainFrameDocument() const
{
    ListHashSet<RefPtr<Document> >::const_iterator it = m_documents.begin();
    if (it != m_documents.end())
        return it->get();
    return 0;
}

bool InspectorDOMAgent::operator==(const EventListener& listener)
{
    if (const InspectorDOMAgent* inspectorDOMAgentListener = InspectorDOMAgent::cast(&listener))
        return mainFrameDocument() == inspectorDOMAgentListener->mainFrameDocument();
    return false;
}

void InspectorDOMAgent::didInsertDOMNode(Node* node)
{
    if (isWhitespace(node))
        return;

    if (m_breakpoints.size()) {
        uint32_t mask = m_breakpoints.get(innerParentNode(node));
        uint32_t inheritableTypesMask = (mask | (mask >> domBreakpointDerivedTypeShift)) & inheritableDOMBreakpointTypesMask;
        if (inheritableTypesMask)
            updateSubtreeBreakpoints(node, inheritableTypesMask, true);
    }

    // We could be attaching existing subtree. Forget the bindings.
    unbind(node, &m_documentNodeToIdMap);

    ContainerNode* parent = node->parentNode();
    long parentId = m_documentNodeToIdMap.get(parent);
    // Return if parent is not mapped yet.
    if (!parentId)
        return;

    if (!m_childrenRequested.contains(parentId)) {
        // No children are mapped yet -> only notify on changes of hasChildren.
        m_frontend->childNodeCountUpdated(parentId, innerChildNodeCount(parent));
    } else {
        // Children have been requested -> return value of a new child.
        Node* prevSibling = innerPreviousSibling(node);
        long prevId = prevSibling ? m_documentNodeToIdMap.get(prevSibling) : 0;
        RefPtr<InspectorObject> value = buildObjectForNode(node, 0, &m_documentNodeToIdMap);
        m_frontend->childNodeInserted(parentId, prevId, value.release());
    }
}

void InspectorDOMAgent::didRemoveDOMNode(Node* node)
{
    if (isWhitespace(node))
        return;

    if (m_breakpoints.size()) {
        // Remove subtree breakpoints.
        m_breakpoints.remove(node);
        Vector<Node*> stack(1, innerFirstChild(node));
        do {
            Node* node = stack.last();
            stack.removeLast();
            if (!node)
                continue;
            m_breakpoints.remove(node);
            stack.append(innerFirstChild(node));
            stack.append(innerNextSibling(node));
        } while (!stack.isEmpty());
    }

    ContainerNode* parent = node->parentNode();
    long parentId = m_documentNodeToIdMap.get(parent);
    // If parent is not mapped yet -> ignore the event.
    if (!parentId)
        return;

    if (m_domListener)
        m_domListener->didRemoveDOMNode(node);

    if (!m_childrenRequested.contains(parentId)) {
        // No children are mapped yet -> only notify on changes of hasChildren.
        if (innerChildNodeCount(parent) == 1)
            m_frontend->childNodeCountUpdated(parentId, 0);
    } else
        m_frontend->childNodeRemoved(parentId, m_documentNodeToIdMap.get(node));
    unbind(node, &m_documentNodeToIdMap);
}

void InspectorDOMAgent::didModifyDOMAttr(Element* element)
{
    long id = m_documentNodeToIdMap.get(element);
    // If node is not mapped yet -> ignore the event.
    if (!id)
        return;

    if (m_domListener)
        m_domListener->didModifyDOMAttr(element);

    m_frontend->attributesUpdated(id, buildArrayForElementAttributes(element));
}

void InspectorDOMAgent::characterDataModified(CharacterData* characterData)
{
    long id = m_documentNodeToIdMap.get(characterData);
    if (!id)
        return;
    m_frontend->characterDataModified(id, characterData->data());
}

bool InspectorDOMAgent::hasBreakpoint(Node* node, long type)
{
    uint32_t rootBit = 1 << type;
    uint32_t derivedBit = rootBit << domBreakpointDerivedTypeShift;
    return m_breakpoints.get(node) & (rootBit | derivedBit);
}

void InspectorDOMAgent::updateSubtreeBreakpoints(Node* node, uint32_t rootMask, bool set)
{
    uint32_t oldMask = m_breakpoints.get(node);
    uint32_t derivedMask = rootMask << domBreakpointDerivedTypeShift;
    uint32_t newMask = set ? oldMask | derivedMask : oldMask & ~derivedMask;
    if (newMask)
        m_breakpoints.set(node, newMask);
    else
        m_breakpoints.remove(node);

    uint32_t newRootMask = rootMask & ~newMask;
    if (!newRootMask)
        return;

    for (Node* child = innerFirstChild(node); child; child = innerNextSibling(child))
        updateSubtreeBreakpoints(child, newRootMask, set);
}

Node* InspectorDOMAgent::nodeForPath(const String& path)
{
    // The path is of form "1,HTML,2,BODY,1,DIV"
    Node* node = mainFrameDocument();
    if (!node)
        return 0;

    Vector<String> pathTokens;
    path.split(",", false, pathTokens);
    if (!pathTokens.size())
        return 0;
    for (size_t i = 0; i < pathTokens.size() - 1; i += 2) {
        bool success = true;
        unsigned childNumber = pathTokens[i].toUInt(&success);
        if (!success)
            return 0;
        if (childNumber >= innerChildNodeCount(node))
            return 0;

        Node* child = innerFirstChild(node);
        String childName = pathTokens[i + 1];
        for (size_t j = 0; child && j < childNumber; ++j)
            child = innerNextSibling(child);

        if (!child || child->nodeName() != childName)
            return 0;
        node = child;
    }
    return node;
}

PassRefPtr<InspectorArray> InspectorDOMAgent::toArray(const Vector<String>& data)
{
    RefPtr<InspectorArray> result = InspectorArray::create();
    for (unsigned i = 0; i < data.size(); ++i)
        result->pushString(data[i]);
    return result.release();
}

void InspectorDOMAgent::onMatchJobsTimer(Timer<InspectorDOMAgent>*)
{
    if (!m_pendingMatchJobs.size()) {
        searchCanceled();
        return;
    }

    ListHashSet<Node*> resultCollector;
    MatchJob* job = m_pendingMatchJobs.takeFirst();
    job->match(resultCollector);
    delete job;

    reportNodesAsSearchResults(resultCollector);

    m_matchJobsTimer.startOneShot(0.025);
}

void InspectorDOMAgent::reportNodesAsSearchResults(ListHashSet<Node*>& resultCollector)
{
    RefPtr<InspectorArray> nodeIds = InspectorArray::create();
    for (ListHashSet<Node*>::iterator it = resultCollector.begin(); it != resultCollector.end(); ++it) {
        if (m_searchResults.contains(*it))
            continue;
        m_searchResults.add(*it);
        nodeIds->pushNumber(static_cast<long long>(pushNodePathToFrontend(*it)));
    }
    m_frontend->addNodesToSearchResult(nodeIds.release());
}

void InspectorDOMAgent::copyNode(long nodeId)
{
    Node* node = nodeForId(nodeId);
    if (!node)
        return;
    String markup = createMarkup(node);
    Pasteboard::generalPasteboard()->writePlainText(markup);
}

void InspectorDOMAgent::pushNodeByPathToFrontend(const String& path, long* nodeId)
{
    if (Node* node = nodeForPath(path))
        *nodeId = pushNodePathToFrontend(node);
}

} // namespace WebCore

#endif // ENABLE(INSPECTOR)
