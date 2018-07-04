#include <Urho3D/Urho3DAll.h>
#include "SettingsWindow.h"
#include "../../MyEvents.h"

/// Construct.
SettingsWindow::SettingsWindow(Context* context) :
    BaseWindow(context, IntVector2(300, 300))
{
    Init();
}

SettingsWindow::~SettingsWindow()
{
}

void SettingsWindow::Init()
{
    Create();

    SubscribeToEvents();
}

void SettingsWindow::Create()
{
    UI* ui = GetSubsystem<UI>();

    _closeButton = _base->CreateChild<Button>();
    _closeButton->SetSize(IntVector2(200, 50));
    _closeButton->SetPosition(IntVector2(50, 50));
    _closeButton->SetStyleAuto();

    _closeButton->SetAlignment(HA_LEFT, VA_TOP);

    Text* text = _closeButton->CreateChild<Text>();
    text->SetText("Close me");
    text->SetStyleAuto();
    text->SetAlignment(HA_CENTER, VA_CENTER);
}

void SettingsWindow::SubscribeToEvents()
{
    SubscribeToEvent(_closeButton, E_RELEASED, URHO3D_HANDLER(SettingsWindow, HandleClose));
}

void SettingsWindow::HandleClose(StringHash eventType, VariantMap& eventData)
{
    VariantMap data;
    data["Name"] = "SettingsWindow";
    SendEvent(MyEvents::E_CLOSE_WINDOW, data);
}