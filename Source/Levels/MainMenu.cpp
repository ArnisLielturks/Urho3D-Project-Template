#include <Urho3D/Urho3DAll.h>
#include "MainMenu.h"
#include "../Global.h"
#include "../MyEvents.h"
#include "../Audio/AudioManagerDefs.h"
#include "../Messages/Achievements.h"

using namespace Levels;

    /// Construct.
MainMenu::MainMenu(Context* context) :
    BaseLevel(context),
    _showGUI(true),
    _active(true)
{
}

MainMenu::~MainMenu()
{
}

void MainMenu::Init()
{
    // Disable achievement showing for this level
    GetSubsystem<Achievements>()->SetShowAchievements(true);

    if (data_.Contains("Message")) {
        //SharedPtr<Urho3D::MessageBox> messageBox(new Urho3D::MessageBox(context_, data_["Message"].GetString(), "Oh crap!"));
        VariantMap data = GetEventDataMap();
        data["Title"] = "Error!";
        data["Message"] = data_["Message"].GetString();
        SendEvent("ShowAlertMessage", data);
    }
    BaseLevel::Init();

    // Create the scene content
    CreateScene();

    // Create the UI content
    CreateUI();

    VariantMap data = GetEventDataMap();
    data["Message"] = "Entered menu!";
    SendEvent("NewAchievement", data);

//    data["Title"] = "Hey!";
//    data["Message"] = "Seems like everything is ok!";
//    SendEvent("ShowAlertMessage", data);
}

void MainMenu::CreateScene()
{
    
}

void MainMenu::CreateUI()
{
    Input* input = GetSubsystem<Input>();
    if (!input->IsMouseVisible()) {
        input->SetMouseVisible(true);
    }

    int marginBottom = -140;
    _newGameButton = CreateButton("New game", 150, IntVector2(-20, marginBottom));
    _newGameButton->SetAlignment(HA_RIGHT, VA_BOTTOM);

    SubscribeToEvent(_newGameButton, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "NewGameSettingsWindow";
        SendEvent(MyEvents::E_OPEN_WINDOW, data);

    });

    marginBottom += 40;
    _settingsButton = CreateButton("Settings", 150, IntVector2(-20, marginBottom));
    _settingsButton->SetAlignment(HA_RIGHT, VA_BOTTOM);
    SubscribeToEvent(_settingsButton, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "SettingsWindow";
        SendEvent(MyEvents::E_OPEN_WINDOW, data);
    });

    marginBottom += 40;
    _creditsButton = CreateButton("Credits", 150, IntVector2(-20, marginBottom));
    _creditsButton->SetAlignment(HA_RIGHT, VA_BOTTOM);
    SubscribeToEvent(_creditsButton, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "Credits";
        SendEvent(MyEvents::E_SET_LEVEL, data);
    });

    marginBottom += 40;
    _exitButton = CreateButton("Exit", 150, IntVector2(-20, marginBottom));
    _exitButton->SetAlignment(HA_RIGHT, VA_BOTTOM);
    SubscribeToEvent(_exitButton, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "QuitConfirmationWindow";
        SendEvent(MyEvents::E_OPEN_WINDOW, data);
    });
}

Button* MainMenu::CreateButton(const String& text, int width, IntVector2 position)
{
    auto* cache = GetSubsystem<ResourceCache>();
    auto* font = cache->GetResource<Font>(APPLICATION_FONT);

    auto* button = GetSubsystem<UI>()->GetRoot()->CreateChild<Button>();
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