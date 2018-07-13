/**
 * Entry function for the mod
 */
void Start()
{
    log.Info("LevelLoader.as START");
    SubscribeToEvent("LevelLoaded", "HandleLevelLoaded");
}

void Stop()
{
	log.Info("LevelLoader.as STOP");
}

/**
 * Show notification with the level that was loaded
 */
void HandleLevelLoaded(StringHash eventType, VariantMap& eventData)
{
    String levelName = eventData["Name"].GetString();

    VariantMap data;
    data["Message"] = "Level '" + levelName + "' loaded!";
    SendEvent("ShowNotification", data);
}