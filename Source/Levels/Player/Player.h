#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Input/Controls.h>
#include <Urho3D/Graphics/Terrain.h>
#include <Urho3D/Network/Connection.h>

using namespace Urho3D;

class Player : public Object
{
    URHO3D_OBJECT(Player, Object);

public:
    explicit Player(Context* context);
    ~Player();

    static void RegisterObject(Context* context);

    /**
     * Create player controlled node
     */
    void CreateNode(Scene* scene, int controllerId, Terrain* terrain);

    /**
     * Set controller ID to know which controller is controlling this player
     */
    void SetControllerId(unsigned int id);

    /**
     * Get created player node
     */
    Node* GetNode();

    /**
     * Set if this instance can be controleld with the user input
     */
    void SetControllable(bool value);

    void SetClientConnection(Connection* connection);
    void SetServerConnection(Connection* connection);

private:

    void HandlePhysicsPrestep(StringHash eventType, VariantMap& eventData);
    void HandlePostUpdate(StringHash eventType, VariantMap& eventData);

    void UpdatePlayerList(bool remove = false);

    /**
     * Detect when player is on the ground or not
     */
    void HandleNodeCollision(StringHash eventType, VariantMap& eventData);

    RigidBody* _rigidBody;
    SharedPtr<Node> _node;

    void ResetPosition();

    void SetLabel();
    
    /**
     * Controller ID
     */
    int _controllerId;

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

    Terrain* _terrain;

    Connection* _connection{nullptr};
    Connection* _serverConnection{nullptr};

    bool _isControlled{false};
};
