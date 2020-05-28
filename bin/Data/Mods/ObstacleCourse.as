String loadingEventName = "LoadObstacleCourse";
void Start()
{
    SubscribeToEvent(loadingEventName, "HandleLoadObstacleCourse");

    // Register our loading step
    VariantMap data;
    data["Name"] = "Initializing obstacle course";
    data["Event"] = loadingEventName;
    SendEvent("RegisterLoadingStep", data);

    RegisterMap();
}

void Stop()
{

}

void RegisterMap()
{
    VariantMap data;
    data["Map"] = "Mods/ObstacleCourse/ObstacleCourse.xml";
    data["Name"] = "Obstacle course";
    data["Description"] = "Finish the obstacle course without falling!";
    data["Image"] = "Textures/Achievements/falling.png";

    Array<String> commands;
    commands.Push("ambient_light 0.6 0.6 0.5");
    data["Commands"] = commands;
    SendEvent("AddMap", data);
}

void HandleLoadGameMode(StringHash eventType, VariantMap& eventData)
{
    String map = eventData["Map"].GetString();
    if (map != "Mods/ObstacleCourse/ObstacleCourse.xml") {
        log.Info("Skipping ObstacleCourse initialization");
        return;
    }
    VariantMap data;
    data["Event"] = loadingEventName;

    // Sent event to let the system know that we will handle this loading step
    SendEvent("AckLoadingStep", data);

    // Let the loading system know that we finished our work
    SendEvent("LoadingStepFinished", data);
}