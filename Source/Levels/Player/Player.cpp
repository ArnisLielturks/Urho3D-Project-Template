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

#if !defined(__EMSCRIPTEN__)
#include <Urho3D/Network/Network.h>
#endif

#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Engine/DebugHud.h>
#include "Player.h"
#include "../../Global.h"
#include "../../Input/ControllerInput.h"
#include "../../BehaviourTree/BehaviourTree.h"
#include "PlayerState.h"
#include "../../Console/ConsoleHandlerEvents.h"

#ifdef VOXEL_SUPPORT
#include "../Voxel/VoxelWorld.h"
#endif

#include "../../Input/ControllerEvents.h"

static float MOVE_TORQUE = 20.0f;
static float JUMP_FORCE = 40.0f;
static float NOCLIP_CAMERA_INERTIA_TIME = 0.1f; // Camera inertia time
static float NOCLIP_CAMERA_SPEED = 5.0f; // Camera movement speed

using namespace ConsoleHandlerEvents;
using namespace ControllerEvents;

Player::Player(Context* context):
    Object(context)
{
    SubscribeToEvent(E_PHYSICSPRESTEP, URHO3D_HANDLER(Player, HandlePhysicsPrestep));
    SubscribeToEvent(E_MAPPED_CONTROL_PRESSED, URHO3D_HANDLER(Player, HandleMappedControlPressed));
    RegisterConsoleCommands();

    auto* cache = GetSubsystem<ResourceCache>();
    auto* font = cache->GetResource<Font>(APPLICATION_FONT);

#ifdef VOXEL_SUPPORT
    if (GetSubsystem<VoxelWorld>()) {
        selectedItemUI_ = GetSubsystem<UI>()->GetRoot()->CreateChild<Text>();
        selectedItemUI_->SetAlignment(HA_CENTER, VA_BOTTOM);
        selectedItemUI_->SetPosition(0, -20);
        selectedItemUI_->SetStyleAuto();
        selectedItemUI_->SetFont(font, 20);
        selectedItemUI_->SetText(GetSubsystem<VoxelWorld>()->GetBlockName(static_cast<BlockType>(selectedItem_)));
    }
#endif

    positionUI_ = GetSubsystem<UI>()->GetRoot()->CreateChild<Text>();
    positionUI_->SetAlignment(HA_LEFT, VA_BOTTOM);
    positionUI_->SetPosition(20, -20);
    positionUI_->SetStyleAuto();
    positionUI_->SetFont(font, 20);
}

Player::~Player()
{
    if (node_) {
#ifdef VOXEL_SUPPORT
        if (GetSubsystem<VoxelWorld>()) {
            GetSubsystem<VoxelWorld>()->RemoveObserver(node_);
        }
#endif
        node_->Remove();
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

    SendEvent(
            E_CONSOLE_COMMAND_ADD,
            ConsoleCommandAdd::P_NAME, "noclip",
            ConsoleCommandAdd::P_EVENT, "#noclip",
            ConsoleCommandAdd::P_DESCRIPTION, "Enable/Disable noclip mode",
            ConsoleCommandAdd::P_OVERWRITE, true
    );
    SubscribeToEvent("#noclip", [&](StringHash eventType, VariantMap& eventData) {
        StringVector params = eventData["Parameters"].GetStringVector();
        if (params.Size() != 1) {
            URHO3D_LOGERROR("This command doesn't have any arguments!");
            return;
        }
        if (!rigidBody_) {
            rigidBody_ = node_->GetComponent<RigidBody>();
        }
        if (noclip_) {
            noclip_ = false;
            rigidBody_->SetLinearFactor(Vector3::ONE);
        } else {
            noclip_ = true;
            rigidBody_->SetLinearFactor(Vector3::ZERO);
//            SetCameraTarget(noclipNode_);
//            SetCameraDistance(0.0f);

//            GetSubsystem<VoxelWorld>()->AddObserver(noclipNode_);

            // Clear server connection controls
#if !defined(__EMSCRIPTEN__)
            auto serverConnection = GetSubsystem<Network>()->GetServerConnection();
            if (serverConnection) {
                serverConnection->SetControls(Controls());
            }
#endif
        }
    });
}

void Player::CreateNode(Scene* scene, int controllerId, Terrain* terrain, int type)
{
    type_ = type;
    SetControllerId(controllerId);

    auto* cache = GetSubsystem<ResourceCache>();

    // Create the scene node & visual representation. This will be a replicated object
    node_ = scene->CreateChild("Player" + String(controllerId));
    auto playerState = node_->CreateComponent<PlayerState>(REPLICATED);
    playerState->SetPlayerID(controllerId_);
    URHO3D_LOGINFOF("Creating player node=%d, playerstate=%d", node_->GetID(), playerState->GetID());
    node_->SetVar("Player", controllerId_);

    node_->SetPosition(Vector3(0, 2, 0));
    node_->SetScale(0.9f);

    auto* ballObject = node_->CreateComponent<StaticModel>();
    ballObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
    ballObject->SetMaterial(cache->GetResource<Material>("Materials/Ball.xml"));
    ballObject->SetCastShadows(true);
    ballObject->SetViewMask(VIEW_MASK_PLAYER);

    if (type == 0) {
        // Create the physics components
        rigidBody_ = node_->CreateComponent<RigidBody>(REPLICATED);
        rigidBody_->SetMass(5.0f);
        rigidBody_->SetFriction(2.0f);
        // In addition to friction, use motion damping so that the ball can not accelerate limitlessly
        rigidBody_->SetLinearDamping(0.8f);
        rigidBody_->SetAngularDamping(0.8f);
        rigidBody_->SetCollisionEventMode(COLLISION_ALWAYS);
        rigidBody_->SetCollisionLayerAndMask(COLLISION_MASK_PLAYER | COLLISION_MASK_CHUNK_LOADER,
                                             COLLISION_MASK_PLAYER | COLLISION_MASK_CHECKPOINT |
                                             COLLISION_MASK_OBSTACLES | COLLISION_MASK_GROUND | COLLISION_MASK_CHUNK);

        auto light = node_->CreateComponent<Light>();
        light->SetRadius(10.0f);
        light->SetLightType(LIGHT_POINT);

        auto *shape = node_->CreateComponent<CollisionShape>();
        shape->SetSphere(1.0f);
    } else {
        // Create rigidbody, and set non-zero mass so that the body becomes dynamic
        rigidBody_ = node_->CreateComponent<RigidBody>();
        rigidBody_->SetCollisionLayerAndMask(COLLISION_MASK_PLAYER | COLLISION_MASK_CHUNK_LOADER,
                                             COLLISION_MASK_PLAYER | COLLISION_MASK_CHECKPOINT |
                                             COLLISION_MASK_OBSTACLES | COLLISION_MASK_GROUND | COLLISION_MASK_CHUNK);
        rigidBody_->SetMass(1.0f);

        // Set zero angular factor so that physics doesn't turn the character on its own.
        // Instead we will control the character yaw manually
        rigidBody_->SetAngularFactor(Vector3::ZERO);

        // Set the rigidbody to signal collision also when in rest, so that we get ground collisions properly
        rigidBody_->SetCollisionEventMode(COLLISION_ALWAYS);

        // Set a capsule shape for collision
        auto* shape = node_->CreateComponent<CollisionShape>();
        shape->SetCapsule(0.7f, 1.8f, Vector3(0.0f, 0.9f, 0.0f));
    }

    SubscribeToEvent(node_, E_NODECOLLISION, URHO3D_HANDLER(Player, HandleNodeCollision));

    terrain_ = terrain;

    ResetPosition();
}

void Player::FindNode(Scene* scene, int id)
{
    node_ = scene->GetNode(id);
    if (node_) {
        node_->SetInterceptNetworkUpdate("Network Position", true);
        node_->GetComponent<PlayerState>()->HideLabel();
        rigidBody_ = node_->GetComponent<RigidBody>();
        SubscribeToEvent(E_INTERCEPTNETWORKUPDATE, URHO3D_HANDLER(Player, HandlePredictPlayerPosition));
    }
}

void Player::ResetPosition()
{
    Vector3 position = spawnPoint_;
    if (terrain_) {
        position.y_ = terrain_->GetHeight(position) + 1.0f;
    }
    GetNode()->SetWorldPosition(position);

    if (GetNode() && GetNode()->HasComponent<RigidBody>()) {
        GetNode()->GetComponent<RigidBody>()->SetLinearVelocity(Vector3::ZERO);
        GetNode()->GetComponent<RigidBody>()->SetAngularVelocity(Vector3::ZERO);
    }
}

void Player::SetControllerId(unsigned int id)
{
    controllerId_ = id;
}

SharedPtr<Node> Player::GetNode()
{
    return node_;
}

void Player::SetControllable(bool value)
{
    isControlled_ = value;
    if (isControlled_) {
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
#if !defined(__EMSCRIPTEN__)
    auto serverConnection = GetSubsystem<Network>()->GetServerConnection();
#endif

    Controls controls;
    float movementSpeed = MOVE_TORQUE;

    if (isControlled_) {
        if (connection_) {
            controls = connection_->GetControls();
        } else if (!IsCameraTargetSet()) {
            controls = GetSubsystem<ControllerInput>()->GetControls(controllerId_);
        }
    } else {
        controls = GetNode()->GetComponent<BehaviourTree>()->GetControls();
    }

    if (positionUI_) {
        Vector3 position = node_->GetWorldPosition();
        String content =
                "X:" + String(static_cast<int>(position.x_)) + " Y:" + String(static_cast<int>(position.y_)) + " Z:" +
                String(static_cast<int>(position.z_));
        positionUI_->SetText(content);
    }

    if (noclip_) {
        node_->SetRotation(Quaternion(controls.pitch_, controls.yaw_, 0.0f));
        // Movement speed as world units per second
        float MOVE_SPEED = NOCLIP_CAMERA_SPEED;
        if (controls.IsDown(CTRL_SPRINT)) {
            MOVE_SPEED *= 2;
        }
        // Read WASD keys and move the camera scene node to the corresponding direction if they are pressed
        if (controls.IsDown(CTRL_FORWARD)) {
            cameraInertia_ += Vector3::FORWARD * MOVE_SPEED * timeStep * 0.5f;
//            node_->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
        }
        if (controls.IsDown(CTRL_BACK)) {
//            node_->Translate(Vector3::BACK * MOVE_SPEED * timeStep);
            cameraInertia_ += Vector3::BACK * MOVE_SPEED * timeStep * 0.5f;
        }
        if (controls.IsDown(CTRL_LEFT)) {
//            node_->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
            cameraInertia_ += Vector3::LEFT * MOVE_SPEED * timeStep * 0.5f;
        }
        if (controls.IsDown(CTRL_RIGHT)) {
//            node_->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);
            cameraInertia_ += Vector3::RIGHT * MOVE_SPEED * timeStep * 0.5f;
        }

        cameraInertia_.x_ = Clamp(cameraInertia_.x_, -MOVE_SPEED, MOVE_SPEED);
        cameraInertia_.y_ = Clamp(cameraInertia_.y_, -MOVE_SPEED, MOVE_SPEED);
        cameraInertia_.z_ = Clamp(cameraInertia_.z_, -MOVE_SPEED, MOVE_SPEED);

        cameraInertia_ -= cameraInertia_ * timeStep * 1.0f / NOCLIP_CAMERA_INERTIA_TIME;
        node_->Translate(cameraInertia_);

        return;
    }

#if !defined(__EMSCRIPTEN__)
    if (serverConnection) {
        if (IsCameraTargetSet()) {
            // We are not following our player node, so we must not control it
            serverConnection->SetControls(Controls());
        } else {
            serverConnection->SetControls(controls);
        }
        return;
    }
#endif

//    if (node_->GetPosition().y_ < -SIZE_Y * 1.5) {
//        ResetPosition();
//
//        VariantMap &data = GetEventDataMap();
//        data["Player"] = controllerId_;
//        SendEvent("FallOffTheMap", data);
//        node_->GetComponent<PlayerState>()->AddScore(-10);
//    }

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
        rigidBody_->ApplyTorque(rotation * Vector3::RIGHT * movementSpeed * strength);
        if (!onGround_) {
            rigidBody_->ApplyForce(rotation * Vector3::FORWARD * movementSpeed * strength);
        }
    }
    if (controls.IsDown(CTRL_BACK)) {
        static StringHash backward(CTRL_BACK);
        float strength = 1.0f;
        if (controls.extraData_.Contains(backward)) {
            strength = controls.extraData_[backward].GetFloat();
        }
        rigidBody_->ApplyTorque(rotation * Vector3::LEFT * movementSpeed * strength);
        if (!onGround_) {
            rigidBody_->ApplyForce(rotation * Vector3::BACK * movementSpeed * strength);
        }
    }
    if (controls.IsDown(CTRL_LEFT)) {
        static StringHash left(CTRL_LEFT);
        float strength = 1.0f;
        if (controls.extraData_.Contains(left)) {
            strength = controls.extraData_[left].GetFloat();
        }
        rigidBody_->ApplyTorque(rotation * Vector3::FORWARD * movementSpeed * strength);
        if (!onGround_) {
            rigidBody_->ApplyForce(rotation * Vector3::LEFT * movementSpeed * strength);
        }
    }
    if (controls.IsDown(CTRL_RIGHT)) {
        static StringHash right(CTRL_RIGHT);
        float strength = 1.0f;
        if (controls.extraData_.Contains(right)) {
            strength = controls.extraData_[right].GetFloat();
        }
        rigidBody_->ApplyTorque(rotation * Vector3::BACK * movementSpeed * strength);
        if (!onGround_) {
            rigidBody_->ApplyForce(rotation * Vector3::RIGHT * movementSpeed * strength);
        }
    }
    if (controls.IsPressed(CTRL_JUMP, controls_) && onGround_) {
        rigidBody_->ApplyImpulse(Vector3::UP * JUMP_FORCE);
    }

    controls_ = controls;
    onGround_ = false;
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
        if (contactPosition.y_ < (node_->GetPosition().y_ + 1.0f))
        {
            float level = contactNormal.y_;
            if (level > 0.75)
                onGround_ = true;
        }
    }
}

void Player::SetClientConnection(Connection* connection)
{
    connection_ = connection;
    selectedItemUI_->Remove();
    selectedItemUI_.Reset();
    positionUI_->Remove();
    positionUI_.Reset();
}

void Player::SetCameraTarget(Node* target)
{
    cameraTarget_ = target;
}

Node* Player::GetCameraTarget()
{
    if (cameraTarget_) {
        return cameraTarget_;
    }

    return GetNode();
}

bool Player::IsCameraTargetSet()
{
    return cameraTarget_ && cameraTarget_ != node_ && !noclip_;
}

void Player::SetCameraDistance(float distance)
{
    cameraDistance_ = distance;
}

float Player::GetCameraDistance()
{
    return cameraDistance_;
}

void Player::SetName(const String& name)
{
    if (node_) {
        node_->GetOrCreateComponent<PlayerState>()->SetPlayerName(name);
    }
}

void Player::HandlePredictPlayerPosition(StringHash eventType, VariantMap& eventData)
{
    using namespace InterceptNetworkUpdate;
    String name = eventData[P_NAME].GetString();
    Node* node = static_cast<Node*>(eventData[P_SERIALIZABLE].GetPtr());
    Vector3 position = eventData[P_VALUE].GetVector3();
//    node_->SetWorldPosition(position);
//    URHO3D_LOGINFOF("HandlePredictPlayerPosition %s", (position - node->GetPosition()).ToString().CString());
    const AttributeInfo& attr = node->GetAttributes()->At(eventData[P_INDEX].GetInt());
    node->OnSetAttribute(attr, eventData[P_VALUE]);
}

void Player::SetSpawnPoint(Vector3 position)
{
    spawnPoint_ = position;
}

void Player::HandleMappedControlPressed(StringHash eventType, VariantMap& eventData)
{
    using namespace MappedControlPressed;
    int id = eventData[P_CONTROLLER].GetInt();
    if (id == controllerId_) {
        int action = eventData[P_ACTION].GetInt();
        if (action == CTRL_CHANGE_ITEM) {
#ifdef VOXEL_SUPPORT
            if (GetSubsystem<VoxelWorld>()) {
                selectedItem_++;
                if (selectedItem_ >= BlockType::BT_NONE) {
                    selectedItem_ = 1;
                }
                selectedItemUI_->SetText(GetSubsystem<VoxelWorld>()->GetBlockName(static_cast<BlockType>(selectedItem_)));
            }
#endif
        }
    }

}

int Player::GetSelectedItem()
{
    return selectedItem_;
}