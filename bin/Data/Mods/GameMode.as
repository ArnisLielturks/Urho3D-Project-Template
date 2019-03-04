uint count = 0;


void Start()
{
	SubscribeToEvent("LoadGamemode", "HandleLoadGameMode");
    SubscribeToEvent("BoxDestroyed", "HandleBoxDestroyed");
    SubscribeToEvent("BoxDropped", "HandleBoxDropped");
    SubscribeToEvent("CheckpointReached", "HandleCheckpointReached");

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
    scene.InstantiateXML(xml.root, Vector3(Random(30.0f) - 15.0f, 5.0f, Random(30.0f) - 15.0f), Quaternion());
}

void CreateObject()
{
    if (count > 200) {
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

    // Sent event to let the system know that we will handle this loading step
    SendEvent("AckLoadingStep", data);

    for (uint i = 0; i < 30; i++) {
        CreateObject();
    }
    CreateCheckpoint();

    // Let the loading system know that we finished our work
    SendEvent("LoadingStepFinished", data);
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
    CreateCheckpoint();

    UpdatePlayerScore(playerId, 5);

    for( uint i = 0; i < 5; i++) {
        CreateObject();
    }
}

void HandleBoxDropped(StringHash eventType, VariantMap& eventData)
{
    int playerId = eventData["Player"].GetInt();

    UpdatePlayerScore(playerId, 2);
}