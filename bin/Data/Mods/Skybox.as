Array<Sprite@> sprites;

void Start()
{
    SubscribeToEvent("CreateSkybox", "HandleCreateSkybox");
    if (GetGlobalVar("CurrentLevel").GetString() == "Level") {
        Create();
    }

    // Register our loading step
    VariantMap data;
    data["Name"] = "Creating skybox";
    data["Event"] = "CreateSkybox";
    SendEvent("RegisterLoadingStep", data);
}

void Stop()
{
    Destroy();
}

void Destroy()
{
    Node@ sky = scene.GetChild("Sky");
    if (sky !is null) {
        sky.Remove();
    }

    Node@ zone = scene.GetChild("Zone");
    if (zone !is null) {
        zone.Remove();
    }
}

void Create()
{
    // Create skybox. The Skybox component is used like StaticModel, but it will be always located at the camera, giving the
    // illusion of the box planes being far away. Use just the ordinary Box model and a suitable material, whose shader will
    // generate the necessary 3D texture coordinates for cube mapping
    Node@ skyNode = scene.CreateChild("Sky");
    Skybox@ skybox = skyNode.CreateComponent("Skybox");
    skybox.model = cache.GetResource("Model", "Models/Box.mdl");
    skybox.material = cache.GetResource("Material", "Mods/Skybox/Materials/Skybox.xml");

    // Create a Zone component for ambient lighting & fog control
    Node@ zoneNode = scene.CreateChild("Zone");
    Zone@ zone = zoneNode.CreateComponent("Zone");
    zone.boundingBox = BoundingBox(-100.0f, 100.0f);
    zone.ambientColor = Color(0.0f, 0.0f, 0.0f);
    zone.fogColor = Color(1.0f, 1.0f, 1.0f);
    zone.fogStart = 10.0f;
    zone.fogEnd = 20.0f;
}

void HandleCreateSkybox(StringHash eventType, VariantMap& eventData)
{
    VariantMap data;
    data["Event"] = "CreateSkybox";
    SendEvent("AckLoadingStep", data);

    Create();

    SendEvent("LoadingStepFinished", data);
}
