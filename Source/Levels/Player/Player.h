#pragma once

#include <Urho3D/Urho3DAll.h>

class Player : public Object
{
    URHO3D_OBJECT(Player, Object);

public:
    /// Construct.
    explicit Player(Context* context);

    static void RegisterObject(Context* context);

    void CreateNode(Scene* scene, unsigned int controllerId);

    void SetControllerId(unsigned int id);

    Node* GetNode();
private:

    void HandlePhysicsPrestep(StringHash eventType, VariantMap& eventData);
    void HandlePostUpdate(StringHash eventType, VariantMap& eventData);
    void HandleNodeCollision(StringHash eventType, VariantMap& eventData);

    RigidBody* _rigidBody;
    SharedPtr<Node> _node;
    unsigned int _controllerId;
    SharedPtr<Node> _label;
    Controls _controls;
    bool _onGround;

};
