class Drop : ScriptObject
{
    VariantMap nodePositions;
    void Start()
    {
        node.AddTag("Finish");
        // Subscribe physics collisions that concern this scene node
        SubscribeToEvent(node, "NodeCollisionStart", "HandleNodeCollision");
    }

    void HandleNodeCollision(StringHash eventType, VariantMap& eventData)
    {
        // Get the other colliding body, make sure it is moving (has nonzero mass)
        RigidBody@ otherBody = eventData["OtherBody"].GetPtr();

        if (otherBody.mass > 0.0f)
        {
            if (otherBody.node.vars.Contains("Player")) {
                VectorBuffer contacts = eventData["Contacts"].GetBuffer();
                VariantMap data;
                data["Score"] = -20;
                otherBody.node.SendEvent("PlayerScoreAdd", data);

                Vector3 pos = otherBody.node.vars["Respawn"].GetVector3();
                otherBody.node.worldPosition = pos;
            }
        }
    }
}
