#include <Urho3D/Urho3DAll.h>
#include "NewGameSettingsWindow.h"
#include "../../MyEvents.h"
#include "../../Audio/AudioManagerDefs.h"

/// Construct.
NewGameSettingsWindow::NewGameSettingsWindow(Context* context) :
    BaseWindow(context)
{
    Init();
}

NewGameSettingsWindow::~NewGameSettingsWindow()
{
    _newGameButton->Remove();
    _exitWindow->Remove();
    _baseWindow->Remove();
}

void NewGameSettingsWindow::Init()
{
    Create();

    SubscribeToEvents();
}

void NewGameSettingsWindow::Create()
{
    _baseWindow = GetSubsystem<UI>()->GetRoot()->CreateChild<Window>();
    _baseWindow->SetStyleAuto();
    _baseWindow->SetAlignment(HA_CENTER, VA_CENTER);
    _baseWindow->SetSize(220, 80);
    _baseWindow->BringToFront();

    _newGameButton = CreateButton("Start", 80, IntVector2(20, 0));
    _newGameButton->SetAlignment(HA_LEFT, VA_CENTER);

    SubscribeToEvent(_newGameButton, "Released", [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "Loading";
        SendEvent(MyEvents::E_SET_LEVEL, data);
    });

    _exitWindow = CreateButton("Exit", 80, IntVector2(-20, 0));
    _exitWindow->SetAlignment(HA_RIGHT, VA_CENTER);
    SubscribeToEvent(_exitWindow, "Released", [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "NewGameSettingsWindow";
        SendEvent(MyEvents::E_CLOSE_WINDOW, data);
    });
}

void NewGameSettingsWindow::SubscribeToEvents()
{
}


Button* NewGameSettingsWindow::CreateButton(const String& text, int width, IntVector2 position)
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