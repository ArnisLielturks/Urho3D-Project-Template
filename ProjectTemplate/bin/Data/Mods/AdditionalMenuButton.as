/**
 * Entry function for the mod
 */
void Start()
{
    VariantMap buttonInfo;
    buttonInfo["Name"] = "New Game (mod)";
    buttonInfo["EventToCall"] = "ButtonEvent";
    VariantMap buttons = GetGlobalVar("MenuButtons").GetVariantMap();
    buttons["NewButton"] = buttonInfo;
    SetGlobalVar("MenuButtons", buttons);
    SubscribeToEvent("ButtonEvent", "HandleButtonEvent");
}

void HandleButtonEvent(StringHash eventType, VariantMap& eventData)
{
    // For test purposes we will call the same event that is called by the "New game" button
    VariantMap data;
    data["Name"] = "NewGameSettingsWindow";
    SendEvent("OpenWindow", data);
}

void Stop()
{

}


