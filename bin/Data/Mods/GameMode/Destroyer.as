#include "GameObject.as"

class Destroyer : ScriptObject
{
    void Start()
    {
        node.AddTag("Cube");
        // Subscribe physics collisions that concern this scene node
        SubscribeToEvent(node, "NodeCollisionStart", "HandleNodeCollision");
    }

    void HandleNodeCollision(StringHash eventType, VariantMap& eventData)
    {
        // Get the other colliding body, make sure it is moving (has nonzero mass)
        RigidBody@ otherBody = eventData["OtherBody"].GetPtr();

        if (otherBody.mass > 0.0f)
        {
            VectorBuffer contacts = eventData["Contacts"].GetBuffer();

            RigidBody@ body = node.GetComponent("RigidBody");
            while (!contacts.eof)
            {
                Vector3 contactPosition = contacts.ReadVector3();
                Vector3 contactNormal = contacts.ReadVector3();
                float contactDistance = contacts.ReadFloat();
                float contactImpulse = contacts.ReadFloat();
                if (otherBody.node.name.Contains("Player")) {
                    body.ApplyImpulse(contactNormal * contactImpulse * 10);
                }
            }

            // node.scale -= Vector3(0.5, 0.5, 0.5);

            if (otherBody.node.name.Contains("Player")) {
                node.vars["LastTouchedNode"] = otherBody.node;
            }

            if (otherBody.node.name == "Character") {
                SendEvent("BoxDestroyed");
                node.Remove();
            }

            VariantMap data;
            data["Type"] = SOUND_EFFECT;
            data["SoundFile"] = "Sounds/kick.wav";
            SendEvent("PlaySound", data);
        }
    }

    // Update is called during the variable timestep scene update
    void Update(float timeStep)
    {
        if (node.position.y < -4) {
            VariantMap data;
            data["Type"] = SOUND_EFFECT;
            data["SoundFile"] = "Sounds/achievement.wav";
            SendEvent("PlaySound", data);

            Node@ playerNode = node.vars["LastTouchedNode"].GetPtr();
            if (playerNode !is null) {
                Node@ playerNode = node.vars["LastTouchedNode"].GetPtr();
                data["Score"] = 1;
                playerNode.SendEvent("PlayerScoreAdd", data);
            }
            SendEvent("BoxDestroyed");
            SendEvent("BoxDropped", data);

            Burst();
            node.Remove();
        }
    }

    void Burst()
    {
        Node@ emitter = scene.CreateChild("ParticleEffect");
        emitter.worldPosition = node.worldPosition;
        ParticleEmitter@ particleEmitter = emitter.CreateComponent("ParticleEmitter");
        particleEmitter.effect = cache.GetResource("ParticleEffect", "Particle/Burst.xml");
        particleEmitter.emitting = true;
        //particleEmitterNodeList.Push(emitter);
        emitter.temporary = true;
        // particleEmitter.autoRemoveMode = REMOVE_NODE;
        GameObject@ object = cast<GameObject>(emitter.CreateScriptObject(scriptFile, "GameObject", LOCAL));
        object.duration = 5.0f;
    }
}
