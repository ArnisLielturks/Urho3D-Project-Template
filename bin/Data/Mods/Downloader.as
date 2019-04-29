HttpRequest@ httpRequest;

void Start()
{
    SubscribeToEvent("DownloadImages", "HandleDownloadImages");

    VariantMap data;
    data["Name"] = "Downloading avatar";
    data["Event"] = "DownloadImages";
    SendEvent("RegisterLoadingStep", data);
}

void Stop()
{
}

void HandleDownloadImages(StringHash eventType, VariantMap& eventData)
{
    VariantMap data;
    data["Event"] = "DownloadImages";

    // Sent event to let the system know that we will handle this loading step
    SendEvent("AckLoadingStep", data);

    SubscribeToEvent("Update", "HandleUpdate");
}

void HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    // Create HTTP request
    if (httpRequest is null) {
        SetRandomSeed(time.systemTime);
        uint randomNumber = Random(1000);
        String url = "http://www.gravatar.com/avatar/" + String(randomNumber) + "?s=100&d=retro&r=g";
        log.Info("Downloading avatar from: " + url);
        httpRequest = network.MakeHttpRequest(url);
    }
    else
    {
        // Initializing HTTP request
        if (httpRequest.state == HTTP_INITIALIZING) {
            return;
        }
        // An error has occurred
        else if (httpRequest.state == HTTP_ERROR)
        {
            UnsubscribeFromEvent("Update");

            // Let the loading system know that we finished our work
            VariantMap data;
            data["Event"] = "DownloadImages";
            SendEvent("LoadingStepFinished", data);

            httpRequest = null;
        }
        else
        {
            if (httpRequest.availableSize > 0) {
                File@ file = File("Data/Textures/Avatar.png", FILE_READWRITE);
                file.Write(httpRequest.Read(httpRequest.availableSize));
            }
            else
            {
                log.Info("Saving remote image to Data/Textures/Avatar.png");
                UnsubscribeFromEvent("Update");

                // Let the loading system know that we finished our work
                VariantMap data;
                data["Event"] = "DownloadImages";
                SendEvent("LoadingStepFinished", data);

                httpRequest = null;
            }
        }
    }
}
