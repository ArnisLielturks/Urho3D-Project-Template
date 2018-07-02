Array<String>@  mods = {};

void Start()
{
    SubscribeToEvent("Set levels", "HandleLevelChange");
    SubscribeToEvent("LevelLoaded", "HandleLevelLoaded");
    SubscribeToEvent("ModsLoaded", "HandleModsLoaded");
}

void HandleLevelChange(StringHash eventType, VariantMap& eventData)
{
    String levelName = eventData["Name"].GetString();
    log.Info(">>>>>>> Level " + levelName);
}

void HandleLevelLoaded(StringHash eventType, VariantMap& eventData)
{
    DrawModNames();
}

void HandleModsLoaded(StringHash eventType, VariantMap& eventData)
{
    mods = eventData["Mods"].GetStringVector();
    log.Info("############### Total mods loaded: " + mods.length);
    DrawModNames();
}

void DrawModNames()
{
    if (mods.empty) {
        return;
    }
    CreateScriptName("Mods Loaded:", 0);
    for (uint i = 0; i < mods.length; i++) {
        CreateScriptName(mods[i], i + 1);
    }
}

void CreateScriptName(String name, int index)
{
    int fontSize = 10;
    int margin = 2;
     // Construct new Text object
    Text@ helloText = Text();

    // Set String to display
    helloText.text = name;

    // Set font and text color
    helloText.SetFont(cache.GetResource("Font", "Fonts/Anonymous Pro.ttf"), fontSize);
    helloText.color = Color(0.0f, 0.8f, 0.0f);

    // Align Text center-screen
    helloText.horizontalAlignment = HA_LEFT;
    helloText.verticalAlignment = VA_TOP;

    helloText.position = IntVector2(10, 10 + index * (fontSize + margin));

    // Add Text instance to the UI root element
    ui.root.AddChild(helloText);
}