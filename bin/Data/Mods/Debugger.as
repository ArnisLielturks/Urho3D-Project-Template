void Start()
{
    SubscribeToEvent("Set levels", "HandleLevelChange");
}

void HandleLevelChange(StringHash eventType, VariantMap& eventData)
{
    log.Info(">>>>>>> Level " + eventData["Set levels"].GetString());
}