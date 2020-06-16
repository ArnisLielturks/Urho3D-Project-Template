void Start()
{
    SubscribeToEvent("LevelChangingFinished", "HandleLevelLoaded");
}

void Stop()
{
    UIElement@ crosshair = ui.root.GetChild("Crosshair", false);
    if (crosshair !is null) {
        crosshair.Remove();
    }
}

void CreateCrosshair()
{
    BorderImage@ sight = BorderImage();
    sight.name = "Crosshair";
    sight.texture = cache.GetResource("Texture2D", "Textures/Sight.png");
    sight.SetAlignment(HA_CENTER, VA_CENTER);
    sight.SetSize(32, 32);
    ui.root.AddChild(sight);
}

/**
 * Show notification with the level that was loaded
 */
void HandleLevelLoaded(StringHash eventType, VariantMap& eventData)
{
    String previousLevelName = eventData["From"].GetString();
    String levelName = eventData["To"].GetString();

    if (levelName == "Level") {
        CreateCrosshair();
    }
}