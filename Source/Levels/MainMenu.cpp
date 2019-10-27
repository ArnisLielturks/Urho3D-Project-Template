#include <Urho3D/Input/Input.h>
#include <Urho3D/Resource/Localization.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Resource/XMLFile.h>
#ifdef URHO3D_ANGELSCRIPT
#include <Urho3D/AngelScript/Script.h>
#endif
#include "MainMenu.h"
#include "../Global.h"
#include "../MyEvents.h"
#include "../Audio/AudioManagerDefs.h"
#include "../Messages/Achievements.h"
#include "../AndroidEvents/ServiceCmd.h"

using namespace Levels;

const static int BUTTON_WIDTH = 250;
const static int BUTTON_HEIGHT = 50;
const static int BUTTON_SPACING = 10;
const static int BUTTON_FONT_SIZE = 20;

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

    SubscribeToEvents();
}

void MainMenu::CreateScene()
{
    // Create a simple background scene for the menu
    _scene = new Scene(context_);
    _scene->CreateComponent<Octree>();
    auto xmlFile = GetSubsystem<ResourceCache>()->GetResource<XMLFile>("Scenes/Menu.xml");
    _scene->LoadXML(xmlFile->GetRoot());

    InitCamera();

    SubscribeToEvent(MyEvents::E_VIDEO_SETTINGS_CHANGED, [&](StringHash eventType, VariantMap& eventData) {
        InitCamera();
    });

    auto* zone = _scene->CreateComponent<Zone>();
    zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
    zone->SetFogStart(1.0f);
    zone->SetFogEnd(20.0f);

#ifdef URHO3D_ANGELSCRIPT
    if (GetSubsystem<Script>()) {
        GetSubsystem<Script>()->SetDefaultScene(_scene);
    }
#endif
}

void MainMenu::InitCamera()
{
    CreateSingleCamera();
    ApplyPostProcessEffects();

    _cameraRotateNode = _scene->CreateChild("CameraRotate");
    _cameraRotateNode->AddChild(_cameras[0]);
    _cameras[0]->SetPosition(Vector3(3, 3, 3));
    _cameras[0]->LookAt(Vector3(0, 0, 0));
}

void MainMenu::SubscribeToEvents()
{
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(MainMenu, HandleUpdate));
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

    int marginBottom = -4 * (BUTTON_HEIGHT + BUTTON_SPACING) - BUTTON_SPACING;
    _newGameButton = CreateButton(localization->Get("NEW_GAME"), BUTTON_WIDTH, IntVector2(-BUTTON_SPACING, marginBottom));
    _newGameButton->SetAlignment(HA_RIGHT, VA_BOTTOM);
    SubscribeToEvent(_newGameButton, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "NewGameSettingsWindow";
        SendEvent(MyEvents::E_OPEN_WINDOW, data);

    });

    marginBottom += BUTTON_HEIGHT + BUTTON_SPACING;
    _settingsButton = CreateButton(localization->Get("SETTINGS"), BUTTON_WIDTH, IntVector2(-BUTTON_SPACING, marginBottom));
    _settingsButton->SetAlignment(HA_RIGHT, VA_BOTTOM);
    SubscribeToEvent(_settingsButton, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "SettingsWindow";
        SendEvent(MyEvents::E_OPEN_WINDOW, data);
    });

    marginBottom += BUTTON_HEIGHT + BUTTON_SPACING;
    _achievementsButton = CreateButton(localization->Get("ACHIEVEMENTS"), BUTTON_WIDTH, IntVector2(-BUTTON_SPACING, marginBottom));
    _achievementsButton->SetAlignment(HA_RIGHT, VA_BOTTOM);
    SubscribeToEvent(_achievementsButton, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "AchievementsWindow";
        SendEvent(MyEvents::E_OPEN_WINDOW, data);
    });

    marginBottom += BUTTON_HEIGHT + BUTTON_SPACING;
    _creditsButton = CreateButton(localization->Get("CREDITS"), BUTTON_WIDTH, IntVector2(-BUTTON_SPACING, marginBottom));
    _creditsButton->SetAlignment(HA_RIGHT, VA_BOTTOM);
    SubscribeToEvent(_creditsButton, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "Credits";
        SendEvent(MyEvents::E_SET_LEVEL, data);
    });

    marginBottom += BUTTON_HEIGHT + BUTTON_SPACING;
    _exitButton = CreateButton(localization->Get("EXIT"), BUTTON_WIDTH, IntVector2(-BUTTON_SPACING, marginBottom));
    _exitButton->SetAlignment(HA_RIGHT, VA_BOTTOM);
    SubscribeToEvent(_exitButton, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "QuitConfirmationWindow";
        SendEvent(MyEvents::E_OPEN_WINDOW, data);
    });

    // Test Communication between the sample and android activity
    GetSubsystem<ServiceCmd>()->SendCmdMessage(11, 1);
}

Button* MainMenu::CreateButton(const String& text, int width, IntVector2 position)
{
    auto* cache = GetSubsystem<ResourceCache>();
    auto* font = cache->GetResource<Font>(APPLICATION_FONT);

    auto* button = GetSubsystem<UI>()->GetRoot()->CreateChild<Button>();
    button->SetStyleAuto();
    button->SetFixedWidth(width);
    button->SetFixedHeight(BUTTON_HEIGHT);
    button->SetPosition(position);

    auto* buttonText = button->CreateChild<Text>();
    buttonText->SetFont(font, BUTTON_FONT_SIZE);
    buttonText->SetAlignment(HA_CENTER, VA_CENTER);
    buttonText->SetText(text);

    return button;
}

void MainMenu::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;
    float timestep = eventData[P_TIMESTEP].GetFloat();
    if (GetSubsystem<Input>()->GetKeyPress(KEY_ESCAPE)) {
        SendEvent(MyEvents::E_CLOSE_ALL_WINDOWS);
    }

    _cameraRotateNode->Yaw(timestep * 5);
}
