#include <Urho3D/Urho3DAll.h>
#include "MainMenu.h"
#include "../Global.h"
#include "../MyEvents.h"
#include "../Audio/AudioManagerDefs.h"
#include "../Messages/Achievements.h"
#include "../AndroidEvents/ServiceCmd.h"

using namespace Levels;

    /// Construct.
MainMenu::MainMenu(Context* context) :
    BaseLevel(context)
{
}

MainMenu::~MainMenu()
{
}

void MainMenu::Init()
{
    // Disable achievement showing for this level
    GetSubsystem<Achievements>()->SetShowAchievements(true);

    // Create the scene content
    CreateScene();

    // Create the UI content
    CreateUI();
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

    if (data_.Contains("Message")) {
        auto* localization = GetSubsystem<Localization>();

        VariantMap& data = GetEventDataMap();
        data["Title"] = localization->Get("WARNING");
        data["Message"] = data_["Message"].GetString();
        data["Name"] = "PopupMessageWindow";
        data["Type"] = "warning";
        data["ClosePrevious"] = true;
        SendEvent(MyEvents::E_OPEN_WINDOW, data);
    }
    auto* localization = GetSubsystem<Localization>();

    int marginBottom = -180;
    _newGameButton = CreateButton(localization->Get("NEW_GAME"), 150, IntVector2(-20, marginBottom));
    _newGameButton->SetAlignment(HA_RIGHT, VA_BOTTOM);
    SubscribeToEvent(_newGameButton, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "NewGameSettingsWindow";
        SendEvent(MyEvents::E_OPEN_WINDOW, data);

    });

    marginBottom += 40;
    _settingsButton = CreateButton(localization->Get("SETTINGS"), 150, IntVector2(-20, marginBottom));
    _settingsButton->SetAlignment(HA_RIGHT, VA_BOTTOM);
    SubscribeToEvent(_settingsButton, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "SettingsWindow";
        SendEvent(MyEvents::E_OPEN_WINDOW, data);
    });

    marginBottom += 40;
    _achievementsButton = CreateButton(localization->Get("ACHIEVEMENTS"), 150, IntVector2(-20, marginBottom));
    _achievementsButton->SetAlignment(HA_RIGHT, VA_BOTTOM);
    SubscribeToEvent(_achievementsButton, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "AchievementsWindow";
        SendEvent(MyEvents::E_OPEN_WINDOW, data);

        GetSubsystem<ServiceCmd>()->SendCmdMessage(10, 1);
    });

    marginBottom += 40;
    _creditsButton = CreateButton(localization->Get("CREDITS"), 150, IntVector2(-20, marginBottom));
    _creditsButton->SetAlignment(HA_RIGHT, VA_BOTTOM);
    SubscribeToEvent(_creditsButton, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "Credits";
        SendEvent(MyEvents::E_SET_LEVEL, data);
    });

    marginBottom += 40;
    _exitButton = CreateButton(localization->Get("EXIT"), 150, IntVector2(-20, marginBottom));
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