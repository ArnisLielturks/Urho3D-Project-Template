/**
 * Entry function for the mod
 */
void Start()
{
    log.Info("Minimap.as loaded");
    SubscribeToEvent("LevelLoaded", "HandleLevelLoaded");
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