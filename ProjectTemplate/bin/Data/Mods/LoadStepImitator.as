/**
 * Entry function for the mod
 */
void Start()
{
    return;
    SubscribeToEvent("LoadImages", "HandleLoadImages");

    VariantMap data;

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

float progress = 0.0;
void HandleLoadImages()
{
    VariantMap data;
    data["Event"] = "LoadImages";

    // Sent event
    SendEvent("AckLoadingStep", data);

    progress = 0.0;
    DelayedExecute(1.0, false, "void LoadImagesProgress()");
    DelayedExecute(2.0, false, "void FinishLoadImages()");
}

void LoadImagesProgress()
{
    progress += 0.5;
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
