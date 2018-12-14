#include "Helpers/Helper.as"

int index = 0;
/**
 * Entry function for the mod
 */
void Start()
{
    log.Info("Achievements.as START");
    // Function from Helpers/Helper.as file
    Test();

    log.Info("Achievements.as loaded");
    SubscribeToEvent("LevelChangingFinished", "HandleLevelLoaded");

    VariantMap data;

    data["Message"] = "Discovered this amazing Urho3D sample";
    data["Image"] = "Textures/lunar-module.png";
    SendEvent("NewAchievement", data);
    //
    // data["Title"] = "title";
    // data["Message"] = "Message";
    // SendEvent("ShowAlertMessage", data);

    // DelayedExecute(1, true, "DisplayAchievement");
    // DisplayAchievement();
}

void DisplayAchievement()
{
    VariantMap data;
    data["Message"] = "Initialized achievements.as script from the mods directory";
    SendEvent("NewAchievement", data);
    index++;
}

void Stop()
{
    log.Info("Achievements.as STOP");
}

/**
 * When specific level is loaded, show achievement
 */
void HandleLevelLoaded(StringHash eventType, VariantMap& eventData)
{
    String previousLevelName = eventData["From"].GetString();
    String levelName = eventData["To"].GetString();
    if (levelName == "Level") {
        VariantMap data;
        data["Message"] = "Achievement called from Mods/Achievements.as!";
        SendEvent("NewAchievement", data);
    }
}