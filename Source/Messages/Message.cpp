#include <Urho3D/Urho3DAll.h>
#include "Message.h"
#include "../Audio/AudioManagerDefs.h"
#include "../MyEvents.h"

#ifdef MessageBox
#undef MessageBox
#endif

Message::Message(Context* context) :
    Object(context)
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
    String title = eventData["Title"].GetString();
    String message = eventData["Message"].GetString();

    //new Urho3D::MessageBox(context_, message, title);
}
