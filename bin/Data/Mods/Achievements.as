/**
 * Entry function for the mod
 */
void Start()
{
    log.Info("Achievements.as loaded");
    SubscribeToEvent("LevelLoaded", "HandleLevelLoaded");
}

/**
 * When specific level is loaded, show achievement
 */
void HandleLevelLoaded(StringHash eventType, VariantMap& eventData)
{
    String levelName = eventData["Name"].GetString();
    if (levelName == "Level") {
        VariantMap data;
        data["Message"] = "Achievement called from\nMods/Achievements.as!";
        SendEvent("NewAchievement", data);
    }
}