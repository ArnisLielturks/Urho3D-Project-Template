#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Scene/SceneEvents.h>
#include "Player.h"
#include "../../Global.h"
#include "../../Input/ControllerInput.h"
#include "../../BehaviourTree/BehaviourTree.h"
#include "PlayerState.h"
#include "../../Console/ConsoleHandlerEvents.h"

static float MOVE_TORQUE = 20.0f;
static float JUMP_FORCE = 40.0f;

using namespace ConsoleHandlerEvents;

Player::Player(Context* context):
    Object(context)
{
    SubscribeToEvent(E_PHYSICSPRESTEP, URHO3D_HANDLER(Player, HandlePhysicsPrestep));
    RegisterConsoleCommands();
}

Player::~Player()
{
    if (_node) {
        _node->Remove();
    }
}

void Player::RegisterObject(Context* context)
{
    context->RegisterFactory<Player>();
    PlayerState::RegisterObject(context);
}

void Player::RegisterConsoleCommands()
{
    SendEvent(
            E_CONSOLE_COMMAND_ADD,
            ConsoleCommandAdd::P_NAME, "movement_speed",
            ConsoleCommandAdd::P_EVENT, "#movement_speed",
            ConsoleCommandAdd::P_DESCRIPTION, "Show player labels",
            ConsoleCommandAdd::P_OVERWRITE, true
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
            E_CONSOLE_COMMAND_ADD,
            ConsoleCommandAdd::P_NAME, "jump_force",
            ConsoleCommandAdd::P_EVENT, "#jump_force",
            ConsoleCommandAdd::P_DESCRIPTION, "Show player labels",
            ConsoleCommandAdd::P_OVERWRITE, true
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

void Player::CreateNode(Scene* scene, int controllerId, Terrain* terrain)
{
    SetControllerId(controllerId);

    auto* cache = GetSubsystem<ResourceCache>();

    // Create the scene node & visual representation. This will be a replicated object
    _node = scene->CreateChild("Player");
    auto playerState = _node->CreateComponent<PlayerState>(REPLICATED);
    playerState->SetPlayerID(_controllerId);
    URHO3D_LOGINFOF("Creating player node=%d, playerstate=%d", _node->GetID(), playerState->GetID());
    _node->SetVar("Player", _controllerId);

    _node->SetPosition(Vector3(0, 2, 0));
    _node->SetScale(0.5f);

    auto* ballObject = _node->CreateComponent<StaticModel>();
    ballObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
    ballObject->SetMaterial(cache->GetResource<Material>("Materials/Ball.xml"));
    ballObject->SetCastShadows(true);

    // Create the physics components
    _rigidBody = _node->CreateComponent<RigidBody>(LOCAL);
    _rigidBody->SetMass(5.0f);
    _rigidBody->SetFriction(2.0f);
    // In addition to friction, use motion damping so that the ball can not accelerate limitlessly
    _rigidBody->SetLinearDamping(0.8f);
    _rigidBody->SetAngularDamping(0.8f);
    _rigidBody->SetCollisionLayerAndMask(COLLISION_MASK_PLAYER, COLLISION_MASK_PLAYER | COLLISION_MASK_CHECKPOINT | COLLISION_MASK_OBSTACLES | COLLISION_MASK_GROUND);

    auto* shape = _node->CreateComponent<CollisionShape>();
    shape->SetSphere(1.0f);

    SubscribeToEvent(_node, E_NODECOLLISION, URHO3D_HANDLER(Player, HandleNodeCollision));

    _terrain = terrain;

    ResetPosition();
}

void Player::FindNode(Scene* scene, int id)
{
    _node = scene->GetNode(id);
    if (_node) {
        _node->SetInterceptNetworkUpdate("Network Position", true);
        _node->GetComponent<PlayerState>()->HideLabel();
        SubscribeToEvent(E_INTERCEPTNETWORKUPDATE, URHO3D_HANDLER(Player, HandlePredictPlayerPosition));
    }
}

void Player::ResetPosition()
{
    Vector3 position = _spawnPoint;
    if (_terrain) {
        position.y_ = _terrain->GetHeight(position) + 1.0f;
    }
    GetNode()->SetWorldPosition(position);

    GetNode()->GetComponent<RigidBody>()->SetLinearVelocity(Vector3::ZERO);
    GetNode()->GetComponent<RigidBody>()->SetAngularVelocity(Vector3::ZERO);
}

void Player::SetControllerId(unsigned int id)
{
    _controllerId = id;
}

Node* Player::GetNode()
{
    return _node.Get();
}

void Player::SetControllable(bool value)
{
    _isControlled = value;
    if (_isControlled) {
        if (GetNode() && GetNode()->HasComponent<BehaviourTree>()) {
            GetNode()->RemoveComponent<BehaviourTree>();
        }
    } else {
        if (GetNode() && !GetNode()->HasComponent<BehaviourTree>()) {
            GetNode()->CreateComponent<BehaviourTree>();
            GetNode()->GetComponent<BehaviourTree>()->Init("Config/Behaviour.json");
        }
    }
}

void Player::HandlePhysicsPrestep(StringHash eventType, VariantMap& eventData)
{
    using namespace PhysicsPreStep;
    float timeStep = eventData[P_TIMESTEP].GetFloat();
    auto serverConnection = GetSubsystem<Network>()->GetServerConnection();
    if (serverConnection) {
        if (IsCameraTargetSet()) {
            // We are not following our player node, so we must not control it
            serverConnection->SetControls(Controls());
        } else {
            serverConnection->SetControls(GetSubsystem<ControllerInput>()->GetControls(_controllerId));
        }
        return;
    }

    if (_node->GetPosition().y_ < -20) {
        ResetPosition();

        VariantMap &data = GetEventDataMap();
        data["Player"] = _controllerId;
        SendEvent("FallOffTheMap", data);
        _node->GetComponent<PlayerState>()->AddScore(-10);
    }

    Controls controls;
    float movementSpeed = MOVE_TORQUE;

    if (_isControlled) {
        if (_connection) {
            controls = _connection->GetControls();
        } else if (!IsCameraTargetSet()) {
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
        static StringHash forward(CTRL_FORWARD);
        float strength = 1.0f;
        if (controls.extraData_.Contains(forward)) {
            strength = controls.extraData_[forward].GetFloat();
        }
        _rigidBody->ApplyTorque(rotation * Vector3::RIGHT * movementSpeed * strength);
    }
    if (controls.IsDown(CTRL_BACK)) {
        static StringHash backward(CTRL_BACK);
        float strength = 1.0f;
        if (controls.extraData_.Contains(backward)) {
            strength = controls.extraData_[backward].GetFloat();
        }
        _rigidBody->ApplyTorque(rotation * Vector3::LEFT * movementSpeed * strength);
    }
    if (controls.IsDown(CTRL_LEFT)) {
        static StringHash left(CTRL_LEFT);
        float strength = 1.0f;
        if (controls.extraData_.Contains(left)) {
            strength = controls.extraData_[left].GetFloat();
        }
        _rigidBody->ApplyTorque(rotation * Vector3::FORWARD * movementSpeed * strength);
    }
    if (controls.IsDown(CTRL_RIGHT)) {
        static StringHash right(CTRL_RIGHT);
        float strength = 1.0f;
        if (controls.extraData_.Contains(right)) {
            strength = controls.extraData_[right].GetFloat();
        }
        _rigidBody->ApplyTorque(rotation * Vector3::BACK * movementSpeed * strength);
    }
    if (controls.IsPressed(CTRL_JUMP, _controls) && _onGround) {
        _rigidBody->ApplyImpulse(Vector3::UP * JUMP_FORCE);
    }
    _controls = controls;
    _onGround = false;
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

void Player::SetCameraTarget(Node* target)
{
    _cameraTarget = target;
}

Node* Player::GetCameraTarget()
{
    if (_cameraTarget) {
        return _cameraTarget;
    }

    return GetNode();
}

bool Player::IsCameraTargetSet()
{
    return _cameraTarget && _cameraTarget != _node;
}

void Player::SetCameraDistance(float distance)
{
    _cameraDistance = distance;
}

float Player::GetCameraDistance()
{
    return _cameraDistance;
}

void Player::SetName(const String& name)
{
    if (_node) {
        _node->GetOrCreateComponent<PlayerState>()->SetPlayerName(name);
    }
}

void Player::HandlePredictPlayerPosition(StringHash eventType, VariantMap& eventData)
{
    using namespace InterceptNetworkUpdate;
    String name = eventData[P_NAME].GetString();
    Node* node = static_cast<Node*>(eventData[P_SERIALIZABLE].GetPtr());
    Vector3 position = eventData[P_VALUE].GetVector3();
//    _node->SetWorldPosition(position);
//    URHO3D_LOGINFOF("HandlePredictPlayerPosition %s", (position - node->GetPosition()).ToString().CString());
    const AttributeInfo& attr = node->GetAttributes()->At(eventData[P_INDEX].GetInt());
    node->OnSetAttribute(attr, eventData[P_VALUE]);
}

void Player::SetSpawnPoint(Vector3 position)
{
    _spawnPoint = position;
}
