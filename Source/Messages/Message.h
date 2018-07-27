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

    bool Create();

    void HandleShowMessage(StringHash eventType, VariantMap& eventData);

    void HandleUpdate(StringHash eventType, VariantMap& eventData);

protected:
    virtual void Init();

private:

    void SubscribeToEvents();

    String _title;
    String _message;
    float _messageTime;
};