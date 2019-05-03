#pragma once

#include <Urho3D/Urho3DAll.h>

class Player : public Object
{
    URHO3D_OBJECT(Player, Object);

public:
    explicit Player(Context* context);

    static void RegisterObject(Context* context);

    /**
     * Create player controlled node
     */
    void CreateNode(Scene* scene, unsigned int controllerId);

    /**
     * Set controller ID to know which controller is controlling this player
     */
    void SetControllerId(unsigned int id);

    /**
     * Get created player node
     */
    Node* GetNode();
private:

    void HandlePhysicsPrestep(StringHash eventType, VariantMap& eventData);
    void HandlePostUpdate(StringHash eventType, VariantMap& eventData);

    /**
     * Detect when player is on the ground or not
     */
    void HandleNodeCollision(StringHash eventType, VariantMap& eventData);

    RigidBody* _rigidBody;
    SharedPtr<Node> _node;
    
    /**
     * Controller ID
     */
    unsigned int _controllerId;

    /**
     * 3D Text player label node
     */
    SharedPtr<Node> _label;

    /**
     * Player controlers
     */
    Controls _controls;

    /**
     * Is the player on the ground
     */
    bool _onGround;

};
