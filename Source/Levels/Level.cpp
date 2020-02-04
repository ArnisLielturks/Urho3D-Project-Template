#include <Urho3D/Resource/Localization.h>
#include <Urho3D/Audio/SoundListener.h>
#include <Urho3D/Audio/Audio.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Resource/Image.h>
#include "../Generator/Generator.h"
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/Material.h>
#include "../Generator/PerlinNoise.h"
#include "Level.h"
#include "../MyEvents.h"
#include "../Global.h"
#include "../Audio/AudioManagerDefs.h"
#include "../Audio/AudioManager.h"
#include "../Input/ControllerInput.h"
#include "../UI/WindowManager.h"
#include "../Messages/Achievements.h"

using namespace Levels;

Level::Level(Context* context) :
    BaseLevel(context),
    _showScoreboard(false),
    _drawDebug(false)
{
    Player::RegisterObject(context);
}

Level::~Level()
{
    StopAllAudio();
}

void Level::Init()
{
    if (!_scene) {
        auto localization = GetSubsystem<Localization>();
        // There is no scene, get back to the main menu
        VariantMap& eventData = GetEventDataMap();
        eventData["Name"] = "MainMenu";
        eventData["Message"] = localization->Get("NO_SCENE");
        SendEvent(MyEvents::E_SET_LEVEL, eventData);

        return;
    }

    auto listener = _scene->CreateComponent<SoundListener>();
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

    if (_data.Contains("Map") && _data["Map"].GetString() == "Scenes/Terrain.xml") {

        auto cache = GetSubsystem<ResourceCache>();
        Node *terrainNode = _scene->CreateChild("Terrain");
        terrainNode->SetPosition(Vector3(0.0f, -0.0f, 0.0f));
        _terrain = terrainNode->CreateComponent<Terrain>();
        _terrain->SetPatchSize(64);
        _terrain->SetSpacing(Vector3(1.0f, 0.1f, 1.0f));
        _terrain->SetSmoothing(true);
        _terrain->SetHeightMap(cache->GetResource<Image>("Textures/HeightMap.png"));
        _terrain->SetMaterial(cache->GetResource<Material>("Materials/Terrain.xml"));
        _terrain->SetOccluder(true);
        _terrain->SetCastShadows(true);

        auto *body = terrainNode->CreateComponent<RigidBody>();
        body->SetCollisionLayer(2); // Use layer bitmask 2 for static geometry
        auto *shape = terrainNode->CreateComponent<CollisionShape>();
        shape->SetTerrain();
    }

    Node* zoneNode = _scene->CreateChild("Zone", LOCAL);
    _defaultZone = zoneNode->CreateComponent<Zone>();
    _defaultZone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
    _defaultZone->SetAmbientColor(Color(0.1f, 0.1f, 0.1f));
    _defaultZone->SetFogStart(100.0f);
    _defaultZone->SetFogEnd(300.0f);

    // Subscribe to global events for camera movement
    SubscribeToEvents();

    auto input = GetSubsystem<Input>();
    if (input->IsMouseVisible()) {
        input->SetMouseVisible(false);
    }

    Node* movableNode = _scene->GetChild("PathNode");
    if (movableNode) {
        _path = movableNode->GetComponent<SplinePath>();
    }

    for (auto it = controlIndexes.Begin(); it != controlIndexes.End(); ++it) {
        _players[(*it)] = new Player(context_);
        _players[(*it)]->CreateNode(_scene, (*it), _terrain);
    }
}
//
//void Level::GenerateMap(float frequency, int octaves, int seed)
//{
//    if (!_terrain) {
//        return;
//    }
//
//    frequency = Clamp(frequency, 0.1f, 64.0f);
//    octaves   = Clamp(octaves, 1, 16);
//
//    URHO3D_LOGINFOF("Frequency: %f", frequency);
//    URHO3D_LOGINFOF("Octaves: %d", octaves);
//    URHO3D_LOGINFOF("Seed: %d", seed);
//
//
//    Image* image = GetSubsystem<Generator>()->GenerateImage(frequency, octaves, seed);
//    _terrain->SetHeightMap(image);
//
//    // Make sure our players are on top of terrain and don't get stuck
//    auto* controllerInput = GetSubsystem<ControllerInput>();
//    Vector<int> controlIndexes = controllerInput->GetControlIndexes();
//    for (auto it = controlIndexes.Begin(); it != controlIndexes.End(); ++it) {
//        Vector3 position = _players[(*it)]->GetNode()->GetWorldPosition();
//        position.y_ = _terrain->GetHeight(position) + 1.0f;
//        _players[(*it)]->GetNode()->SetWorldPosition(position);
//    }
//}

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
    if (!_scene->HasComponent<PhysicsWorld>()) {
        _scene->CreateComponent<PhysicsWorld>(LOCAL);
    }
}

void Level::CreateUI()
{
    auto* localization = GetSubsystem<Localization>();
    auto ui = GetSubsystem<UI>();

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
    SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(Level, HandlePostRenderUpdate));

    SubscribeToEvent(MyEvents::E_CONTROLLER_ADDED, URHO3D_HANDLER(Level, HandleControllerConnected));
    SubscribeToEvent(MyEvents::E_CONTROLLER_REMOVED, URHO3D_HANDLER(Level, HandleControllerDisconnected));

    SubscribeToEvent(MyEvents::E_VIDEO_SETTINGS_CHANGED, URHO3D_HANDLER(Level, HandleVideoSettingsChanged));

    SendEvent(
        MyEvents::E_CONSOLE_COMMAND_ADD,
        MyEvents::ConsoleCommandAdd::P_NAME, "debug_geometry",
        MyEvents::ConsoleCommandAdd::P_EVENT, "#debug_geometry",
        MyEvents::ConsoleCommandAdd::P_DESCRIPTION, "Toggle debugging geometry",
        MyEvents::ConsoleCommandAdd::P_OVERWRITE, true
    );
    SubscribeToEvent("#debug_geometry", [&](StringHash eventType, VariantMap& eventData) {
        StringVector params = eventData["Parameters"].GetStringVector();
        if (params.Size() > 1) {
            URHO3D_LOGERROR("This command doesn't take any arguments!");
            return;
        }
        _drawDebug = !_drawDebug;
    });

    SendEvent(
            MyEvents::E_CONSOLE_COMMAND_ADD,
            MyEvents::ConsoleCommandAdd::P_NAME, "ambient_light",
            MyEvents::ConsoleCommandAdd::P_EVENT, "#ambient_light",
            MyEvents::ConsoleCommandAdd::P_DESCRIPTION, "Change scene ambient light",
            MyEvents::ConsoleCommandAdd::P_OVERWRITE, true
    );
    SubscribeToEvent("#ambient_light", [&](StringHash eventType, VariantMap &eventData) {
        StringVector params = eventData["Parameters"].GetStringVector();
        if (params.Size() < 4) {
            URHO3D_LOGERROR("ambient_light expects 3 float values: r g b ");
            return;
        }

        const float r = ToFloat(params[1]);
        const float g = ToFloat(params[2]);
        const float b = ToFloat(params[3]);
        _defaultZone->SetAmbientColor(Color(r, g, b));
    });

    SendEvent(
            MyEvents::E_CONSOLE_COMMAND_ADD,
            MyEvents::ConsoleCommandAdd::P_NAME, "fog",
            MyEvents::ConsoleCommandAdd::P_EVENT, "#fog",
            MyEvents::ConsoleCommandAdd::P_DESCRIPTION, "Change custom scene fog",
            MyEvents::ConsoleCommandAdd::P_OVERWRITE, true
    );
    SubscribeToEvent("#fog", [&](StringHash eventType, VariantMap &eventData) {
        StringVector params = eventData["Parameters"].GetStringVector();
        if (params.Size() < 3) {
            URHO3D_LOGERROR("fog expects 2 parameters: fog_start fog_end ");
            return;
        }

        const float start = ToFloat(params[1]);
        const float end = ToFloat(params[2]);
        _defaultZone->SetFogStart(start);
        _defaultZone->SetFogEnd(end);
    });
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

    if (!_players.Contains(controllerIndex)) {
        _players[controllerIndex] = new Player(context_);
        _players[controllerIndex]->CreateNode(_scene, controllerIndex, _terrain);
    }
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
    if (!_scene->IsUpdateEnabled()) {
        return;
    }
}

void Level::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
{
    if (!_drawDebug) {
        return;
    }
    _scene->GetComponent<PhysicsWorld>()->DrawDebugGeometry(true);
    _scene->GetComponent<PhysicsWorld>()->SetDebugRenderer(_scene->GetComponent<DebugRenderer>());
}

void Level::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{

    if (!_scene->IsUpdateEnabled()) {
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

        int playerId = (*it).first_;
        Controls controls = controllerInput->GetControls((*it).first_);
        if (_cameras[playerId]) {
            Quaternion rotation(controls.pitch_, controls.yaw_, 0.0f);
            _cameras[playerId]->SetRotation(rotation);
            const float CAMERA_DISTANCE = 1.5f;

            // Move camera some distance away from the ball
            _cameras[playerId]->SetPosition((*it).second_->GetNode()->GetPosition() + _cameras[playerId]->GetRotation() * Vector3::BACK * CAMERA_DISTANCE);
        }
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
