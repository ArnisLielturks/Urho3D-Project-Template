Text@ packetsIn;
Text@ packetsOut;
Timer timer;

void Start()
{
    SubscribeToEvent("Update", "HandleUpdate");
    SubscribeToEvent("LevelChangingInProgress", "HandleLevelLoaded");
}

void Stop()
{
    ui.root.GetChild("PacketsIn").Remove();
    ui.root.GetChild("PacketsOut").Remove();
}

void CreateElements()
{
    packetsIn = CreateTextElement("PacketsIn",   "Packets In :", 0);
    packetsOut = CreateTextElement("PacketsOut", "Packets Out:", 1);
    packetsIn.visible = false;
    packetsOut.visible = false;
}

void HandleLevelLoaded(StringHash eventType, VariantMap& eventData)
{
    CreateElements();
}

void HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    if (timer.GetMSec(false) > 1000) {
        timer.Reset();
        if (packetsIn is null || packetsIn is null) {
            return;
        }
        Connection@ serverConnection = network.serverConnection;
        if (serverConnection !is null) {
            packetsIn.visible = true;
            packetsOut.visible = true;
            packetsIn.text = "Packets In : " + String(serverConnection.packetsInPerSec);
            packetsOut.text = "Packets Out: " + String(serverConnection.packetsOutPerSec);
        } else if (network.serverRunning) {
            int pIn = 0;
            int pOut = 0;
            for (uint i = 0; i < network.clientConnections.length; i++) {
                pIn += network.clientConnections[i].packetsInPerSec;
                pOut += network.clientConnections[i].packetsOutPerSec;
            }
            packetsIn.visible = true;
            packetsOut.visible = true;
            packetsIn.text = "Packets In : " + String(pIn);
            packetsOut.text = "Packets Out: " + String(pOut);
        } else {
            packetsIn.visible = false;
            packetsOut.visible = false;
        }
    }
}

Text@ CreateTextElement(String id, String name, int index)
{
    int fontSize = 12;
    int margin = 2;
    // Construct new Text object
    Text@ element = Text(id);

    // Set String to display
    element.text = name;

    // Set font and text color
    element.SetFont(cache.GetResource("Font", "Fonts/Muli-Regular.ttf"), fontSize);
    element.color = Color(0.7f, 0.7f, 0.0f);
    element.textEffect = TE_STROKE;

    // Align Text center-screen
    element.horizontalAlignment = HA_LEFT;
    element.verticalAlignment = VA_CENTER;

    element.position = IntVector2(10, 10 + index * (fontSize + margin));

    // Add Text instance to the UI root element
    ui.root.AddChild(element);

    return element;
}
