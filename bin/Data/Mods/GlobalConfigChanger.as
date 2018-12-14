Window@ window = null;
ListView@ list = null;

/**
 * List of all the configuration keys to check
 */
Array<String> configList;

/**
 * Entry function for the mod
 */
void Start()
{
    configList.Push("Debugger_Developer");
    configList.Push("LogLevel");
    configList.Push("LogName");
    configList.Push("LogQuiet");
    configList.Push("ResourcePaths");
    configList.Push("WindowWidth");
    configList.Push("WindowHeight");
    configList.Push("FullScreen");
    configList.Push("Borderless");
    configList.Push("Monitor");
    configList.Push("LowQualityShadows");
    configList.Push("MaterialQuality");
    configList.Push("MultiSample");
    configList.Push("Shadows");
    configList.Push("TextureAnisotropy");
    configList.Push("TextureQuality");
    configList.Push("TripleBuffer");
    configList.Push("TextureFilterMode");
    configList.Push("VSync");
    configList.Push("Sound");
    configList.Push("SoundBuffer");
    configList.Push("SoundInterpolation");
    configList.Push("SoundMixRate");
    configList.Push("SoundStereo");
    configList.Push("SoundMasterVolume");
    configList.Push("SoundEffectsVolume");
    configList.Push("SoundAmbientVolume");
    configList.Push("SoundVoiceVolume");
    configList.Push("SoundMusicVolume");

    log.Info("GlobalConfigChanger.as START");
    SubscribeToEvent("KeyDown", "HandleKeyDown");
}

void Stop()
{
	log.Info("GlobalConfigChanger.ass STOP");
}

/**
 * Show notification with the level that was loaded
 */
void HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    int key = eventData["Key"].GetInt();
    if (key == KEY_F3) {
        if (window !is null) {
            DestroyMenu();
        } else {
            CreateMenu();
        }
    }
}

void CreateMenu()
{
    log.Info("GlobalConfigChanger.as creating menu");
     // Create the Window and add it to the UI's root node
    window = Window();
    ui.root.AddChild(window);

    // Set Window size and layout settings
    window.minWidth = 500;
    window.SetLayout(LM_VERTICAL, 20, IntRect(10, 10, 10, 10));
    window.SetAlignment(HA_CENTER, VA_CENTER);
    window.name = "Window";

    // Create Window 'titlebar' container
    UIElement@ titleBar = UIElement();
    //titleBar.SetMinSize(0, 24);
    titleBar.verticalAlignment = VA_TOP;
    //titleBar.layoutMode = LM_HORIZONTAL;

    // Create the Window title Text
    Text@ windowTitle = Text();
    windowTitle.name = "WindowTitle";
    windowTitle.text = "Global configuration";
    windowTitle.textEffect = TE_SHADOW;
    windowTitle.SetAlignment(HA_CENTER, VA_CENTER);

    // Add the controls to the title bar
    titleBar.AddChild(windowTitle);

    // Add the title bar to the Window
    window.AddChild(titleBar);

    // Apply styles
    window.SetStyleAuto();
    windowTitle.SetStyleAuto();

    // Subscribe also to all UI mouse clicks just to see where we have clicked
    // SubscribeToEvent("UIMouseClick", "HandleControlClicked");

    list = window.CreateChild("ListView");
    list.selectOnClickEnd = true;
    list.highlightMode = HM_ALWAYS;
    list.minHeight = 400;
    list.SetStyleAuto();

    for (uint i = 0; i < configList.length; i++) {
        String key = configList[i];
        CreateItem(key, GetGlobalVar(key));
    }
}

void CreateItem(String label, Variant value)
{
    UIElement@ singleLine = UIElement();
    singleLine.SetStyleAuto();
    singleLine.SetLayout(LM_HORIZONTAL, 6, IntRect(6, 6, 6, 6));

    // Create the Window title Text
    Text@ labelText = singleLine.CreateChild("Text");
    labelText.name = label;
    labelText.text = label;
    labelText.textEffect = TE_SHADOW;
    labelText.SetStyleAuto();

    // Create the Window title Text
    Text@ valueText = singleLine.CreateChild("Text");
    valueText.name = label;
    valueText.text = "N/A";
    valueText.textEffect = TE_SHADOW;
    if (value.type == VAR_STRING) {
        valueText.text = value.GetString();
    }
    if (value.type == VAR_BOOL) {
        valueText.text = (value.GetBool()) ? "true" : "false";
    }
    if (value.type == VAR_INT) {
        valueText.text = String(value.GetInt());
    }
    if (value.type == VAR_FLOAT) {
        valueText.text = String(value.GetFloat());
    }
    valueText.SetStyleAuto();

    list.AddItem(singleLine);
}

void DestroyMenu()
{
    log.Info("GlobalConfigChanger.as destroying menu");
    window.Remove();
    window = null;
}