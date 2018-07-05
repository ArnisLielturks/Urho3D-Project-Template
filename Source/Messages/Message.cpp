#include <Urho3D/Urho3DAll.h>
#include "Message.h"

/// Construct.
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

void Message::Create()
{
    if (_baseElement) {
        URHO3D_LOGERROR("Another pop-up message is already active");
        return;
    }
    UI* ui = GetSubsystem<UI>();

    //////////////
    _baseElement = ui->GetRoot()->CreateChild("Menu");
    File startButtonFile(context_, GetSubsystem<FileSystem>()->GetProgramDir() + "Data/UI/Message.xml", FILE_READ);
    _baseElement->LoadXML(startButtonFile);
    _okButton = static_cast<Button*>(_baseElement->GetChild("MessageOkButton", true));
    _title = static_cast<Text*>(_baseElement->GetChild("MessageTitle", true));
    _message = static_cast<Text*>(_baseElement->GetChild("MessageBody", true));
    SubscribeToEvents();
}

void Message::SubscribeToEvents()
{
    SubscribeToEvent(_okButton, E_RELEASED, URHO3D_HANDLER(Message, HandleOkButton));
    SubscribeToEvent("ShowAlertMessage", URHO3D_HANDLER(Message, HandleShowMessage));
}

void Message::HandleShowMessage(StringHash eventType, VariantMap& eventData)
{
    Create();
    String title = eventData["Title"].GetString();
    String message = eventData["Message"].GetString();
    _title->SetText(title);
    _message->SetText(message);
}

void Message::Dispose()
{
}

void Message::HandleOkButton(StringHash eventType, VariantMap& eventData)
{
    _baseElement->SetVisible(false);
    _baseElement->Remove();
    _baseElement = nullptr;
}