#include <Urho3D/Input/Input.h>
#include <Urho3D/Resource/Localization.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Engine/Engine.h>
#ifdef URHO3D_ANGELSCRIPT
#include <Urho3D/AngelScript/Script.h>
#endif
#include "MainMenu.h"
#include "../Global.h"
#include "../MyEvents.h"
#include "../Messages/Achievements.h"
#include "../AndroidEvents/ServiceCmd.h"
#include "../Input/ControllerInput.h"

using namespace Levels;

const static int BUTTON_FONT_SIZE = 20;

    /// Construct.
MainMenu::MainMenu(Context* context) :
    BaseLevel(context)
{
}

MainMenu::~MainMenu()
{
}

void MainMenu::RegisterObject(Context* context)
{
    context->RegisterFactory<MainMenu>();
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

    if (GetSubsystem<Engine>()->IsHeadless()) {
        return;
    }

    _cameraRotateNode = _scene->CreateChild("CameraRotate");
    _cameraRotateNode->AddChild(_cameras[0]);
    _cameras[0]->SetPosition(Vector3(1, 2, 1));
    _cameras[0]->LookAt(Vector3(0, 0, 0));
}

void MainMenu::SubscribeToEvents()
{
    if (GetSubsystem<Engine>()->IsHeadless()) {
        return;
    }
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(MainMenu, HandleUpdate));
}

void MainMenu::CreateUI()
{
    Input* input = GetSubsystem<Input>();
    if (!input->IsMouseVisible()) {
        input->SetMouseVisible(true);
    }

    if (_data.Contains("Message")) {
        auto* localization = GetSubsystem<Localization>();

        VariantMap& data = GetEventDataMap();
        data["Title"] = localization->Get("WARNING");
        data["Message"] = _data["Message"].GetString();
        data["Name"] = "PopupMessageWindow";
        data["Type"] = _data.Contains("Type") ? _data["Type"].GetString() : "warning";
        data["ClosePrevious"] = true;
        SendEvent(MyEvents::E_OPEN_WINDOW, data);
    }
    auto* localization = GetSubsystem<Localization>();

    _buttonsContainer = GetSubsystem<UI>()->GetRoot()->CreateChild<UIElement>();
    _buttonsContainer->SetFixedWidth(300);
    _buttonsContainer->SetLayout(LM_VERTICAL, 10);
    _buttonsContainer->SetAlignment(HA_RIGHT, VA_BOTTOM);
    _buttonsContainer->SetPosition(-10, -10);

    _newGameButton = CreateButton(localization->Get("NEW_GAME"));
    SubscribeToEvent(_newGameButton, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "NewGameSettingsWindow";
        SendEvent(MyEvents::E_OPEN_WINDOW, data);
    });

    // Load dynamic buttons
    VariantMap buttons = GetGlobalVar("MenuButtons").GetVariantMap();
    for (auto it = buttons.Begin(); it != buttons.End(); ++it) {
        VariantMap item = (*it).second_.GetVariantMap();
        SharedPtr<Button> button(CreateButton(item["Name"].GetString()));
        button->SetVar("EventToCall", item["Event"].GetString());
        _dynamicButtons.Push(button);
        SubscribeToEvent(_dynamicButtons.Back(), E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
            using namespace Released;
            Button* button = static_cast<Button*>(eventData[P_ELEMENT].GetPtr());
            SendEvent(button->GetVar("EventToCall").GetString());
        });
    }

    _settingsButton = CreateButton(localization->Get("SETTINGS"));
    SubscribeToEvent(_settingsButton, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "SettingsWindow";
        SendEvent(MyEvents::E_OPEN_WINDOW, data);
    });

    _achievementsButton = CreateButton(localization->Get("ACHIEVEMENTS"));
    SubscribeToEvent(_achievementsButton, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "AchievementsWindow";
        SendEvent(MyEvents::E_OPEN_WINDOW, data);
    });

    _creditsButton = CreateButton(localization->Get("CREDITS"));
    SubscribeToEvent(_creditsButton, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "Credits";
        SendEvent(MyEvents::E_SET_LEVEL, data);
    });

#if !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)
    _exitButton = CreateButton(localization->Get("EXIT"));
    SubscribeToEvent(_exitButton, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "QuitConfirmationWindow";
        SendEvent(MyEvents::E_OPEN_WINDOW, data);
    });
#endif

    // Test Communication between the sample and android activity
    GetSubsystem<ServiceCmd>()->SendCmdMessage(ANDROID_AD_LOAD_INTERSTITIAL, 1);

    SubscribeToEvent(MyEvents::E_SERVICE_MESSAGE, [&](StringHash eventType, VariantMap& eventData) {
        using namespace MyEvents::ServiceMessage;
        int eventId = eventData[P_COMMAND].GetInt();
        if (eventId == ANDROID_AD_INTERSTITIAL_LOADED) {
            GetSubsystem<ServiceCmd>()->SendCmdMessage(ANDROID_AD_SHOW_INTERSTITIAL, 1);
        }
    });
}

Button* MainMenu::CreateButton(const String& text)
{
    auto* cache = GetSubsystem<ResourceCache>();
    auto* font = cache->GetResource<Font>(APPLICATION_FONT);

    auto* button = _buttonsContainer->CreateChild<Button>();
    button->SetStyleAuto();
    button->SetFixedHeight(50);
    button->SetFocusMode(FM_FOCUSABLE);

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
    if (!GetSubsystem<ControllerInput>()->IsMappingInProgress()) {
        if (GetSubsystem<Input>()->GetKeyPress(KEY_ESCAPE)) {
            SendEvent(MyEvents::E_CLOSE_ALL_WINDOWS);
        }
    }

    static float elapsedTime = 0.0f;
    elapsedTime += timestep;

    float pos = 2.0 + Sin(elapsedTime * 20.0) + 1.0;
    _cameras[0]->SetPosition(Vector3(pos, pos, pos));
    _cameras[0]->LookAt(Vector3(0, 0, 0));

    _cameraRotateNode->Yaw(timestep * 10);
}
