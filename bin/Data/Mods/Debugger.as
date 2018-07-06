/**
 * Array of all the loaded mod (script) names
 */
Array<String>@  mods = {};

/**
 * Entry function for the mod
 */
void Start()
{
    SubscribeToEvent("SetLevel", "HandleLevelChange");
    SubscribeToEvent("LevelLoaded", "HandleLevelLoaded");
    SubscribeToEvent("ModsLoaded", "HandleModsLoaded");
}

/**
 * Output debug message when level changing is requested
 */
void HandleLevelChange(StringHash eventType, VariantMap& eventData)
{
    String levelName = eventData["Name"].GetString();
    log.Info("[Debugger.as] Level loaded: " + levelName);

    VariantMap data;
    data["Name"] = "MyTestVariable";
    SendEvent("AddConfig", data);

    SetGlobalVar("MyTestVariable", 123.21);
}

/**
 * Display all the loaded mods(scripts) when the level is finished loading
 */
void HandleLevelLoaded(StringHash eventType, VariantMap& eventData)
{
    DrawModNames();
}

/**
 * When all the mods are loaded, retrieve the list
 */
void HandleModsLoaded(StringHash eventType, VariantMap& eventData)
{
    mods = eventData["Mods"].GetStringVector();
    log.Info("[Debugger.as] Total mods loaded: " + mods.length);
}

/**
 * Create a list of loaded mods(scripts)
 */
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

/**
 * Create a single text element
 */
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