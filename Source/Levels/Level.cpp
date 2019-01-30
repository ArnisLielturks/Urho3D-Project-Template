#include <Urho3D/Urho3DAll.h>
#include "Level.h"
#include "../MyEvents.h"
#include "../Global.h"
#include "../Audio/AudioManagerDefs.h"
#include "../Audio/AudioManager.h"
#include "../Input/ControllerInput.h"
#include "../UI/WindowManager.h"
#include "../Messages/Achievements.h"

using namespace Levels;

    /// Construct.
Level::Level(Context* context) :
    BaseLevel(context),
    _showScoreboard(false)
{
}

Level::~Level()
{
    StopAllAudio();
}

void Level::Init()
{
    if (!scene_) {
        // There is no scene, get back to the main menu
        VariantMap& eventData = GetEventDataMap();
        eventData["Name"] = "MainMenu";
        SendEvent(MyEvents::E_SET_LEVEL, eventData);

        return;
    }

    auto listener = scene_->CreateComponent<SoundListener>();
    GetSubsystem<Audio>()->SetListener(listener);
    // Enable achievement showing for this level
    GetSubsystem<Achievements>()->SetShowAchievements(true);

    StopAllAudio();
    StartAudio();

    auto* controllerInput = GetSubsystem<ControllerInput>();
    Vector<int> controlIndexes = controllerInput->GetControlIndexes();
    InitViewports(controlIndexes);

    // Create the scene content
    CreateScene();

    // Create the UI content
    CreateUI();

    for (auto it = controlIndexes.Begin(); it != controlIndexes.End(); ++it) {
        _players[(*it)] = CreateControllableObject();
    }

    // Subscribe to global events for camera movement
    SubscribeToEvents();

    Input* input = GetSubsystem<Input>();
    if (input->IsMouseVisible()) {
        input->SetMouseVisible(false);
    }

    Node* movableNode = scene_->GetChild("PathNode");
    _path = movableNode->GetComponent<SplinePath>();
}

void Level::StartAudio()
{
    using namespace AudioDefs;
    using namespace MyEvents::PlaySound;
    VariantMap data = GetEventDataMap();
    data[P_INDEX] = AMBIENT_SOUNDS::LEVEL;
    data[P_TYPE] = SOUND_AMBIENT;
    SendEvent(MyEvents::E_PLAY_SOUND, data);

    /*auto node = scene_->GetChild("Radio", true);
    if (node) {
        auto soundSource = GetSubsystem<AudioManager>()->AddEffectToNode(node, SOUND_EFFECTS::HIT);
        soundSource->SetFarDistance(10);
        soundSource->GetSound()->SetLooped(true);
    }*/
}

void Level::StopAllAudio()
{
    SendEvent(MyEvents::E_STOP_ALL_SOUNDS);
}

void Level::CreateScene()
{
    if (!scene_->HasComponent<PhysicsWorld>()) {
        scene_->CreateComponent<PhysicsWorld>(LOCAL);
    }
}

void Level::CreateUI()
{
    auto* localization = GetSubsystem<Localization>();
    UI* ui = GetSubsystem<UI>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    Text* text = ui->GetRoot()->CreateChild<Text>();
    text->SetHorizontalAlignment(HA_CENTER);
    text->SetVerticalAlignment(VA_TOP);
    text->SetPosition(0, 50);
    text->SetStyleAuto();
    text->SetText(localization->Get("TUTORIAL"));
    text->SetTextEffect(TextEffect::TE_STROKE);
    text->SetFontSize(16);
    text->SetColor(Color(0.8f, 0.8f, 0.2f));
    text->SetBringToBack(true);
}

void Level::SubscribeToEvents()
{
    SubscribeToEvent(E_PHYSICSPRESTEP, URHO3D_HANDLER(Level, HandlePhysicsPrestep));
    SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(Level, HandlePostUpdate));
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(Level, HandleKeyDown));
    SubscribeToEvent(E_KEYUP, URHO3D_HANDLER(Level, HandleKeyUp));

    SubscribeToEvent(MyEvents::E_CONTROLLER_ADDED, URHO3D_HANDLER(Level, HandleControllerConnected));
    SubscribeToEvent(MyEvents::E_CONTROLLER_REMOVED, URHO3D_HANDLER(Level, HandleControllerDisconnected));

    SubscribeToEvent(MyEvents::E_VIDEO_SETTINGS_CHANGED, URHO3D_HANDLER(Level, HandleVideoSettingsChanged));
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

    _players[controllerIndex] = CreateControllableObject();
}

void Level::HandleControllerDisconnected(StringHash eventType, VariantMap& eventData)
{
    using namespace MyEvents::ControllerRemoved;
    int controllerIndex = eventData[P_INDEX].GetInt();

    if (controllerIndex > 0) {
        _playerLabels[_players[controllerIndex]]->Remove();
        _playerLabels.Erase(_players[controllerIndex]);
        _players[controllerIndex]->Remove();
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


    auto* controllerInput = GetSubsystem<ControllerInput>();
    for (auto it = _players.Begin(); it != _players.End(); ++it) {
        int playerId = (*it).first_;
        if (!_players.Contains(playerId)) {
            continue;
        }

        // Movement speed as world units per second
        float MOVE_TORQUE = 20.0f;
        Controls controls = GetSubsystem<ControllerInput>()->GetControls(playerId);
        if (controls.IsDown(CTRL_SPRINT)) {
            MOVE_TORQUE = 40.0f;
        }
        auto* body = _players[playerId]->GetComponent<RigidBody>();
        // Torque is relative to the forward vector
        Quaternion rotation(0.0f, controls.yaw_, 0.0f);
        if (controls.IsDown(CTRL_FORWARD)) {
            body->ApplyTorque(rotation * Vector3::RIGHT * MOVE_TORQUE);
         //   _cameras[playerId]->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
        }
        if (controls.IsDown(CTRL_BACK)) {
            body->ApplyTorque(rotation * Vector3::LEFT * MOVE_TORQUE);
            //_cameras[playerId]->Translate(Vector3::BACK * MOVE_SPEED * timeStep);
        }
        if (controls.IsDown(CTRL_LEFT)) {
            body->ApplyTorque(rotation * Vector3::FORWARD * MOVE_TORQUE);
            //_cameras[playerId]->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
        }
        if (controls.IsDown(CTRL_RIGHT)) {
            body->ApplyTorque(rotation * Vector3::BACK * MOVE_TORQUE);
           // _cameras[playerId]->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);
        }
        if (controls.IsDown(CTRL_UP)) {
           // _cameras[playerId]->Translate(Vector3::UP * MOVE_SPEED * timeStep);
        }
    }
}

void Level::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{

    if (!scene_->IsUpdateEnabled()) {
        return;
    }

    using namespace PostUpdate;
    float timeStep = eventData[P_TIMESTEP].GetFloat();
    if (_path) {
        _path->Move(timeStep);
        if (_path->IsFinished()) {
            _path->Reset();
        }
    }
    auto* controllerInput = GetSubsystem<ControllerInput>();
    for (auto it = _players.Begin(); it != _players.End(); ++it) {
        if ((*it).second_->GetPosition().y_ < -10) {
            (*it).second_->SetPosition(Vector3(0, 1, 0));
            SendEvent("FallOffTheMap");
        }

        int playerId = (*it).first_;
        Node* playerNode = (*it).second_;
        Controls controls = controllerInput->GetControls((*it).first_);
        if (_cameras[playerId]) {
            //_cameras[playerId]->SetWorldPosition(playerNode->GetWorldPosition() + Vector3::UP * 0.1);
            Quaternion rotation(controls.pitch_, controls.yaw_, 0.0f);
            _cameras[playerId]->SetRotation(rotation);
            const float CAMERA_DISTANCE = 2.0f;

            // Move camera some distance away from the ball
            _cameras[playerId]->SetPosition((*it).second_->GetPosition() + _cameras[playerId]->GetRotation() * Vector3::BACK * CAMERA_DISTANCE);
        }
    }

    for (auto it = _playerLabels.Begin(); it != _playerLabels.End(); ++it) {
        (*it).second_->SetPosition((*it).first_->GetPosition() + Vector3::UP * 0.2);
    }
}

void Level::OnLoaded()
{
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

void Level::HandleVideoSettingsChanged(StringHash eventType, VariantMap& eventData)
{
    auto* controllerInput = GetSubsystem<ControllerInput>();
    Vector<int> controlIndexes = controllerInput->GetControlIndexes();
    InitViewports(controlIndexes);
}

Node* Level::CreateControllableObject()
{
    auto* cache = GetSubsystem<ResourceCache>();

    // Create the scene node & visual representation. This will be a replicated object
    Node* ballNode = scene_->CreateChild("Player");
    ballNode->SetVar("Player", _players.Size());
    ballNode->SetPosition(Vector3(0, 2, 0));
    ballNode->SetScale(0.5f);
    auto* ballObject = ballNode->CreateComponent<StaticModel>();
    ballObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
    ballObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
    ballObject->SetCastShadows(true);

    // Create the physics components
    auto* body = ballNode->CreateComponent<RigidBody>();
    body->SetMass(5.0f);
    body->SetFriction(2.0f);
    // In addition to friction, use motion damping so that the ball can not accelerate limitlessly
    body->SetLinearDamping(0.8f);
    body->SetAngularDamping(0.8f);
    body->SetCollisionLayerAndMask(COLLISION_MASK_PLAYER, COLLISION_MASK_PLAYER | COLLISION_MASK_CHECKPOINT | COLLISION_MASK_OBSTACLES);

    auto* shape = ballNode->CreateComponent<CollisionShape>();
    shape->SetSphere(1.0f);

    // Create a random colored point light at the ball so that can see better where is going
    auto* light = ballNode->CreateComponent<Light>();
    light->SetRange(5.0f);
    light->SetColor(Color(0.5f + Random(0.5f), 0.5f + Random(0.5f), 0.5f + Random(0.5f)));
    light->SetCastShadows(false);

    _playerLabels[ballNode] = scene_->CreateChild("Label");

    auto text3D = _playerLabels[ballNode]->CreateComponent<Text3D>();
    text3D->SetText("Player " + String(_players.Size()));
    text3D->SetFont(cache->GetResource<Font>(APPLICATION_FONT), 30);
    text3D->SetColor(Color::GRAY);
    text3D->SetAlignment(HA_CENTER, VA_BOTTOM);
    text3D->SetFaceCameraMode(FaceCameraMode::FC_LOOKAT_Y);
    text3D->SetViewMask(~(1 << _players.Size()));

    return ballNode;
}