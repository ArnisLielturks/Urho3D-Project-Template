Array<Sprite@> sprites;

void Start()
{
    SubscribeToEvent("LevelChangingInProgress", "HandleLevelLoaded");
}

void Stop()
{

}

void CreateSprite(String image, float velocity, Vector2 position)
{
    // Get the Urho3D fish texture
    Texture2D@ decalTex = cache.GetResource("Texture2D", image);

    // Create a new sprite, set it to use the texture
    Sprite@ sprite = Sprite();
    sprite.texture = decalTex;

    // The UI root element is as big as the rendering window, set random position within it
    sprite.position = position;

    // Set sprite size & hotspot in its center
    sprite.size = IntVector2(128, 128);
    sprite.hotSpot = IntVector2(64, 64);

    // Add as a child of the root UI element
    ui.root.AddChild(sprite);

    // Store sprite's velocity as a custom variable
    sprite.vars["Velocity"] = velocity;

    sprite.priority = -sprites.length;

    // Store sprites to our own container for easy movement update iteration
    sprites.Push(sprite);

    sprite.bringToBack = true;
}
void CreateUI()
{
    // Get rendering window size as floats
    float width = graphics.width;
    float height = graphics.height;

    // get files in directory
    Array<String> textures = fileSystem.ScanDir(fileSystem.programDir + "/Data/Textures/Achievements", "", SCAN_FILES, false);

    // add new files
    for (uint i = 0; i < textures.length; i++)
    {
        float randomX = Random() * width;
        float randomY = height / 2 + (Random() * 200.0 - 100.0);
        Vector2 pos(randomX, randomY);
        float velocity = Random() * 25.0 + 1.0;
        if (randomX > width / 2) {
            velocity *= -1;
        }
        CreateSprite(fileSystem.programDir + "/Data/Textures/Achievements/" + textures[i], velocity, pos);
    }
}

void HandleLevelLoaded(StringHash eventType, VariantMap& eventData)
{
    String previousLevelName = eventData["From"].GetString();
    String levelName = eventData["To"].GetString();

    if (levelName == "Loading") {
        CreateUI();
        SubscribeToEvent("Update", "HandleUpdate");
    } else {
        UnsubscribeFromEvent("Update");
    }
}

void HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    // Take the frame time step, which is stored as a float
    float timeStep = eventData["TimeStep"].GetFloat();

    for (uint i = 0; i < sprites.length; i++) {
        Sprite@ sprite = sprites[i];
        Vector2 pos = sprite.position;
        pos.x += timeStep * sprite.vars["Velocity"].GetFloat();
        sprite.position = pos;

        // Rotate
        float newRot = sprite.rotation + timeStep * sprite.vars["Velocity"].GetFloat();
        sprite.rotation = newRot;
    }
}