String message;
Text@ text;
HttpRequest@ httpRequest;

void Start()
{
    // Create the user interface
    CreateUI();

    SubscribeToEvent("LevelChangingFinished", "HandleLevelChangingFinished");
}

void Stop()
{

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
    text = Text();

    // Set font and text color
    text.SetFont(cache.GetResource("Font", "Fonts/ABeeZee-Regular.ttf"), 15);
    text.color = Color(1.0f, 1.0f, 0.0f);

    // Align Text center-screen
    text.horizontalAlignment = HA_CENTER;
    text.verticalAlignment = VA_CENTER;

    // Add Text instance to the UI root element
    ui.root.AddChild(text);
}

void HandleUpdate(StringHash eventType, VariantMap& eventData)
{

    // // Create HTTP request
    // if (httpRequest is null)
    //     httpRequest = network.MakeHttpRequest("https://quotes.rest/qod");
    // else
    // {
    //     // Initializing HTTP request
    //     if (httpRequest.state == HTTP_INITIALIZING)
    //         return;
    //     // An error has occurred
    //     else if (httpRequest.state == HTTP_ERROR)
    //     {
    //         text.text = httpRequest.get_error();
    //         UnsubscribeFromEvent("Update");
    //     }
    //     // Get message data
    //     else
    //     {
    //         if (httpRequest.availableSize > 0)
    //             message += httpRequest.ReadLine();
    //         else
    //         {
    //             text.text = "Processing...";
    //
    //             JSONFile@ json = JSONFile();
    //             json.FromString(message);
    //
    //             JSONValue quotes = json.GetRoot().Get("contents");
    //
    //             if (quotes["quotes"].isArray) {
    //                 text.text = quotes["quotes"][0].Get("quote").GetString();
    //             }
    //
    //             log.Error("AAAA " + text.text);
    //
    //             UnsubscribeFromEvent("Update");
    //         }
    //     }
    // }
}
