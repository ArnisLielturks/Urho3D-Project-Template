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

    for (auto it = controlIndexes.Begin(); it != controlIndexes.End(); ++it) {
        _players[(*it)] = new Player(context_);
        _players[(*it)]->CreateNode(_scene, (*it));
    }

    if (data_.Contains("Map") && data_["Map"].GetString() == "Scenes/Scene2.xml") {

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

        GenerateMap(30, 4, 0);

        auto *body = terrainNode->CreateComponent<RigidBody>();
        body->SetCollisionLayer(2); // Use layer bitmask 2 for static geometry
        auto *shape = terrainNode->CreateComponent<CollisionShape>();
        shape->SetTerrain();

        SendEvent(
                MyEvents::E_CONSOLE_COMMAND_ADD,
                MyEvents::ConsoleCommandAdd::P_NAME, "generate_map",
                MyEvents::ConsoleCommandAdd::P_EVENT, "#generate_map",
                MyEvents::ConsoleCommandAdd::P_DESCRIPTION, "Perlin noise map generating",
                MyEvents::ConsoleCommandAdd::P_OVERWRITE, true
        );
        SubscribeToEvent("#generate_map", [&](StringHash eventType, VariantMap &eventData) {
            StringVector params = eventData["Parameters"].GetStringVector();
            if (params.Size() < 4) {
                URHO3D_LOGERROR("generate_map expects 3 parameters: frequency(0.1 - 64.0), octaves(1 - 16), seed");
                return;
            }

            const float frequency = ToFloat(params[1]);
            const int octaves     = ToInt(params[2]);
            const int seed        = ToInt(params[3]);

            GenerateMap(frequency, octaves, seed);
        });
    }

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
}

void Level::GenerateMap(float frequency, int octaves, int seed)
{
    if (!_terrain) {
        return;
    }

    frequency = Clamp(frequency, 0.1f, 64.0f);
    octaves   = Clamp(octaves, 1, 16);

    URHO3D_LOGINFOF("Frequency: %f", frequency);
    URHO3D_LOGINFOF("Octaves: %d", octaves);
    URHO3D_LOGINFOF("Seed: %d", seed);


    PerlinNoise perlin(seed);

    Image *image = _terrain->GetHeightMap();
    for (int x = 0; x < image->GetWidth(); x++) {
        for (int y = 0; y < image->GetHeight(); y++) {
            float dx = x / frequency;
            float dy = y / frequency;
            auto result = perlin.octaveNoise0_1(dx, dy, octaves);
            image->SetPixel(x, y, Color(result, result, result));
        }
    }
    _terrain->ApplyHeightMap();  // has to be called to apply the changes

    auto* controllerInput = GetSubsystem<ControllerInput>();
    Vector<int> controlIndexes = controllerInput->GetControlIndexes();
    for (auto it = controlIndexes.Begin(); it != controlIndexes.End(); ++it) {
        Vector3 position = _players[(*it)]->GetNode()->GetWorldPosition();
        position.y_ = _terrain->GetHeight(position) + 1.0f;
        _players[(*it)]->GetNode()->SetWorldPosition(position);
    }

    image->SavePNG("Data/Textures/HeightMap.png");
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
        _players[controllerIndex]->CreateNode(_scene, controllerIndex);
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
