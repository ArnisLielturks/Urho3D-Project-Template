// Rotator script object class. Script objects to be added to a scene node must implement the empty ScriptObject interface
class Destroyer : ScriptObject
{
    Vector3 rotationSpeed = Vector3(0.1, 0.2, 0.3);

    void Start()
    {
        // Subscribe physics collisions that concern this scene node
        SubscribeToEvent(node, "NodeCollision", "HandleNodeCollision");
    }

    void SetRotationSpeed(const Vector3&in speed)
    {
        rotationSpeed = speed;
    }

    void HandleNodeCollision(StringHash eventType, VariantMap& eventData)
    {
        // Get the other colliding body, make sure it is moving (has nonzero mass)
        RigidBody@ otherBody = eventData["OtherBody"].GetPtr();

        if (otherBody.mass > 0.0f)
        {
            RigidBody@ body = node.GetComponent("RigidBody");
            body.ApplyImpulse(Vector3(0, 2.0, 0));
            node.scale = node.scale * 0.9;

            if (node.scale.length < 1.0) {
                VariantMap data;
                data["Message"] = "Destroyed node " + String(node.id);
                SendEvent("ShowNotification", data);                
                node.Remove();

                data["Type"] = SOUND_EFFECT;
                data["SoundFile"] = "Data/Sounds/achievement.wav";
                SendEvent("PlaySound", data);

                SendEvent("BoxDestroyed");
            } else {
                VariantMap data;
                data["Type"] = SOUND_EFFECT;
                data["SoundFile"] = "Data/Sounds/PlayerFistHit.wav";
                SendEvent("PlaySound", data);
            }
        }
    }
}
