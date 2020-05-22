int count = 0;
Node@ ground = null;
Vector3 targetGroundScale = Vector3(40, 40, 40);

void Start()
{
    SubscribeToEvent("LoadGamemode", "HandleLoadGameMode");
    SubscribeToEvent("BoxDestroyed", "HandleBoxDestroyed");
    SubscribeToEvent("BoxDropped", "HandleBoxDropped");
    SubscribeToEvent("CheckpointReached", "HandleCheckpointReached");
    SubscribeToEvent("Update", "HandleUpdate");
    SubscribeToEvent("FallOffTheMap", "HandleFallOfMap");

    // Register our loading step
    VariantMap data;
    data["Name"] = "Initializing GameMode";
    data["Event"] = "LoadGamemode";
    SendEvent("RegisterLoadingStep", data);

    data["Event"] = "BoxDestroyed";
    data["Message"] = "Destroy 1 box";
    data["Image"] = "Textures/Achievements/trophy-cup.png";
    data["Threshold"] = 1;
    SendEvent("AddAchievement", data);

    for (uint i = 2; i < 10; i++) {
        data["Message"] = "Destroy " + String(i) + " boxes";
        data["Threshold"] = i;
        SendEvent("AddAchievement", data);
    }

    data["Event"] = "BoxDropped";
    data["Message"] = "Drop 1 box";
    data["Threshold"] = 1;
    SendEvent("AddAchievement", data);

    for (uint i = 2; i < 10; i++) {
        data["Message"] = "Drop " + String(i) + " boxes";
        data["Threshold"] = i;
        SendEvent("AddAchievement", data);
    }

    data["Event"] = "CheckpointReached";
    data["Message"] = "Reach 1 checkpoint";
    data["Threshold"] = 1;
    SendEvent("AddAchievement", data);

    for (uint i = 2; i < 10; i++) {
        data["Message"] = "Reach " + String(i) + " checkpoints";
        data["Threshold"] = i;
        SendEvent("AddAchievement", data);
    }
}

void Stop()
{

}

void CreateCheckpoint()
{
    Connection@ serverConnection = network.serverConnection;
    if (serverConnection !is null) {
        return;
    }
    XMLFile@ xml = cache.GetResource("XMLFile", "Mods/GameMode/Checkpoint.xml");
    float scale = targetGroundScale.x * 0.5;
    scene.InstantiateXML(xml.root, Vector3(Random(scale * 2) - scale, 2.0, Random(scale * 2) - scale), Quaternion());
}

void UpdateBoxes()
{
    Connection@ serverConnection = network.serverConnection;
    if (serverConnection !is null) {
        return;
    }
    Array<Node@>@ cubes = scene.GetChildrenWithTag("Cube", true);
    for (int i = 0; i < cubes.length; i++)
    {
        RigidBody@ body = cubes[i].GetComponent("RigidBody");
        body.Activate();
        body.ApplyImpulse(Vector3(0, 10, 0));
    }
}

void CreateObject()
{
    Connection@ serverConnection = network.serverConnection;
    if (serverConnection !is null) {
        return;
    }

    int limit = 20;
    if (network.serverRunning) {
        limit = 4;
    }
    if (count > limit) {
        log.Info("Box limit reached, current count=" + String(count));
        return;
    }
    XMLFile@ xml = cache.GetResource("XMLFile", "Mods/GameMode/DestroyCube.xml");
    scene.InstantiateXML(xml.root, Vector3(Random(30.0f) - 15.0f, 50.0f, Random(30.0f) - 15.0f), Quaternion());
    count++;
}
/**
 * Output debug message when level changing is requested
 */
void HandleLoadGameMode(StringHash eventType, VariantMap& eventData)
{
    VariantMap data;
    data["Event"] = "LoadGamemode";

    ground = scene.GetChild("Ground", true);

    // Sent event to let the system know that we will handle this loading step
    SendEvent("AckLoadingStep", data);

    for (uint i = 0; i < 4; i++) {
        CreateObject();
    }
    CreateCheckpoint();

    count = 0;

    // Let the loading system know that we finished our work
    SendEvent("LoadingStepFinished", data);
}

void HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    Connection@ serverConnection = network.serverConnection;
    if (serverConnection !is null) {
        return;
    }
    if (ground is null) {
        return;
    }
    float timestep = eventData["TimeStep"].GetFloat();
    float diff = (targetGroundScale.x - ground.scale.x);
    if (Abs(diff) > 0.1) {
        ground.scale += Vector3(1, 1, 1) * diff * timestep;
    } else {
        UpdateBoxes();
        UnsubscribeFromEvent("Update");
    }
}

void UpdatePlayerScore(int playerId, int points)
{
    Connection@ serverConnection = network.serverConnection;
    if (serverConnection !is null) {
        return;
    }
    if (playerId < 0) {
        return;
    }

    VariantMap data;
    data["ID"] = playerId;
    data["Score"] = points;
    SendEvent("PlayerScoreChanged", data);

    data["Message"] = "Player [" + String(playerId) + "] got " + String(points) + " points!";
    if (points < 0) {
        data["Message"] = "Player [" + String(playerId) + "] lost " + String(Abs(points)) + " points!";
        data["Status"] = "Warning";
    }
    SendEvent("ShowNotification", data);
}

void HandleBoxDestroyed(StringHash eventType, VariantMap& eventData)
{
    Connection@ serverConnection = network.serverConnection;
    if (serverConnection !is null) {
        return;
    }

    count--;
    CreateObject();
}

void HandleCheckpointReached(StringHash eventType, VariantMap& eventData)
{
    Connection@ serverConnection = network.serverConnection;
    if (serverConnection !is null) {
        return;
    }

    for( uint i = 0; i < 5; i++) {
        CreateObject();
    }
    CreateCheckpoint();
    // if (ground !is null) {
    //     if (ground.scale.x > 10) {
    //         targetGroundScale = ground.scale * 0.9;
    //         CreateCheckpoint();
    //         SubscribeToEvent("Update", "HandleUpdate");
    //     } else {
    //         targetGroundScale = Vector3(40, 40, 40);
    //         DelayedExecute(2.0, false, "void CreateCheckpoint()");
    //     }
    // }
}

void HandleBoxDropped(StringHash eventType, VariantMap& eventData)
{
    Connection@ serverConnection = network.serverConnection;
    if (serverConnection !is null) {
        return;
    }
    int playerId = eventData["Player"].GetInt();

    UpdatePlayerScore(playerId, 2);
}

void HandleFallOfMap(StringHash eventType, VariantMap& eventData)
{
    Connection@ serverConnection = network.serverConnection;
    if (serverConnection !is null) {
        return;
    }
    int playerId = eventData["Player"].GetInt();

    UpdatePlayerScore(playerId, -20);
}
