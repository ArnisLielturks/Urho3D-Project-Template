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

    void FindNode(Scene* scene, int id);

    /**
     * Set controller ID to know which controller is controlling this player
     */
    void SetControllerId(unsigned int id);

    int GetControllerId() { return controllerId_; }

    void SetName(const String& name);

    /**
     * Get created player node
     */
    Node* GetNode();

    /**
     * Set if this instance can be controleld with the user input
     */
    void SetControllable(bool value);

    void SetClientConnection(Connection* connection);

    void SetCameraTarget(Node* target);
    Node* GetCameraTarget();

    void SetCameraDistance(float distance);
    float GetCameraDistance();

    void SetSpawnPoint(Vector3 position);

private:

    bool IsCameraTargetSet();
    void HandlePhysicsPrestep(StringHash eventType, VariantMap& eventData);
    void RegisterConsoleCommands();

    /**
     * Detect when player is on the ground or not
     */
    void HandleNodeCollision(StringHash eventType, VariantMap& eventData);

    void HandlePredictPlayerPosition(StringHash eventType, VariantMap& eventData);

    void ResetPosition();

    RigidBody* rigidBody_;
    SharedPtr<Node> node_;

    WeakPtr<Node> cameraTarget_;
    float cameraDistance_{1.5f};
    
    /**
     * Controller ID
     */
    int controllerId_;

    /**
     * Player controlers
     */
    Controls controls_;

    /**
     * Is the player on the ground
     */
    bool onGround_;

    Terrain* terrain_{nullptr};

    Connection* connection_{nullptr};

    bool isControlled_{false};

    Vector3 spawnPoint_;
};
