#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/UI/Text3D.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include "Player.h"
#include "../../MyEvents.h"
#include "../../Global.h"
#include "../../Input/ControllerInput.h"
#include "../../BehaviourTree/BehaviourTree.h"

static bool SHOW_LABELS = true;
static float MOVE_TORQUE = 20.0f;
static float JUMP_FORCE = 40.0f;

Player::Player(Context* context):
    Object(context)
{
    SubscribeToEvent(E_PHYSICSPRESTEP, URHO3D_HANDLER(Player, HandlePhysicsPrestep));
    SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(Player, HandlePostUpdate));

    SendEvent(
        MyEvents::E_CONSOLE_COMMAND_ADD,
        MyEvents::ConsoleCommandAdd::P_NAME, "show_labels",
        MyEvents::ConsoleCommandAdd::P_EVENT, "#show_labels",
        MyEvents::ConsoleCommandAdd::P_DESCRIPTION, "Show player labels",
        MyEvents::ConsoleCommandAdd::P_OVERWRITE, true
    );
    SubscribeToEvent("#show_labels", [&](StringHash eventType, VariantMap& eventData) {
        StringVector params = eventData["Parameters"].GetStringVector();
        if (params.Size() != 2) {
            URHO3D_LOGERROR("Invalid number of arguments!");
            return;
        }
        SHOW_LABELS = ToBool(params[1]);
        _label->SetEnabled(SHOW_LABELS);
    });

    SendEvent(
        MyEvents::E_CONSOLE_COMMAND_ADD,
        MyEvents::ConsoleCommandAdd::P_NAME, "movement_speed",
        MyEvents::ConsoleCommandAdd::P_EVENT, "#movement_speed",
        MyEvents::ConsoleCommandAdd::P_DESCRIPTION, "Show player labels",
        MyEvents::ConsoleCommandAdd::P_OVERWRITE, true
    );
    SubscribeToEvent("#movement_speed", [&](StringHash eventType, VariantMap& eventData) {
        StringVector params = eventData["Parameters"].GetStringVector();
        if (params.Size() != 2) {
            URHO3D_LOGERROR("Invalid number of arguments!");
            return;
        }
        MOVE_TORQUE = ToFloat(params[1]);
    });

    SendEvent(
        MyEvents::E_CONSOLE_COMMAND_ADD,
        MyEvents::ConsoleCommandAdd::P_NAME, "jump_force",
        MyEvents::ConsoleCommandAdd::P_EVENT, "#jump_force",
        MyEvents::ConsoleCommandAdd::P_DESCRIPTION, "Show player labels",
        MyEvents::ConsoleCommandAdd::P_OVERWRITE, true
    );
    SubscribeToEvent("#jump_force", [&](StringHash eventType, VariantMap& eventData) {
        StringVector params = eventData["Parameters"].GetStringVector();
        if (params.Size() != 2) {
            URHO3D_LOGERROR("Invalid number of arguments!");
            return;
        }
        JUMP_FORCE = ToFloat(params[1]);
    });
}

Player::~Player()
{
    UpdatePlayerList(true);
    _node->Remove();
}

void Player::RegisterObject(Context* context)
{
    context->RegisterFactory<Player>();
}

void Player::UpdatePlayerList(bool remove)
{
    if (_serverConnection) {
        return;
    }

    VariantMap players = GetGlobalVar("Players").GetVariantMap();
    if (remove) {
        players.Erase(String(_controllerId));
    } else {
        VariantMap data;
        data["Name"] = "Player " + String(_controllerId);
        data["Score"] = 0;
        players[String(_controllerId)] = data;
    }
    SetGlobalVar("Players", players);
}

void Player::CreateNode(Scene* scene, int controllerId, Terrain* terrain)
{
    SetControllerId(controllerId);

    auto* cache = GetSubsystem<ResourceCache>();

    // Create the scene node & visual representation. This will be a replicated object
    _node = scene->CreateChild("Player");
    _node->SetVar("Player", _controllerId);

    _node->SetPosition(Vector3(0, 2, 0));
    _node->SetScale(0.5f);

    auto* ballObject = _node->CreateComponent<StaticModel>();
    ballObject->SetModel(cache->GetResource<Model>("Models/Urchin.mdl"));
    ballObject->SetMaterial(cache->GetResource<Material>("Materials/NoMaterial.xml"));
    ballObject->SetCastShadows(true);

    // Create the physics components
    _rigidBody = _node->CreateComponent<RigidBody>();
    _rigidBody->SetMass(5.0f);
    _rigidBody->SetFriction(2.0f);
    // In addition to friction, use motion damping so that the ball can not accelerate limitlessly
    _rigidBody->SetLinearDamping(0.8f);
    _rigidBody->SetAngularDamping(0.8f);
    _rigidBody->SetCollisionLayerAndMask(COLLISION_MASK_PLAYER, COLLISION_MASK_PLAYER | COLLISION_MASK_CHECKPOINT | COLLISION_MASK_OBSTACLES | COLLISION_MASK_GROUND);
    _rigidBody->SetCollisionEventMode(CollisionEventMode::COLLISION_ALWAYS);

    auto* shape = _node->CreateComponent<CollisionShape>();
    shape->SetSphere(1.0f);

    _label = scene->CreateChild("Label");

    auto text3D = _label->CreateComponent<Text3D>();
    text3D->SetFont(cache->GetResource<Font>(APPLICATION_FONT), 30);
    text3D->SetColor(Color::GRAY);
    text3D->SetAlignment(HA_CENTER, VA_BOTTOM);
    text3D->SetFaceCameraMode(FaceCameraMode::FC_LOOKAT_Y);
    text3D->SetViewMask(~(1 << _controllerId));
    SetLabel();

    if (!SHOW_LABELS) {
        _label->SetEnabled(false);
    }

    SubscribeToEvent(_node, E_NODECOLLISION, URHO3D_HANDLER(Player, HandleNodeCollision));

    _terrain = terrain;

    ResetPosition();

    UpdatePlayerList();
}

void Player::FindNode(Scene* scene, int id)
{
    _node = scene->GetNode(id);
}

void Player::ResetPosition()
{
    Vector3 position = GetNode()->GetWorldPosition();
    if (_terrain) {
        position.y_ = _terrain->GetHeight(position) + 1.0f;
    } else {
        position.y_ = 1;
        position.x_ = 0;
        position.z_ = 0;
    }
    GetNode()->SetWorldPosition(position);

    GetNode()->GetComponent<RigidBody>()->SetLinearVelocity(Vector3::ZERO);
    GetNode()->GetComponent<RigidBody>()->SetAngularVelocity(Vector3::ZERO);
}

void Player::SetControllerId(unsigned int id)
{
    _controllerId = id;
}

void Player::SetRemotePlayerId(int id)
{
    _remotePlayerId = id;
}

Node* Player::GetNode()
{
    return _node.Get();
}

void Player::SetLabel()
{
    if (!_label) {
        return;
    }
    if (_isControlled) {
        _label->GetComponent<Text3D>()->SetText("Player " + String(_controllerId));
    } else {
        _label->GetComponent<Text3D>()->SetText("Bot " + String(_controllerId));
    }
}

void Player::SetControllable(bool value)
{
    _isControlled = value;
    if (_isControlled) {
        if (GetNode()->HasComponent<BehaviourTree>()) {
            GetNode()->RemoveComponent<BehaviourTree>();
        }
    } else {
        if (!GetNode()->HasComponent<BehaviourTree>()) {
            GetNode()->CreateComponent<BehaviourTree>();
            GetNode()->GetComponent<BehaviourTree>()->Init("Config/Behaviour.json");
        }
    }
    SetLabel();
}

void Player::HandlePhysicsPrestep(StringHash eventType, VariantMap& eventData)
{
    using namespace PhysicsPreStep;
    float timeStep = eventData[P_TIMESTEP].GetFloat();
    if (_serverConnection) {
        _serverConnection->SetControls(GetSubsystem<ControllerInput>()->GetControls(_controllerId));
        return;
    }

    if (_node->GetPosition().y_ < -30) {
        ResetPosition();

        if (_isControlled) {
            VariantMap &data = GetEventDataMap();
            data["Player"] = _controllerId;
            SendEvent("FallOffTheMap", data);
        }
    }

    Controls controls;
    float movementSpeed = MOVE_TORQUE;

    if (_isControlled) {
        if (_connection) {
            controls = _connection->GetControls();
        } else {
            controls = GetSubsystem<ControllerInput>()->GetControls(_controllerId);
        }
    } else {
        controls = GetNode()->GetComponent<BehaviourTree>()->GetControls();
    }

    if (controls.IsDown(CTRL_SPRINT)) {
        movementSpeed *= 2;
    }

    // Torque is relative to the forward vector
    Quaternion rotation(0.0f, controls.yaw_, 0.0f);
    if (controls.IsDown(CTRL_FORWARD)) {
        _rigidBody->ApplyTorque(rotation * Vector3::RIGHT * movementSpeed);
    }
    if (controls.IsDown(CTRL_BACK)) {
        _rigidBody->ApplyTorque(rotation * Vector3::LEFT * movementSpeed);
    }
    if (controls.IsDown(CTRL_LEFT)) {
        _rigidBody->ApplyTorque(rotation * Vector3::FORWARD * movementSpeed);
    }
    if (controls.IsDown(CTRL_RIGHT)) {
        _rigidBody->ApplyTorque(rotation * Vector3::BACK * movementSpeed);
    }
    if (controls.IsPressed(CTRL_JUMP, _controls) && _onGround) {
        _rigidBody->ApplyImpulse(Vector3::UP * JUMP_FORCE);
    }
    _controls = controls;
    _onGround = false;
}

void Player::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{
    if (_label) {
        _label->SetPosition(_node->GetPosition() + Vector3::UP * 0.2);
    }
}

void Player::HandleNodeCollision(StringHash eventType, VariantMap& eventData)
{
    // Check collision contacts and see if character is standing on ground (look for a contact that has near vertical normal)
    using namespace NodeCollision;

    MemoryBuffer contacts(eventData[P_CONTACTS].GetBuffer());

    while (!contacts.IsEof())
    {
        Vector3 contactPosition = contacts.ReadVector3();
        Vector3 contactNormal = contacts.ReadVector3();
        /*float contactDistance = */contacts.ReadFloat();
        /*float contactImpulse = */contacts.ReadFloat();

        // If contact is below node center and pointing up, assume it's a ground contact
        if (contactPosition.y_ < (_node->GetPosition().y_ + 1.0f))
        {
            float level = contactNormal.y_;
            if (level > 0.75)
                _onGround = true;
        }
    }
}

void Player::SetClientConnection(Connection* connection)
{
    _connection = connection;
}

void Player::SetServerConnection(Connection *connection)
{
    _serverConnection = connection;
}
