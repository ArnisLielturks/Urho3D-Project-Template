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
    text.SetFont(cache.GetResource("Font", "Fonts/Anonymous Pro.ttf"), 15);
    text.color = Color(1.0f, 1.0f, 0.0f);

    // Align Text center-screen
    text.horizontalAlignment = HA_CENTER;
    text.verticalAlignment = VA_CENTER;

    // Add Text instance to the UI root element
    ui.root.AddChild(text);
}

void HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    // Create HTTP request
    if (httpRequest is null)
        httpRequest = network.MakeHttpRequest("http://frameskippers.com/motd.php");
    else
    {
        // Initializing HTTP request
        if (httpRequest.state == HTTP_INITIALIZING)
            return;
        // An error has occurred
        else if (httpRequest.state == HTTP_ERROR)
        {
            text.text = httpRequest.get_error();
            UnsubscribeFromEvent("Update");
        }
        // Get message data
        else
        {
            if (httpRequest.availableSize > 0)
                message += httpRequest.ReadLine();
            else
            {
                text.text = "Processing...";

                JSONFile@ json = JSONFile();
                json.FromString(message);

                JSONValue quote = json.GetRoot().Get("quote");
                JSONValue author = json.GetRoot().Get("author");

                if (quote.isNull || author.isNull)
                    text.text = "";
                else
                    text.text =  "Message of the day:\n" + quote.GetString() + "\n/ " + author.GetString();

                UnsubscribeFromEvent("Update");
            }
        }
    }
}
