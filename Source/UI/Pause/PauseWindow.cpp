#include <Urho3D/Urho3DAll.h>
#include "PauseWindow.h"
#include "../../MyEvents.h"

/// Construct.
PauseWindow::PauseWindow(Context* context) :
    BaseWindow(context, IntVector2(200, 200))
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

	_resumeButton = CreateButton("Resume", IntVector2(0, -100), IntVector2(150, 30), HA_CENTER, VA_BOTTOM);
	_menuButton = CreateButton("Return to menu", IntVector2(0, -60), IntVector2(150, 30), HA_CENTER, VA_BOTTOM);
    _exitButton = CreateButton("Exit game", IntVector2(0, -20), IntVector2(150, 30), HA_CENTER, VA_BOTTOM);

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
    SubscribeToEvent(_resumeButton, E_RELEASED, URHO3D_HANDLER(PauseWindow, HandleResume));
	SubscribeToEvent(_menuButton, E_RELEASED, URHO3D_HANDLER(PauseWindow, HandleReturnToMenu));
    SubscribeToEvent(_exitButton, E_RELEASED, URHO3D_HANDLER(PauseWindow, HandleExit));
}

void PauseWindow::HandleResume(StringHash eventType, VariantMap& eventData)
{
    VariantMap data = GetEventDataMap();
    data["Name"] = "PauseWindow";
    SendEvent(MyEvents::E_CLOSE_WINDOW, data);
}

void PauseWindow::HandleReturnToMenu(StringHash eventType, VariantMap& eventData)
{
	VariantMap data = GetEventDataMap();

	data["Name"] = "MainMenu";
	SendEvent(MyEvents::E_SET_LEVEL, data);

	data["Name"] = "PauseWindow";
	SendEvent(MyEvents::E_CLOSE_WINDOW, data);
}

void PauseWindow::HandleExit(StringHash eventType, VariantMap& eventData)
{
    VariantMap data = GetEventDataMap();

	data["Name"] = "ExitGame";
	SendEvent(MyEvents::E_SET_LEVEL, data);

	data["Name"] = "PauseWindow";
	SendEvent(MyEvents::E_CLOSE_WINDOW, data);
}