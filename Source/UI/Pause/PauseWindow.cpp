#include <Urho3D/Urho3DAll.h>
#include "PauseWindow.h"
#include "../../MyEvents.h"

/// Construct.
PauseWindow::PauseWindow(Context* context) :
    BaseWindow(context, IntVector2(300, 300))
{
    Init();
}

PauseWindow::~PauseWindow()
{
}

void PauseWindow::Init()
{
    Create();

    SubscribeToEvents();
}

void PauseWindow::Create()
{
    Input* input = GetSubsystem<Input>();
    if (!input->IsMouseVisible()) {
        input->SetMouseVisible(true);
    }

    UI* ui = GetSubsystem<UI>();

    {
        Text* text = _base->CreateChild<Text>();
        text->SetText("Pause");
        text->SetStyleAuto();
        text->SetColor(Color(0.2f, 0.8f, 0.2f));
        text->SetAlignment(HA_CENTER, VA_TOP);
        text->SetPosition(IntVector2(0, 10));
        text->SetFontSize(20);
    }
}

void PauseWindow::SubscribeToEvents()
{
    SubscribeToEvent(_closeButton, E_RELEASED, URHO3D_HANDLER(PauseWindow, HandleClose));
}

void PauseWindow::HandleClose(StringHash eventType, VariantMap& eventData)
{
    VariantMap data = GetEventDataMap();
    data["Name"] = "PauseWindow";
    SendEvent(MyEvents::E_CLOSE_WINDOW, data);
}