#include <Urho3D/Urho3DAll.h>
#include "Message.h"
#include "../Audio/AudioManagerDefs.h"
#include "../MyEvents.h"

/// Construct.
Message::Message(Context* context) :
    Object(context),
    _messageTime(0)
{
    Init();
}

Message::~Message()
{
}

void Message::Init()
{
    SubscribeToEvents();
}

void Message::SubscribeToEvents()
{
    SubscribeToEvent("ShowAlertMessage", URHO3D_HANDLER(Message, HandleShowMessage));
}

void Message::HandleShowMessage(StringHash eventType, VariantMap& eventData)
{
    _title = eventData["Title"].GetString();
    _message = eventData["Message"].GetString();
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Message, HandleUpdate));
    _messageTime = 0.0f;
}

void Message::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;
    float timeStep = eventData[P_TIMESTEP].GetFloat();
    _messageTime += timeStep * 200;
}