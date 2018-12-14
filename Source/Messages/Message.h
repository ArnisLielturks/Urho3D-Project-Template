#pragma once

#include <Urho3D/Urho3DAll.h>

using namespace Urho3D;

class Message : public Object
{
    URHO3D_OBJECT(Message, Object);

public:
    /// Construct.
    Message(Context* context);

    virtual ~Message();

private:
    void Init();

    /**
     * Handle ShowMessage event
     */
    void HandleShowMessage(StringHash eventType, VariantMap& eventData);

    /**
     * Subscribe to message events
     */
    void SubscribeToEvents();
};