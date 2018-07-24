/**
 * Entry function for the mod
 */
void Start()
{
    log.Info("LevelLoader.as START");
    SubscribeToEvent("LevelChangingFinished", "HandleLevelLoaded");
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
    String previousLevelName = eventData["From"].GetString();
    String levelName = eventData["To"].GetString();

    VariantMap data;
    data["Message"] = "Level '" + levelName + "' loaded!";
    SendEvent("ShowNotification", data);
}