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

    for (uint i = 0; i < 1; i++) {
        // Invalid loading steps which will test our ACK flow
        data["Name"] = "Loading step without ACK #" + String(i);
        data["Event"] = "LoadUnknown #" + String(i);
        SendEvent("RegisterLoadingStep", data);
    }
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

    DelayedExecute(1.0, false, "void LoadImagesProgress()");
    DelayedExecute(2.0, false, "void LoadImagesProgress()");
    DelayedExecute(3.0, false, "void LoadImagesProgress()");
    DelayedExecute(4.0, false, "void FinishLoadImages()");
}

float progress = 0.0;

void LoadImagesProgress()
{
    progress += 0.25;
    VariantMap data;
    data["Event"] = "LoadImages";
    data["Progress"] = progress;
    SendEvent("LoadingStepProgress", data);
}

void FinishLoadImages()
{
    VariantMap data;
    data["Event"] = "LoadImages";
    SendEvent("LoadingStepFinished", data);
}
