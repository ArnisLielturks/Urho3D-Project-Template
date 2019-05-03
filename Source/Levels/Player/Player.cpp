#include <Urho3D/Urho3DAll.h>
#include "Player.h"
#include "../../MyEvents.h"
#include "../../Global.h"
#include "../../Input/ControllerInput.h"

Player::Player(Context* context) :
    Object(context)
{
    SubscribeToEvent(E_PHYSICSPRESTEP, URHO3D_HANDLER(Player, HandlePhysicsPrestep));
    SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(Player, HandlePostUpdate));
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
    light->SetRange(5.0f);
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

    // Movement speed as world units per second
    float MOVE_TORQUE = 20.0f;
    Controls controls = GetSubsystem<ControllerInput>()->GetControls(_controllerId);
    if (controls.IsDown(CTRL_SPRINT)) {
        MOVE_TORQUE = 40.0f;
    }

    // Torque is relative to the forward vector
    Quaternion rotation(0.0f, controls.yaw_, 0.0f);
    if (controls.IsDown(CTRL_FORWARD)) {
        _rigidBody->ApplyTorque(rotation * Vector3::RIGHT * MOVE_TORQUE);
        //   _cameras[playerId]->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
    }
    if (controls.IsDown(CTRL_BACK)) {
        _rigidBody->ApplyTorque(rotation * Vector3::LEFT * MOVE_TORQUE);
        //_cameras[playerId]->Translate(Vector3::BACK * MOVE_SPEED * timeStep);
    }
    if (controls.IsDown(CTRL_LEFT)) {
        _rigidBody->ApplyTorque(rotation * Vector3::FORWARD * MOVE_TORQUE);
        //_cameras[playerId]->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
    }
    if (controls.IsDown(CTRL_RIGHT)) {
        _rigidBody->ApplyTorque(rotation * Vector3::BACK * MOVE_TORQUE);
        // _cameras[playerId]->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);
    }
    if (controls.IsDown(CTRL_JUMP)) {
        //            body->ApplyImpulse(Vector3(0, 10, 0));
                    // _cameras[playerId]->Translate(Vector3::UP * MOVE_SPEED * timeStep);
    }
}

void Player::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{
    _label->SetPosition(_node->GetPosition() + Vector3::UP * 0.2);
}