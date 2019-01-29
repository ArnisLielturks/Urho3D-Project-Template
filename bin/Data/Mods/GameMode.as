void Start()
{
	SubscribeToEvent("SetLevel", "HandleLevelChange");

    VariantMap data;
    data["Event"] = "BoxDestroyed";
    data["Message"] = "Destroy 1 box";
    data["Image"] = "Textures/Achievements/trophy-cup.png";
    data["Threshold"] = 1;
    SendEvent("AddAchievement", data);

    data["Message"] = "Destroy 2 boxes";
    data["Threshold"] = 2;
    SendEvent("AddAchievement", data);

    data["Message"] = "Destroy 3 boxes";
    data["Threshold"] = 3;
    SendEvent("AddAchievement", data);

    data["Message"] = "Destroy 4 boxes";
    data["Threshold"] = 4;
    SendEvent("AddAchievement", data);
}

void Stop()
{

}

/**
 * Output debug message when level changing is requested
 */
void HandleLevelChange(StringHash eventType, VariantMap& eventData)
{
    String levelName = eventData["Name"].GetString();

    if (levelName == "Level") {
    	for (uint i = 0; i < 30; i++) {
    		XMLFile@ xml = cache.GetResource("XMLFile", "Mods/GameMode/DestroyCube.xml");
	        scene.InstantiateXML(xml.root, Vector3(Random(40.0f) - 20.0f, 2.0f, Random(40.0f) - 20.0f), Quaternion());
	    }
    }
}