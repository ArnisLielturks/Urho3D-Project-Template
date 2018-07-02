#include <Urho3D/Urho3DAll.h>
#include "MainMenu.h"
#include "../MyEvents.h"
#include <string>

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
    if (data_.Contains("Message")) {
        URHO3D_LOGERROR("Main menu created with message : " + data_["Message"].GetString());
        //SharedPtr<Urho3D::MessageBox> messageBox(new Urho3D::MessageBox(context_, data_["Message"].GetString(), "Oh crap!"));
        VariantMap data;
        data["Title"] = "Error!";
        data["Message"] = data_["Message"].GetString();
        SendEvent("ShowAlertMessage", data);
    }
    URHO3D_LOGINFO("Starting level: MainMenu");
    BaseLevel::Init();

    VariantMap map;
    map["CONSOLE_COMMAND_NAME"] = "connect";
    map["CONSOLE_COMMAND_EVENT"] = "HandleConnect";
    SendEvent("ConsoleCommandAdd", map);

    // Create the scene content
    CreateScene();

    // Create the UI content
    CreateUI();

    // Subscribe to global events for camera movement
    SubscribeToEvents();
    VariantMap data;
    data["Message"] = "Entered menu!";
    SendEvent("NewAchievement", data);

    data["Title"] = "Error!";
    data["Message"] = "Seems like everything is ok!";
    SendEvent("ShowAlertMessage", data);
}

void MainMenu::CreateScene()
{
    
}

void MainMenu::CreateUI()
{
    UI* ui = GetSubsystem<UI>();

    //////////////
    _startButton = ui->GetRoot()->CreateChild<Button>();
    _startButton->SetSize(IntVector2(100, 30));
    _startButton->SetStyleAuto();

    _startButton->SetAlignment(HA_CENTER, VA_CENTER);

    Text* text = _startButton->CreateChild<Text>();
    text->SetText("Start game!");
    text->SetStyleAuto();
    text->SetAlignment(HA_CENTER, VA_CENTER);
}

void MainMenu::SubscribeToEvents()
{
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(MainMenu, HandleUpdate));
    SubscribeToEvent(_startButton, E_RELEASED, URHO3D_HANDLER(MainMenu, HandleStartGame));
}

void MainMenu::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    Input* input = GetSubsystem<Input>();
    if (!input->IsMouseVisible()) {
        input->SetMouseVisible(true);
    }
}

void MainMenu::HandleStartGame(StringHash eventType, VariantMap& eventData)
{
    VariantMap& levelEventData = GetEventDataMap();
    levelEventData["Name"] = "Loading";
    SendEvent(MyEvents::E_SET_LEVEL, levelEventData);
}