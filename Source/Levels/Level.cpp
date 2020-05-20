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
#include <Urho3D/Network/NetworkEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include "../Generator/Generator.h"
#include "Level.h"
#include "../MyEvents.h"
#include "../Global.h"
#include "../Audio/AudioManagerDefs.h"
#include "../Audio/AudioManager.h"
#include "../Input/ControllerInput.h"
#include "../Messages/Achievements.h"

using namespace Levels;

static int REMOTE_PLAYER_ID = 1000;

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
        _terrain->SetHeightMap(GetSubsystem<Generator>()->GetImage());
        _terrain->SetMaterial(cache->GetResource<Material>("Materials/Terrain.xml"));
        _terrain->SetOccluder(true);
        _terrain->SetCastShadows(true);

        auto *body = terrainNode->CreateComponent<RigidBody>();
        body->SetCollisionLayerAndMask(COLLISION_MASK_GROUND, COLLISION_MASK_PLAYER | COLLISION_MASK_OBSTACLES);
        auto *shape = terrainNode->CreateComponent<CollisionShape>();
        shape->SetTerrain();
    }

    // Subscribe to global events for camera movement
    SubscribeToEvents();

    auto input = GetSubsystem<Input>();
    if (input->IsMouseVisible()) {
        input->SetMouseVisible(false);
    }

    if (!GetSubsystem<Engine>()->IsHeadless()) {
        for (auto it = controlIndexes.Begin(); it != controlIndexes.End(); ++it) {
            _players[(*it)] = new Player(context_);
            using namespace MyEvents::RemoteClientId;
            if (_data.Contains(P_NODE_ID) && _data.Contains(P_PLAYER_ID)) {
                // We are the client, we have to lookup the node on the received scene
                _players[(*it)]->FindNode(_scene, _data[P_NODE_ID].GetInt());
                _players[(*it)]->SetControllerId((*it));
            } else {
                _players[(*it)]->CreateNode(_scene, (*it), _terrain);
            }
            _players[(*it)]->SetControllable(true);
            if (_data.Contains("ConnectServer") && !_data["ConnectServer"].GetString().Empty()) {
                _players[(*it)]->SetServerConnection(GetSubsystem<Network>()->GetServerConnection());
                VectorBuffer msg;
                msg.WriteString("");
                GetSubsystem<Network>()->GetServerConnection()->SendMessage(
                        MyEvents::RemoteMessages::MSG_ASK_PLAYER_LIST, true, true, msg);
                URHO3D_LOGINFO("Asking server for player list");
            }
        }
    }

    if (!_data.Contains("ConnectServer")) {
//        _players[100] = new Player(context_);
//        _players[100]->CreateNode(_scene, 100, _terrain);
//        _players[100]->SetControllable(false);
    }
}

void Level::StartAudio()
{
    using namespace AudioDefs;
    using namespace MyEvents::PlaySound;
    VariantMap& data = GetEventDataMap();
    data[P_INDEX] = AMBIENT_SOUNDS::LEVEL;
    data[P_TYPE]  = SOUND_AMBIENT;
    SendEvent(MyEvents::E_PLAY_SOUND, data);
}

void Level::StopAllAudio()
{
    SendEvent(MyEvents::E_STOP_ALL_SOUNDS);
}

void Level::CreateScene()
{
    if (!_scene->HasComponent<PhysicsWorld>()) {
        _scene->CreateComponent<PhysicsWorld>(REPLICATED);
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

    SubscribeToEvent(E_CLIENTCONNECTED, URHO3D_HANDLER(Level, HandleClientConnected));
    SubscribeToEvent(E_CLIENTDISCONNECTED, URHO3D_HANDLER(Level, HandleClientDisconnected));
    SubscribeToEvent(E_SERVERCONNECTED, URHO3D_HANDLER(Level, HandleServerConnected));
    SubscribeToEvent(E_SERVERDISCONNECTED, URHO3D_HANDLER(Level, HandleServerDisconnected));

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

    GetSubsystem<Network>()->RegisterRemoteEvent(MyEvents::E_REMOTE_PLAYER_SCORE_UPDATE);
    GetSubsystem<Network>()->RegisterRemoteEvent(MyEvents::E_REMOTE_ALL_PLAYER_SCORE_UPDATE);

    SubscribeToEvent(MyEvents::E_REMOTE_PLAYER_SCORE_UPDATE, [&](StringHash eventType, VariantMap& eventData) {
        using namespace MyEvents::RemotePlayerScoreUpdate;
        String playerId = String(eventData[P_ID].GetInt());
        VariantMap players = GetGlobalVar("Players").GetVariantMap();
        VariantMap playerData = players[playerId].GetVariantMap();
        playerData["Score"] = eventData[P_SCORE].GetInt();
        playerData["ID"] = eventData[P_ID].GetInt();
        players[playerId] = playerData;
        SetGlobalVar("Players", players);

        SendEvent(MyEvents::E_PLAYER_SCORES_UPDATED);
    });

    SubscribeToEvent(MyEvents::E_PLAYER_SCORE_CHANGED, [&](StringHash eventType, VariantMap& eventData) {
        using namespace MyEvents::PlayerScoreChanged;
        String playerId = String(eventData[P_ID].GetInt());
        int score = eventData[P_SCORE].GetInt();

        VariantMap players = GetGlobalVar("Players").GetVariantMap();
        VariantMap playerData = players[playerId].GetVariantMap();
        int currentScore = playerData["Score"].GetInt();
        currentScore += score;
        if (currentScore < 0) {
            currentScore = 0;
        }
        playerData["Score"] = currentScore;
        playerData["ID"] = eventData[P_ID].GetInt();
        players[playerId] = playerData;
        SetGlobalVar("Players", players);

        VariantMap& data = GetEventDataMap();
        data[MyEvents::RemotePlayerScoreUpdate::P_ID] = eventData[P_ID].GetInt();
        data[MyEvents::RemotePlayerScoreUpdate::P_SCORE] = currentScore;
        GetSubsystem<Network>()->BroadcastRemoteEvent(MyEvents::E_REMOTE_PLAYER_SCORE_UPDATE, true, data);

        SendEvent(MyEvents::E_PLAYER_SCORES_UPDATED);
    });

    SubscribeToEvent(MyEvents::E_REMOTE_ALL_PLAYER_SCORE_UPDATE, [&](StringHash eventType, VariantMap& eventData) {
        using namespace MyEvents::RemoteAllPlayerScoreUpdate;
        SetGlobalVar("Players", eventData[P_DATA].GetVariantMap());

        SendEvent(MyEvents::E_PLAYER_SCORES_UPDATED);
    });

    SubscribeToEvent(E_NETWORKMESSAGE, [&](StringHash eventType, VariantMap& eventData) {
        auto* network = GetSubsystem<Network>();

        using namespace NetworkMessage;

        int msgID = eventData[P_MESSAGEID].GetInt();
        if (msgID == MyEvents::RemoteMessages::MSG_ASK_PLAYER_LIST) {
            const PODVector<unsigned char> &data = eventData[P_DATA].GetBuffer();
            // Use a MemoryBuffer to read the message data so that there is no unnecessary copying
            MemoryBuffer msg(data);
            String text = msg.ReadString();

            // If we are the server, prepend the sender's IP address and port and echo to everyone
            // If we are a client, just display the message
            if (network->IsServerRunning()) {
                URHO3D_LOGINFO("Client asked for player list, sending him it");
                auto *sender = static_cast<Connection *>(eventData[P_CONNECTION].GetPtr());
                VariantMap& data = GetEventDataMap();
                data[P_DATA] = GetGlobalVar("Players").GetVariantMap();
                sender->SendRemoteEvent(MyEvents::E_REMOTE_ALL_PLAYER_SCORE_UPDATE, true, data);
            }
        }
    });

    SubscribeToEvent(MyEvents::E_LEVEL_BEFORE_DESTROY, [&](StringHash eventType, VariantMap& eventData) {
        _remotePlayers.Clear();
        UnsubscribeFromEvent(E_SERVERDISCONNECTED);
        if (GetSubsystem<Network>() && GetSubsystem<Network>()->IsServerRunning()) {
            GetSubsystem<Network>()->StopServer();
        }
        if (GetSubsystem<Network>() && GetSubsystem<Network>()->GetServerConnection()) {
            GetSubsystem<Network>()->Disconnect();
        }
    });
}

void Level::HandleControllerConnected(StringHash eventType, VariantMap& eventData)
{
    using namespace MyEvents::ControllerAdded;
    int controllerIndex = eventData[P_INDEX].GetInt();

    auto* controllerInput = GetSubsystem<ControllerInput>();
    Vector<int> controlIndexes = controllerInput->GetControlIndexes();
    InitViewports(controlIndexes);

    VariantMap& data = GetEventDataMap();
    data["Message"]  = "New controller connected!";
    SendEvent("ShowNotification", data);

    if (!_players.Contains(controllerIndex)) {
        _players[controllerIndex] = new Player(context_);
        _players[controllerIndex]->CreateNode(_scene, controllerIndex, _terrain);
        _players[controllerIndex]->SetControllable(true);
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

    VariantMap& data = GetEventDataMap();
    data["Message"]  = "Controller disconnected!";
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

    if (key == KEY_TAB && !_showScoreboard) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "ScoreboardWindow";
        SendEvent(MyEvents::E_OPEN_WINDOW, data);
        _showScoreboard = true;
    }

    if (key == KEY_ESCAPE) {
        UnsubscribeToEvents();
        VariantMap& data = GetEventDataMap();
        data["Name"] = "PauseWindow";
        SendEvent(MyEvents::E_OPEN_WINDOW, data);
        SubscribeToEvent(MyEvents::E_WINDOW_CLOSED, URHO3D_HANDLER(Level, HandleWindowClosed));
        Pause();
    }
}

void Level::HandleKeyUp(StringHash eventType, VariantMap& eventData)
{
    int key = eventData["Key"].GetInt();

    if (key == KEY_TAB && _showScoreboard) {
        VariantMap& data = GetEventDataMap();
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

void Level::HandleClientConnected(StringHash eventType, VariantMap& eventData)
{
    using namespace ClientConnected;

    // When a client connects, assign to scene to begin scene replication
    auto* newConnection = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());
    newConnection->SetScene(_scene);
    URHO3D_LOGINFO("Level::HandleClientConnected");
    _remotePlayers[newConnection] = new Player(context_);
    _remotePlayers[newConnection]->SetClientConnection(newConnection);
    _remotePlayers[newConnection]->CreateNode(_scene, REMOTE_PLAYER_ID++, _terrain);
    _remotePlayers[newConnection]->SetControllable(true);

    using namespace MyEvents::RemoteClientId;
    VariantMap data;
    data[P_NODE_ID] = _remotePlayers[newConnection]->GetNode()->GetID();
    data[P_PLAYER_ID] = _remotePlayers[newConnection]->GetControllerId();
    URHO3D_LOGINFOF("Sending out remote client id %d", _remotePlayers[newConnection]->GetNode()->GetID());
    newConnection->SendRemoteEvent(MyEvents::E_REMOTE_CLIENT_ID, true, data);
}

void Level::HandleClientDisconnected(StringHash eventType, VariantMap& eventData)
{
    URHO3D_LOGINFO("Client disconnected");
    using namespace ClientDisconnected;

    // When a client connects, assign to scene to begin scene replication
    auto* connection = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());
    _remotePlayers.Erase(connection);
}

void Level::HandleServerConnected(StringHash eventType, VariantMap& eventData)
{

}

void Level::HandleServerDisconnected(StringHash eventType, VariantMap& eventData)
{
    _players.Clear();
    auto localization = GetSubsystem<Localization>();
    VariantMap data;
    data["Name"] = "MainMenu";
    data["Message"] = localization->Get("DISCONNECTED_FROM_SERVER");
    SendEvent(MyEvents::E_SET_LEVEL, data);
}