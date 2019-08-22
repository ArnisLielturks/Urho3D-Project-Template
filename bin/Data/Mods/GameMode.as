uint count = 0;
Node@ ground = null;
Vector3 targetGroundScale = Vector3(40, 40, 40);

void Start()
{
	SubscribeToEvent("LoadGamemode", "HandleLoadGameMode");
    SubscribeToEvent("BoxDestroyed", "HandleBoxDestroyed");
    SubscribeToEvent("BoxDropped", "HandleBoxDropped");
    SubscribeToEvent("CheckpointReached", "HandleCheckpointReached");
    SubscribeToEvent("Update", "HandleUpdate");

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
    XMLFile@ xml = cache.GetResource("XMLFile", "Mods/GameMode/Checkpoint.xml");
    float scale = targetGroundScale.x * 0.5;
    scene.InstantiateXML(xml.root, Vector3(Random(scale * 2) - scale, 0, Random(scale * 2) - scale), Quaternion());
}

void UpdateBoxes()
{
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
    if (count > 40) {
        log.Info("Box limit reached");
        return;
    }
    XMLFile@ xml = cache.GetResource("XMLFile", "Mods/GameMode/DestroyCube.xml");
    scene.InstantiateXML(xml.root, Vector3(Random(30.0f) - 15.0f, 2.0f, Random(30.0f) - 15.0f), Quaternion());
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

    for (uint i = 0; i < 30; i++) {
        CreateObject();
    }
    CreateCheckpoint();

    // Let the loading system know that we finished our work
    SendEvent("LoadingStepFinished", data);
}

void HandleUpdate(StringHash eventType, VariantMap& eventData)
{
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
    if (playerId < 0) {
        return;
    }
    String playerScoreId = "Player" + String(playerId) + "Score";
    int newScore = GetGlobalVar(playerScoreId).GetInt() + points;
    SetGlobalVar(playerScoreId, newScore);
    SendEvent("PlayerScoresUpdated");

    VariantMap data;
    data["Message"] = "Player [" + String(playerId) + "] got " + String(points) + " points!";
    SendEvent("ShowNotification", data);
}

void HandleBoxDestroyed(StringHash eventType, VariantMap& eventData)
{
    int playerId = eventData["Player"].GetInt();

    count--;
    for (uint i = 0; i < 2; i++) {
        CreateObject();
    }

    UpdatePlayerScore(playerId, 1);
}

void HandleCheckpointReached(StringHash eventType, VariantMap& eventData)
{
    int playerId = eventData["Player"].GetInt();

    UpdatePlayerScore(playerId, 5);

    for( uint i = 0; i < 5; i++) {
        CreateObject();
    }

    if (ground !is null) {
        if (ground.scale.x > 10) {
            targetGroundScale = ground.scale * 0.9;
            CreateCheckpoint();
            SubscribeToEvent("Update", "HandleUpdate");
        } else {
            targetGroundScale = Vector3(40, 40, 40);
            DelayedExecute(2.0, false, "void CreateCheckpoint()");
        }
    }
}

void HandleBoxDropped(StringHash eventType, VariantMap& eventData)
{
    int playerId = eventData["Player"].GetInt();

    UpdatePlayerScore(playerId, 2);
}