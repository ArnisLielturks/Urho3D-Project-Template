/**
 * Entry function for the mod
 */
void Start()
{
    SubscribeToEvent("LoadSkills", "HandleLoadSkills");
    SubscribeToEvent("LoadImages", "HandleLoadImages");

    VariantMap data;
    data["Name"] = "Loading skills";
    data["Event"] = "LoadSkills";
    SendEvent("RegisterLoadingStep", data);

    data["Name"] = "Loading images";
    data["Event"] = "LoadImages";
    SendEvent("RegisterLoadingStep", data);


    // Invalid loading steps which will test our ACK flow
    data["Name"] = "Loading unknown #1";
    data["Event"] = "LoadUnknown #1";
    SendEvent("RegisterLoadingStep", data);

    data["Name"] = "Loading unknown #2";
    data["Event"] = "LoadUnknown #2";
    SendEvent("RegisterLoadingStep", data);

    data["Name"] = "Loading unknown #3";
    data["Event"] = "LoadUnknown #3";
    SendEvent("RegisterLoadingStep", data);

    data["Name"] = "Loading unknown #4";
    data["Event"] = "LoadUnknown #4";
    SendEvent("RegisterLoadingStep", data);

    data["Name"] = "Loading unknown #5";
    data["Event"] = "LoadUnknown #5";
    SendEvent("RegisterLoadingStep", data);
}

void Stop()
{
    log.Info("LevelLoader.as STOP");
}

void HandleLoadSkills()
{
    VariantMap data;
    data["Event"] = "LoadSkills";

    // Sent event to let the system know that we will handle this loading step
    SendEvent("AckLoadingStep", data);

    // Imitate loading
    DelayedExecute(2.0, false, "void FinishLoadSkills()");
}

void FinishLoadSkills()
{
    // Let the loading system know that we finished our work
    VariantMap data;
    data["Event"] = "LoadSkills";
    SendEvent("LoadingStepFinished", data);
}

void HandleLoadImages()
{
    VariantMap data;
    data["Event"] = "LoadImages";

    // Sent event
    SendEvent("AckLoadingStep", data);

    DelayedExecute(2.0, false, "void FinishLoadImages()");
}

void FinishLoadImages()
{
    VariantMap data;
    data["Event"] = "LoadImages";
    SendEvent("LoadingStepFinished", data);
}
