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
#include "../CustomEvents.h"
#include "../Messages/Achievements.h"
#include "../AndroidEvents/ServiceCmd.h"
#include "../Input/ControllerInput.h"
#include "../LevelManagerEvents.h"
#include "../UI/WindowEvents.h"
#include "../AndroidEvents/ServiceEvents.h"

using namespace Levels;
using namespace LevelManagerEvents;
using namespace WindowEvents;
using namespace CustomEvents;
using namespace ServiceEvents;

const static int BUTTON_FONT_SIZE = 20;

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
    scene_ = new Scene(context_);
    scene_->CreateComponent<Octree>();
    auto xmlFile = GetSubsystem<ResourceCache>()->GetResource<XMLFile>("Scenes/Menu.xml");
    scene_->LoadXML(xmlFile->GetRoot());

    InitCamera();

    SubscribeToEvent(E_VIDEO_SETTINGS_CHANGED, [&](StringHash eventType, VariantMap& eventData) {
        InitCamera();
    });

#ifdef URHO3D_ANGELSCRIPT
    if (GetSubsystem<Script>()) {
        GetSubsystem<Script>()->SetDefaultScene(scene_);
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

    cameraRotateNode_ = scene_->CreateChild("CameraRotate");
    cameraRotateNode_->AddChild(cameras_[0]);
    cameras_[0]->SetPosition(Vector3(1, 2, 1));
    cameras_[0]->LookAt(Vector3(0, 0, 0));
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

    if (data_.Contains("Message")) {
        auto* localization = GetSubsystem<Localization>();

        VariantMap& data = GetEventDataMap();
        data["Title"] = localization->Get("WARNING");
        data["Message"] = data_["Message"].GetString();
        data["Name"] = "PopupMessageWindow";
        data["Type"] = data_.Contains("Type") ? data_["Type"].GetString() : "warning";
        data["ClosePrevious"] = true;
        SendEvent(E_OPEN_WINDOW, data);
    }
    auto* localization = GetSubsystem<Localization>();

    buttonsContainer_ = GetSubsystem<UI>()->GetRoot()->CreateChild<UIElement>();
    buttonsContainer_->SetFixedWidth(300);
    buttonsContainer_->SetLayout(LM_VERTICAL, 10);
    buttonsContainer_->SetAlignment(HA_RIGHT, VA_BOTTOM);
    buttonsContainer_->SetPosition(-10, -10);

    AddButton("NewGameSettingsWindow", localization->Get("NEW_GAME"), "NewGameSettingsWindow", E_OPEN_WINDOW);
    AddButton("SettingsWindow", localization->Get("SETTINGS"), "SettingsWindow", E_OPEN_WINDOW);
    AddButton("AchievementsWindow", localization->Get("ACHIEVEMENTS"), "AchievementsWindow", E_OPEN_WINDOW);
    AddButton("Credits", localization->Get("CREDITS"), "Credits", E_SET_LEVEL);
#if !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)
    AddButton("QuitConfirmationWindow", localization->Get("EXIT"), "QuitConfirmationWindow", E_OPEN_WINDOW);
#endif

    // Load dynamic buttons
    VariantMap buttons = GetGlobalVar("MenuButtons").GetVariantMap();
    for (auto it = buttons.Begin(); it != buttons.End(); ++it) {
        VariantMap item = (*it).second_.GetVariantMap();
        SharedPtr<Button> button(CreateButton(item["Name"].GetString()));
        button->SetVar("EventToCall", item["EventToCall"].GetStringHash());
        button->SetVar("Data", item["Data"].GetVariantMap());
        dynamicButtons_.Push(button);
        SubscribeToEvent(button, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
            using namespace Released;
            Button* button = static_cast<Button*>(eventData[P_ELEMENT].GetPtr());
            VariantMap data = button->GetVar("Data").GetVariantMap();
            SendEvent(button->GetVar("EventToCall").GetStringHash(), data);
        });
    }

    // Test Communication between the sample and android activity
    GetSubsystem<ServiceCmd>()->SendCmdMessage(ANDROID_AD_LOAD_INTERSTITIAL, 1);

    SubscribeToEvent(E_SERVICE_MESSAGE, [&](StringHash eventType, VariantMap& eventData) {
        using namespace ServiceMessage;
        int eventId = eventData[P_COMMAND].GetInt();
        if (eventId == ANDROID_AD_INTERSTITIAL_LOADED) {
            GetSubsystem<ServiceCmd>()->SendCmdMessage(ANDROID_AD_SHOW_INTERSTITIAL, 1);
        }
    });
}

void MainMenu::AddButton(const String& buttonName, const String& label, const String& windowToOpen, const StringHash& eventToCall)
{
    VariantMap buttons = GetGlobalVar("MenuButtons").GetVariantMap();

    VariantMap data;
    data["Name"] = windowToOpen;
    VariantMap newGameButton;
    newGameButton["Name"] = label;
    newGameButton["EventToCall"] = eventToCall;
    newGameButton["Data"] = data;

    buttons[buttonName] = newGameButton;

    SetGlobalVar("MenuButtons", buttons);
}

Button* MainMenu::CreateButton(const String& text)
{
    auto* cache = GetSubsystem<ResourceCache>();
    auto* font = cache->GetResource<Font>(APPLICATION_FONT);

    auto* button = buttonsContainer_->CreateChild<Button>();
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
            SendEvent(E_CLOSE_ALL_WINDOWS);
        }
    }

    static float elapsedTime = 0.0f;
    elapsedTime += timestep;

    float pos = 2.0 + Sin(elapsedTime * 20.0) + 1.0;
    cameras_[0]->SetPosition(Vector3(pos, pos, pos));
    cameras_[0]->LookAt(Vector3(0, 0, 0));

    cameraRotateNode_->Yaw(timestep * 10);
}
