#include <Urho3D/Urho3DAll.h>
#include "Level.h"
#include "../MyEvents.h"
#include "../Global.h"
#include "../Audio/AudioManagerDefs.h"
#include "../Input/ControllerInput.h"
#include "../UI/WindowManager.h"

using namespace Levels;

    /// Construct.
Level::Level(Context* context) :
    BaseLevel(context),
    shouldReturn(false),
    _showScoreboard(false),
    _showWeaponChoice(false)
{
}

Level::~Level()
{
    StopAllAudio();
}

void Level::Init()
{
    Renderer* renderer = GetSubsystem<Renderer>();
    renderer->SetNumViewports(1);

    StopAllAudio();
    StartAudio();

    Network* network = GetSubsystem<Network>();
    network->RegisterRemoteEvent("SendPlayerNodeID");

    URHO3D_LOGRAW("Starting level: Level");
    BaseLevel::Init();

    // Create the scene content
    CreateScene();

    // Create the UI content
    CreateUI();

    auto* controllerInput = GetSubsystem<ControllerInput>();
    Vector<int> controlIndexes = controllerInput->GetControlIndexes();
    InitViewports(controlIndexes);
    for (auto it = controlIndexes.Begin(); it != controlIndexes.End(); ++it) {
        _players[(*it)] = new Node(context_);
    }

    // Subscribe to global events for camera movement
    SubscribeToEvents();

    Input* input = GetSubsystem<Input>();
    if (input->IsMouseVisible()) {
        input->SetMouseVisible(false);
    }
}

void Level::StartAudio()
{
    using namespace AudioDefs;
    using namespace MyEvents::PlaySound;
    VariantMap data = GetEventDataMap();
    data[P_INDEX] = AMBIENT_SOUNDS::LEVEL;
    data[P_TYPE] = SOUND_AMBIENT;
    SendEvent(MyEvents::E_PLAY_SOUND, data);
}

void Level::StopAllAudio()
{
    SendEvent(MyEvents::E_STOP_ALL_SOUNDS);
}

void Level::CreateScene()
{
    scene_ = new Scene(context_);
    scene_->CreateComponent<Octree>(LOCAL);
    scene_->CreateComponent<DebugRenderer>(LOCAL);
    File loadFile(context_, GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Scenes/Scene.xml", FILE_READ);
    scene_->LoadXML(loadFile);

    if (!scene_->HasComponent<PhysicsWorld>()) {
        scene_->CreateComponent<PhysicsWorld>(LOCAL);
    }

    CreateCamera();
}

void Level::CreateUI()
{
    UI* ui = GetSubsystem<UI>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    Text* text = ui->GetRoot()->CreateChild<Text>();
    text->SetHorizontalAlignment(HA_CENTER);
    text->SetVerticalAlignment(VA_CENTER);
    text->SetStyleAuto();
    text->SetText("This is ingame text!\nPress ESC to pause game\nPress TAB to show ScoreboardWindow\nPress F1 to show/hide console");
    text->SetTextEffect(TextEffect::TE_STROKE);
    text->SetFontSize(16);
    text->SetColor(Color(0.8f, 0.8f, 0.2f));
}

void Level::SubscribeToEvents()
{
    SubscribeToEvent(E_PHYSICSPRESTEP, URHO3D_HANDLER(Level, HandlePhysicsPrestep));
    SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(Level, HandlePostUpdate));
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(Level, HandleKeyDown));
    SubscribeToEvent(E_KEYUP, URHO3D_HANDLER(Level, HandleKeyUp));

    SubscribeToEvent(MyEvents::E_CONTROLLER_ADDED, URHO3D_HANDLER(Level, HandleControllerConnected));
    SubscribeToEvent(MyEvents::E_CONTROLLER_REMOVED, URHO3D_HANDLER(Level, HandleControllerDisconnected));
}

void Level::HandleControllerConnected(StringHash eventType, VariantMap& eventData)
{
    using namespace MyEvents::ControllerAdded;
    int controllerIndex = eventData[P_INDEX].GetInt();

    auto* controllerInput = GetSubsystem<ControllerInput>();
    Vector<int> controlIndexes = controllerInput->GetControlIndexes();
    InitViewports(controlIndexes);

    VariantMap data = GetEventDataMap();
    data["Message"] = "New controller connected!";
    SendEvent("ShowNotification", data);

    _players[controllerIndex] = new Node(context_);
}

void Level::HandleControllerDisconnected(StringHash eventType, VariantMap& eventData)
{
    using namespace MyEvents::ControllerRemoved;
    int controllerIndex = eventData[P_INDEX].GetInt();

    if (controllerIndex > 0) {
        _players.Erase(controllerIndex);
    }
    auto* controllerInput = GetSubsystem<ControllerInput>();
    Vector<int> controlIndexes = controllerInput->GetControlIndexes();
    InitViewports(controlIndexes);

    VariantMap data = GetEventDataMap();
    data["Message"] = "Controller disconnected!";
    SendEvent("ShowNotification", data);

}

void Level::UnsubscribeToEvents()
{
    UnsubscribeFromEvent(E_PHYSICSPRESTEP);
    UnsubscribeFromEvent(E_POSTUPDATE);
    UnsubscribeFromEvent(E_KEYDOWN);
    UnsubscribeFromEvent(E_KEYUP);
}

void Level::HandlePhysicsPrestep(StringHash eventType, VariantMap& eventData)
{
    if (!scene_->IsUpdateEnabled()) {
        return;
    }
    
    using namespace PhysicsPreStep;
    float timeStep = eventData[P_TIMESTEP].GetFloat();

    if (_showWeaponChoice) {
        return;
    }

    auto* controllerInput = GetSubsystem<ControllerInput>();
    for (auto it = _players.Begin(); it != _players.End(); ++it) {
        int playerId = (*it).first_;

        // Movement speed as world units per second
        float MOVE_SPEED = 2.0f;
        Controls controls = GetSubsystem<ControllerInput>()->GetControls(playerId);
        if (controls.IsDown(CTRL_SPRINT))
            MOVE_SPEED = 10.0f;
        if (controls.IsDown(CTRL_FORWARD))
            _cameras[playerId]->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
        if (controls.IsDown(CTRL_BACK))
            _cameras[playerId]->Translate(Vector3::BACK * MOVE_SPEED * timeStep);
        if (controls.IsDown(CTRL_LEFT))
            _cameras[playerId]->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
        if (controls.IsDown(CTRL_RIGHT))
            _cameras[playerId]->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);
        if (controls.IsDown(CTRL_UP))
            _cameras[playerId]->Translate(Vector3::UP * MOVE_SPEED * timeStep);
    }
}

void Level::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{

    if (!scene_->IsUpdateEnabled()) {
        return;
    }

    if (_showWeaponChoice) {
        return;
    }

    using namespace PostUpdate;
    float timeStep = eventData[P_TIMESTEP].GetFloat();
    auto* controllerInput = GetSubsystem<ControllerInput>();
    for (auto it = _players.Begin(); it != _players.End(); ++it) {
        int playerId = (*it).first_;
        Node* playerNode = (*it).second_;
        Controls controls = controllerInput->GetControls((*it).first_);
        if (_cameras[playerId]) {
            //_cameras[playerId]->SetWorldPosition(playerNode->GetWorldPosition() + Vector3::UP * 0.1);
            _cameras[playerId]->SetRotation(Quaternion(controls.pitch_, controls.yaw_, 0.0f));
        }
    }
}

void Level::OnLoaded()
{
    if (shouldReturn) {
        VariantMap eventData = GetEventDataMap();;
        eventData["Name"] = "MainMenu";
        eventData["Message"] = returnMessage;
        SendEvent(MyEvents::E_SET_LEVEL, eventData);
    }
}

void Level::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    int key = eventData["Key"].GetInt();

    // Toggle console by pressing F1
    if (key == KEY_TAB && !_showScoreboard) {
        VariantMap data = GetEventDataMap();
        data["Name"] = "ScoreboardWindow";
        SendEvent(MyEvents::E_OPEN_WINDOW, data);
        _showScoreboard = true;
    }

    if (key == KEY_ESCAPE) {
        UnsubscribeToEvents();
        VariantMap data = GetEventDataMap();
        data["Name"] = "PauseWindow";
        SendEvent(MyEvents::E_OPEN_WINDOW, data);
        SubscribeToEvent(MyEvents::E_WINDOW_CLOSED, URHO3D_HANDLER(Level, HandleWindowClosed));
        Pause();
    }

    if (key == KEY_F2) {
        WindowManager* windowManager = GetSubsystem<WindowManager>();
        if (windowManager->IsWindowOpen("WeaponChoiceWindow")) {
            VariantMap data = GetEventDataMap();
            data["Name"] = "WeaponChoiceWindow";
            SendEvent(MyEvents::E_CLOSE_WINDOW, data);

            Input* input = GetSubsystem<Input>();
            if (input->IsMouseVisible()) {
                input->SetMouseVisible(false);
            }

            _showWeaponChoice = false;
        } else {
            VariantMap data = GetEventDataMap();
            data["Name"] = "WeaponChoiceWindow";
            SendEvent(MyEvents::E_OPEN_WINDOW, data);

            Input* input = GetSubsystem<Input>();
            if (!input->IsMouseVisible()) {
                input->SetMouseVisible(true);
            }

            _showWeaponChoice = true;
        }
    }
}

void Level::HandleKeyUp(StringHash eventType, VariantMap& eventData)
{
    int key = eventData["Key"].GetInt();

    // Toggle console by pressing F1
    if (key == KEY_TAB && _showScoreboard) {
        VariantMap data = GetEventDataMap();
        data["Name"] = "ScoreboardWindow";
        SendEvent(MyEvents::E_CLOSE_WINDOW, data);
        _showScoreboard = false;
    }
}

void Level::HandleWindowClosed(StringHash eventType, VariantMap& eventData)
{
    String name = eventData["Name"].GetString();
    if (name == "PauseWindow") {
        UnsubscribeFromEvent(MyEvents::E_WINDOW_CLOSED);
        SubscribeToEvents();

        Input* input = GetSubsystem<Input>();
        if (input->IsMouseVisible()) {
            input->SetMouseVisible(false);
        }
        Run();
    }
}