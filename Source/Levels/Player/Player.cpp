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

static bool SHOW_LABELS = true;
static float MOVE_TORQUE = 20.0f;
static float JUMP_FORCE = 40.0f;

Player::Player(Context* context) :
    Object(context)
{
    SubscribeToEvent(E_PHYSICSPRESTEP, URHO3D_HANDLER(Player, HandlePhysicsPrestep));
    SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(Player, HandlePostUpdate));

    SendEvent(MyEvents::E_CONSOLE_COMMAND_ADD, MyEvents::ConsoleCommandAdd::P_NAME, "show_labels", MyEvents::ConsoleCommandAdd::P_EVENT, "#show_labels", MyEvents::ConsoleCommandAdd::P_DESCRIPTION, "Show player labels");
    SubscribeToEvent("#show_labels", [&](StringHash eventType, VariantMap& eventData) {
        StringVector params = eventData["Parameters"].GetStringVector();
        if (params.Size() != 2) {
            URHO3D_LOGERROR("Invalid number of arguments!");
            return;
        }
        SHOW_LABELS = ToBool(params[1]);
        _label->SetEnabled(SHOW_LABELS);
    });

    SendEvent(MyEvents::E_CONSOLE_COMMAND_ADD, MyEvents::ConsoleCommandAdd::P_NAME, "movement_speed", MyEvents::ConsoleCommandAdd::P_EVENT, "#movement_speed", MyEvents::ConsoleCommandAdd::P_DESCRIPTION, "Show player labels");
    SubscribeToEvent("#movement_speed", [&](StringHash eventType, VariantMap& eventData) {
        StringVector params = eventData["Parameters"].GetStringVector();
        if (params.Size() != 2) {
            URHO3D_LOGERROR("Invalid number of arguments!");
            return;
        }
        MOVE_TORQUE = ToFloat(params[1]);
    });

    SendEvent(MyEvents::E_CONSOLE_COMMAND_ADD, MyEvents::ConsoleCommandAdd::P_NAME, "jump_force", MyEvents::ConsoleCommandAdd::P_EVENT, "#jump_force", MyEvents::ConsoleCommandAdd::P_DESCRIPTION, "Show player labels");
    SubscribeToEvent("#jump_force", [&](StringHash eventType, VariantMap& eventData) {
        StringVector params = eventData["Parameters"].GetStringVector();
        if (params.Size() != 2) {
            URHO3D_LOGERROR("Invalid number of arguments!");
            return;
        }
        JUMP_FORCE = ToFloat(params[1]);
    });
}

void Player::RegisterObject(Context* context)
{
    context->RegisterFactory<Player>();
}

void Player::CreateNode(Scene* scene, unsigned int controllerId)
{
    SetControllerId(controllerId);

    auto* cache = GetSubsystem<ResourceCache>();

    // Create the scene node & visual representation. This will be a replicated object
    _node = scene->CreateChild("Player");
    _node->SetVar("Player", _controllerId);
    _node->SetPosition(Vector3(0, 2, 0));
    _node->SetScale(0.5f);

    auto* ballObject = _node->CreateComponent<StaticModel>();
    ballObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
    ballObject->SetMaterial(cache->GetResource<Material>("Materials/Player.xml"));
    ballObject->SetCastShadows(true);

    // Create the physics components
    _rigidBody = _node->CreateComponent<RigidBody>();
    _rigidBody->SetMass(5.0f);
    _rigidBody->SetFriction(2.0f);
    // In addition to friction, use motion damping so that the ball can not accelerate limitlessly
    _rigidBody->SetLinearDamping(0.8f);
    _rigidBody->SetAngularDamping(0.8f);
    _rigidBody->SetCollisionLayerAndMask(COLLISION_MASK_PLAYER, COLLISION_MASK_PLAYER | COLLISION_MASK_CHECKPOINT | COLLISION_MASK_OBSTACLES);
    _rigidBody->SetCollisionEventMode(CollisionEventMode::COLLISION_ALWAYS);

    auto* shape = _node->CreateComponent<CollisionShape>();
    shape->SetSphere(1.0f);

    // Create a random colored point light at the ball so that can see better where is going
    auto* light = _node->CreateComponent<Light>();
    light->SetRange(20.0f);
    light->SetColor(Color(0.5f + Random(0.5f), 0.5f + Random(0.5f), 0.5f + Random(0.5f)));
    light->SetCastShadows(false);

    _label = scene->CreateChild("Label");

    auto text3D = _label->CreateComponent<Text3D>();
    text3D->SetText("Player " + String(_controllerId));
    text3D->SetFont(cache->GetResource<Font>(APPLICATION_FONT), 30);
    text3D->SetColor(Color::GRAY);
    text3D->SetAlignment(HA_CENTER, VA_BOTTOM);
    text3D->SetFaceCameraMode(FaceCameraMode::FC_LOOKAT_Y);
    text3D->SetViewMask(~(1 << _controllerId));

    if (!SHOW_LABELS) {
        _label->SetEnabled(false);
    }

    SubscribeToEvent(_node, E_NODECOLLISION, URHO3D_HANDLER(Player, HandleNodeCollision));
}

void Player::SetControllerId(unsigned int id)
{
    _controllerId = id;
}

Node* Player::GetNode()
{
    return _node.Get();
}

void Player::HandlePhysicsPrestep(StringHash eventType, VariantMap& eventData)
{
    using namespace PhysicsPreStep;
    float timeStep = eventData[P_TIMESTEP].GetFloat();

    if (_node->GetPosition().y_ < -10) {
        _node->SetPosition(Vector3(0, 1, 0));
        _node->GetComponent<RigidBody>()->SetLinearVelocity(Vector3::ZERO);
        _node->GetComponent<RigidBody>()->SetAngularVelocity(Vector3::ZERO);
        SendEvent("FallOffTheMap");
    }

    float movementSpeed = MOVE_TORQUE;
    Controls controls = GetSubsystem<ControllerInput>()->GetControls(_controllerId);
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
    _label->SetPosition(_node->GetPosition() + Vector3::UP * 0.2);
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