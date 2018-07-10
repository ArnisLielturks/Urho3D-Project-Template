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

    _closeButton = CreateButton("X", IntVector2(-5, 5), IntVector2(20, 20), HA_RIGHT, VA_TOP);
    _exitButton = CreateButton("Exit game", IntVector2(0, -20), IntVector2(200, 30), HA_CENTER, VA_BOTTOM);

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

Button* PauseWindow::CreateButton(String name, IntVector2 position, IntVector2 size, HorizontalAlignment hAlign, VerticalAlignment vAlign)
{
    Button* button = _base->CreateChild<Button>();
    button->SetSize(size);
    button->SetPosition(position);
    button->SetStyleAuto();
    button->SetAlignment(hAlign, vAlign);

    Text* text = button->CreateChild<Text>();
    text->SetText(name);
    text->SetStyleAuto();
    text->SetAlignment(HA_CENTER, VA_CENTER);

    return button;
}

void PauseWindow::SubscribeToEvents()
{
    SubscribeToEvent(_closeButton, E_RELEASED, URHO3D_HANDLER(PauseWindow, HandleClose));
    SubscribeToEvent(_exitButton, E_RELEASED, URHO3D_HANDLER(PauseWindow, HandleExit));
}

void PauseWindow::HandleClose(StringHash eventType, VariantMap& eventData)
{
    VariantMap data = GetEventDataMap();
    data["Name"] = "PauseWindow";
    SendEvent(MyEvents::E_CLOSE_WINDOW, data);
}

void PauseWindow::HandleExit(StringHash eventType, VariantMap& eventData)
{
    VariantMap data = GetEventDataMap();;
    data["Name"] = "ExitGame";
    data["Message"] = "";
    SendEvent(MyEvents::E_SET_LEVEL, data);
}