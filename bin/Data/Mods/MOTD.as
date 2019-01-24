String message;
Text@ text;
HttpRequest@ httpRequest;
Timer timer;

void Start()
{
    // Create the user interface
    CreateUI();

    SubscribeToEvent("LevelChangingFinished", "HandleLevelChangingFinished");
    SubscribeToEvent("Update", "HandleUpdate");
}

void Stop()
{
    UIElement@ status = ui.root.GetChild("ModAsStatus", false);
    if (status !is null) {
        status.Remove();
    }
}

void HandleLevelChangingFinished(StringHash eventType, VariantMap& eventData)
{
    String from = eventData["From"].GetString();
    String to = eventData["To"].GetString();
    if (to == "MainMenu") {
        SubscribeToEvent("Update", "HandleUpdate");
    }
}

void CreateUI()
{
    // Construct new Text object
    text = Text("ModAsStatus");

    // Set font and text color
    text.SetFont(cache.GetResource("Font", "Fonts/Muli-Regular.ttf"), 15);
    text.color = Color(1.0f, 1.0f, 0.0f);

    // Align Text center-screen
    text.horizontalAlignment = HA_CENTER;
    text.verticalAlignment = VA_CENTER;
    text.text = "";

    // Add Text instance to the UI root element
    ui.root.AddChild(text);
}

void HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    if (timer.GetMSec(false) < 2000) {
        return;
    }

    return;

    // Create HTTP request
    if (httpRequest is null) {
        httpRequest = network.MakeHttpRequest("http://localhost:8000/mod.as");
        text.text = "Downloading remote mod";
        message = "//Downloaded from http://localhost:8000/mod.as\n";
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
            text.text = httpRequest.get_error();
            UnsubscribeFromEvent("Update");
            text.text = "";
        }
        // Get message data
        else
        {
            if (httpRequest.availableSize > 0) {
                message += httpRequest.ReadLine() + "\n";
            }
            else
            {
                log.Info("Saving remote mod to Data/Mods/mod.as");
                File@ file = File("Data/Mods/RemoteMod.as", FILE_WRITE);
                file.WriteLine(message);
                text.text = "";
                UnsubscribeFromEvent("Update");
            }
        }
    }
}
