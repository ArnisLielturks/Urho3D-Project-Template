/**
 * Entry function for the mod
 * It adds few physical boxes in the main menu scene
 */
void Start()
{
    // By this event the main menu is already been created with a scene
    SubscribeToEvent("LevelChangingFinished", "HandleLevelLoaded");
}

void Stop()
{
}

/**
 * Display all the loaded mods(scripts) when the level is finished loading
 */
void HandleLevelLoaded(StringHash eventType, VariantMap& eventData)
{
    String levelName = eventData["To"].GetString();
    if (levelName == "MainMenu") {
        const uint NUM_OBJECTS = 10;
        for (uint i = 0; i < NUM_OBJECTS; ++i) {
            Node@ box = scene.CreateChild("Mushroom");
            box.AddTag("MenuObject");
            box.position = Vector3(Random(0.5f) - 0.25f, 5 + i, Random(0.5f) - 0.25f);//Vector3(Random(5.0f) - 2.5f, 5.0f, Random(5.0f) - 2.5f);
            box.rotation = Quaternion(0.0f, Random(360.0f), 0.0f);
            //box.SetScale(0.5f + Random(2.0f));
            StaticModel@ boxObject = box.CreateComponent("StaticModel");
            boxObject.model = cache.GetResource("Model", "Models/Box.mdl");
            boxObject.material = cache.GetResource("Material", "Materials/Box.xml");

            RigidBody@ body = box.CreateComponent("RigidBody");
            body.mass = 1.0f;
            body.friction = 0.75f;
            CollisionShape@ shape = box.CreateComponent("CollisionShape");
            shape.SetBox(Vector3::ONE);
        }
    }
}
