#include <Urho3D/Urho3DAll.h>
#include "PauseWindow.h"
#include "../../MyEvents.h"
#include "../../Audio/AudioManagerDefs.h"

/// Construct.
PauseWindow::PauseWindow(Context* context) :
    BaseWindow(context)
{
    Init();
}

PauseWindow::~PauseWindow()
{
    _continueButton->Remove();
    _mainMenuButton->Remove();
    _exitButton->Remove();
    _baseWindow->Remove();
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

    _baseWindow = GetSubsystem<UI>()->GetRoot()->CreateChild<Window>();
    _baseWindow->SetStyleAuto();
    _baseWindow->SetAlignment(HA_CENTER, VA_CENTER);
    _baseWindow->SetSize(300, 190);
    _baseWindow->BringToFront();

    _continueButton = CreateButton("Continue", 200, IntVector2(0, 20));
    _continueButton->SetAlignment(HA_CENTER, VA_TOP);

    SubscribeToEvent(_continueButton, "Released", [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "PauseWindow";
        SendEvent(MyEvents::E_CLOSE_WINDOW, data);
    });

    _mainMenuButton = CreateButton("Return to menu", 200, IntVector2(0, 60));
    _mainMenuButton->SetAlignment(HA_CENTER, VA_TOP);

    SubscribeToEvent(_mainMenuButton, "Released", [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "PauseWindow";
        SendEvent(MyEvents::E_CLOSE_WINDOW, data);

        data["Name"] = "MainMenu";
        SendEvent(MyEvents::E_SET_LEVEL, data);
    });

    _settingsButton = CreateButton("Settings", 200, IntVector2(0, 100));
    _settingsButton->SetAlignment(HA_CENTER, VA_TOP);

    SubscribeToEvent(_settingsButton, "Released", [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "SettingsWindow";
        SendEvent(MyEvents::E_OPEN_WINDOW, data);
    });

    _exitButton = CreateButton("Exit game", 200, IntVector2(0, 140));
    _exitButton->SetAlignment(HA_CENTER, VA_TOP);

    SubscribeToEvent(_exitButton, "Released", [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "PauseWindow";
        SendEvent(MyEvents::E_CLOSE_WINDOW, data);

        data["Name"] = "ExitGame";
        SendEvent(MyEvents::E_SET_LEVEL, data);
    });

}

void PauseWindow::SubscribeToEvents()
{
}

Button* PauseWindow::CreateButton(const String& text, int width, IntVector2 position)
{
    auto* cache = GetSubsystem<ResourceCache>();
    auto* font = cache->GetResource<Font>("Fonts/Anonymous Pro.ttf");

    auto* button = _baseWindow->CreateChild<Button>();
    button->SetStyleAuto();
    button->SetFixedWidth(width);
    button->SetFixedHeight(30);
    button->SetPosition(position);

    auto* buttonText = button->CreateChild<Text>();
    buttonText->SetFont(font, 12);
    buttonText->SetAlignment(HA_CENTER, VA_CENTER);
    buttonText->SetText(text);

    return button;
}
