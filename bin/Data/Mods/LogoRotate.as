Node@ logo = null;
Timer timer;

void Start()
{
    SubscribeToEvent("LevelChangingFinished", "HandleLevelLoaded");
}

void Stop()
{

}

/**
 * Find the Urho3D logo in the scene
 */
void FindLogo()
{
    if (scene !is null) {
        logo = scene.GetChild("Logo", true);
        if (logo !is null) {
            log.Info("Logo found!");
        } else {
            log.Warning("Logo not yet found");
        }
    } else {
        log.Warning("Scene is null");
    }
}

/**
 * Apply some animation to the logo
 */
void HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    float timestep = eventData["TimeStep"].GetFloat();
    if (logo !is null) {
        logo.Roll(timestep * 20);
    } else {
        if (timer.GetMSec(false) > 1000) {
            FindLogo();
            timer.Reset();
        }
    }
}

/**
 * Show notification with the level that was loaded
 */
void HandleLevelLoaded(StringHash eventType, VariantMap& eventData)
{
    String previousLevelName = eventData["From"].GetString();
    String levelName = eventData["To"].GetString();

    if (levelName == "Level") {
        SubscribeToEvent("Update", "HandleUpdate");
    } else {
        UnsubscribeFromEvent("Update");
    }
}