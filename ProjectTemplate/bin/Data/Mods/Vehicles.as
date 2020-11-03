const float ENGINE_FORCE = 5000.0f;
const float DOWN_FORCE = 1000.0f;
const float MAX_WHEEL_ANGLE = 22.5f;
const float CHASSIS_WIDTH = 2.6f;

const int CTRL_FORWARD = 1;
const int CTRL_BACK = 2;
const int CTRL_LEFT = 4;
const int CTRL_RIGHT = 8;
const int CTRL_BRAKE = 16;

Node@ vehicleNode;

void Start()
{
    // Disable vehicle mod for now
    return;
    SubscribeToEvent("CreateVehicles", "HandleCreateVehicles");

    VariantMap data;
    data["Name"] = "Creating vehicles";
    data["Event"] = "CreateVehicles";
    SendEvent("RegisterLoadingStep", data);

    if (vehicleNode is null) {
        vehicleNode = scene.GetChild("Vehicle", true);
    }
}

void Stop()
{
    if (vehicleNode !is null) {
        vehicleNode.Remove();
    } else {
        vehicleNode = scene.GetChild("Vehicle", true);
        if (vehicleNode !is null) {
            vehicleNode.Remove();
        }
    }
}

void HandleCreateVehicles()
{
    VariantMap data;
    data["Event"] = "CreateVehicles";

    // Sent event to let the system know that we will handle this loading step
    SendEvent("AckLoadingStep", data);

    CreateVehicle();

    SendEvent("LoadingStepFinished", data);
}

void CreateVehicle()
{
    vehicleNode = scene.CreateChild("Vehicle");
    vehicleNode.position = Vector3(0.0f, 5.0f, 0.0f);
    // First createing player-controlled vehicle
    // Create the vehicle logic script object
    Vehicle@ vehicle = cast<Vehicle>(vehicleNode.CreateScriptObject(scriptFile, "Vehicle"));
    // Initialize vehicle component
    vehicle.Init();
    vehicleNode.AddTag("vehicle");
    // Set RigidBody physics parameters
    // (The RigidBody was created by vehicle.Init()
    RigidBody@ hullBody = vehicleNode.GetComponent("RigidBody");
    hullBody.mass = 800.0f;
    hullBody.linearDamping = 0.2f;     // Some air resistance
    hullBody.angularDamping = 0.5f;
    hullBody.collisionLayer = 1;
}

// Vehicle script object class
//
// When saving, the node and component handles are automatically converted into nodeID or componentID attributes
// and are acquired from the scene when loading. The steering member variable will likewise be saved automatically.
// The Controls object can not be automatically saved, so handle it manually in the Load() and Save() methods

class Vehicle:ScriptObject
{

    RigidBody@ hullBody;

    // Current left/right steering amount (-1 to 1.)
    float steering = 0.0f;

    // Vehicle controls.
    Controls controls;

    float suspensionRestLength = 0.6f;
    float suspensionStiffness = 14.0f;
    float suspensionDamping = 2.0f;
    float suspensionCompression = 4.0f;
    float wheelFriction = 1000.0f;
    float rollInfluence = 0.12f;
    float maxEngineForce = ENGINE_FORCE;

    float wheelWidth = 0.4f;
    float wheelRadius = 0.5f;
    float brakingForce = 50.0f;
    Array<Vector3> connectionPoints;
    Array<Node@> particleEmitterNodeList;
    protected Vector3 prevVelocity;

    void Load(Deserializer& deserializer)
    {
        controls.yaw = deserializer.ReadFloat();
        controls.pitch = deserializer.ReadFloat();
    }

    void Save(Serializer& serializer)
    {
        serializer.WriteFloat(controls.yaw);
        serializer.WriteFloat(controls.pitch);
    }

    void Init()
    {
        // This function is called only from the main program when initially creating the vehicle, not on scene load
        StaticModel@ hullObject = node.CreateComponent("StaticModel");
        hullBody = node.CreateComponent("RigidBody");
        CollisionShape@ hullShape = node.CreateComponent("CollisionShape");
        node.scale = Vector3(2.3f, 1.0f, 4.0f);
        hullObject.model = cache.GetResource("Model", "Models/Box.mdl");
        hullObject.material = cache.GetResource("Material", "Materials/Stone.xml");
        hullObject.castShadows = true;
        hullShape.SetBox(Vector3(1.0f, 1.0f, 1.0f));
        hullBody.mass = 800.0f;
        hullBody.linearDamping = 0.2f; // Some air resistance
        hullBody.angularDamping = 0.5f;
        hullBody.collisionLayer = 1;
        RaycastVehicle@ raycastVehicle = node.CreateComponent("RaycastVehicle");
        raycastVehicle.Init();
        connectionPoints.Reserve(4);
        float connectionHeight = -0.4f;      //1.2f;
        bool isFrontWheel = true;
        Vector3 wheelDirection(0, -1, 0);
        Vector3 wheelAxle(-1, 0, 0);
        float wheelX = CHASSIS_WIDTH / 2.0 - wheelWidth;
        // Front left
        connectionPoints.Push(Vector3(-wheelX, connectionHeight, 2.5f - wheelRadius * 2.0f));
        // Front right
        connectionPoints.Push(Vector3(wheelX, connectionHeight, 2.5f - wheelRadius * 2.0f));
        // Back left
        connectionPoints.Push(Vector3(-wheelX, connectionHeight, -2.5f + wheelRadius * 2.0f));
        // Back right
        connectionPoints.Push(Vector3(wheelX, connectionHeight, -2.5f + wheelRadius * 2.0f));
        const Color LtBrown(0.972f, 0.780f, 0.412f);
        for (int id = 0; id < connectionPoints.length; id++)
        {
            Node@ wheelNode = scene.CreateChild();
            Vector3 connectionPoint = connectionPoints[id];
            // Front wheels are at front (z > 0)
            // back wheels are at z < 0
            // Setting rotation according to wheel position
            bool isFrontWheel = connectionPoints[id].z > 0.0f;
            wheelNode.rotation = (connectionPoint.x >= 0.0 ? Quaternion(0.0f, 0.0f, -90.0f) : Quaternion(0.0f, 0.0f, 90.0f));
            wheelNode.worldPosition = (node.worldPosition + node.worldRotation * connectionPoints[id]);
            wheelNode.scale = Vector3(1.0f, 0.65f, 1.0f);
            raycastVehicle.AddWheel(wheelNode, wheelDirection, wheelAxle, suspensionRestLength, wheelRadius, isFrontWheel);
            raycastVehicle.SetWheelSuspensionStiffness(id, suspensionStiffness);
            raycastVehicle.SetWheelDampingRelaxation(id, suspensionDamping);
            raycastVehicle.SetWheelDampingCompression(id, suspensionCompression);
            raycastVehicle.SetWheelFrictionSlip(id, wheelFriction);
            raycastVehicle.SetWheelRollInfluence(id, rollInfluence);
            StaticModel@ pWheel = wheelNode.CreateComponent("StaticModel");
            pWheel.model = cache.GetResource("Model", "Models/Cylinder.mdl");
            pWheel.material = cache.GetResource("Material", "Materials/Stone.xml");
            pWheel.castShadows = true;
        }
        CreateEmitters();
        raycastVehicle.ResetWheels();
    }

    void CreateEmitter(Vector3 place)
    {
        Node@ emitter = scene.CreateChild();
        emitter.worldPosition = node.worldPosition + node.worldRotation * place + Vector3(0, -wheelRadius, 0);
        ParticleEmitter@ particleEmitter = emitter.CreateComponent("ParticleEmitter");
        particleEmitter.effect = cache.GetResource("ParticleEffect", "Particle/Dust.xml");
        particleEmitter.emitting = false;
        particleEmitterNodeList.Push(emitter);
        emitter.temporary = true;
    }

    void CreateEmitters()
    {
        particleEmitterNodeList.Clear();
        RaycastVehicle@ raycastVehicle = node.GetComponent("RaycastVehicle");
        for (int id = 0; id < raycastVehicle.numWheels; id++)
        {
            Vector3 connectionPoint = raycastVehicle.GetWheelConnectionPoint(id);
            CreateEmitter(connectionPoint);
        }
    }

    void FixedUpdate(float timeStep)
    {
        float newSteering = 0.0f;
        float accelerator = 0.0f;
        bool brake = false;
        RaycastVehicle@ raycastVehicle = node.GetComponent("RaycastVehicle");
        if (controls.IsDown(CTRL_LEFT))
            newSteering = -1.0f;
        // if (controls.IsDown(CTRL_RIGHT))
            newSteering = 1.0f;
        // if (controls.IsDown(CTRL_FORWARD))
            accelerator = 1.0f;
        if (controls.IsDown(CTRL_BACK))
            accelerator = -0.5f;
        if (controls.IsDown(CTRL_BRAKE))
            brake = true;
        // When steering, wake up the wheel rigidbodies so that their orientation is updated
        if (newSteering != 0.0f)
        {
            steering = steering * 0.95f + newSteering * 0.05f;
        }
        else
            steering = steering * 0.8f + newSteering * 0.2f;
        Quaternion steeringRot(0.0f, steering * MAX_WHEEL_ANGLE, 0.0f);
        raycastVehicle.SetSteeringValue(0, steering);
        raycastVehicle.SetSteeringValue(1, steering);
        raycastVehicle.SetEngineForce(2, maxEngineForce * accelerator);
        raycastVehicle.SetEngineForce(3, maxEngineForce * accelerator);
        for (int i = 0; i < raycastVehicle.numWheels; i++)
        {
            if (brake)
            {
                raycastVehicle.SetBrake(i, brakingForce);
            }
            else
            {
                raycastVehicle.SetBrake(i, 0.0f);
            }
        }
        // Apply downforce proportional to velocity
        Vector3 localVelocity = hullBody.rotation.Inverse() * hullBody.linearVelocity;
        hullBody.ApplyForce(hullBody.rotation * Vector3(0.0f, -1.0f, 0.0f) * Abs(localVelocity.z) * DOWN_FORCE);
    }

    void PostUpdate(float timeStep)
    {
        if (particleEmitterNodeList.length == 0)
            return;
        RaycastVehicle@ raycastVehicle = node.GetComponent("RaycastVehicle");
        RigidBody@ vehicleBody = node.GetComponent("RigidBody");
        Vector3 velocity = hullBody.linearVelocity;
        Vector3 accel = (velocity - prevVelocity) / timeStep;
        float planeAccel = Vector3(accel.x, 0.0f, accel.z).length;
        for (int i = 0; i < raycastVehicle.numWheels; i++)
        {
            Node@ emitter = particleEmitterNodeList[i];
            ParticleEmitter@ particleEmitter = emitter.GetComponent("ParticleEmitter");
            if (raycastVehicle.WheelIsGrounded(i) && (raycastVehicle.GetWheelSkidInfoCumulative(i) < 0.9f || raycastVehicle.GetBrake(i) > 2.0f ||
                planeAccel > 15.0f))
            {
                emitter.worldPosition = raycastVehicle.GetContactPosition(i);
                if (!particleEmitter.emitting)
                    particleEmitter.emitting = true;
            }
            else if (particleEmitter.emitting)
                particleEmitter.emitting = false;
        }
        prevVelocity = velocity;
    }
}
