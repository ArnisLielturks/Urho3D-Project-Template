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

protected:
    virtual void Init();

    /**
     * Handle ShowMessage event
     */
    void HandleShowMessage(StringHash eventType, VariantMap& eventData);

    /**
     * Display message with animation effects
     */
    void HandleUpdate(StringHash eventType, VariantMap& eventData);

private:

    void SubscribeToEvents();

    /**
     * Message title
     */
    String _title;

    /**
     * Message content
     */
    String _message;

    /**
     * Timestep for message animation
     */
    float _messageTime;
};