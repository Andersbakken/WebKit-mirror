#ifndef InspectorClientNetflix_h__
#define InspectorClientNetflix_h__

#include "InspectorClient.h"

namespace WebKit {

class InspectorClientNetflix : public WebCore::InspectorClient
{
public:
    InspectorClientNetflix();
    virtual ~InspectorClientNetflix();

    virtual void inspectorDestroyed() { }

    virtual void openInspectorFrontend(WebCore::InspectorController*) { }

    virtual void highlight(WebCore::Node*) { }
    virtual void hideHighlight() { }

    virtual void populateSetting(const WTF::String&, WTF::String*) { }
    virtual void storeSetting(const WTF::String&, const WTF::String&) { }

    virtual bool sendMessageToFrontend(const WTF::String& message) { return false; }
};

}

#endif // InspectorClientNetflix_h__
