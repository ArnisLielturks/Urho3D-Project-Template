Node@ logo = null;
Node@ characterNode = null;
RigidBody@ body = null;
AnimationController@ animCtrl = null;
float scale = 0.008f;
Vector3 moveSpeed;
float animationSpeed = 1.0f;
Array<String> animationList;
Array<Vector3> animationMovementSpeed;
int activeAnimation = 0;
Timer timer;
Timer followCameraTimer;

void Start()
{
    animationList.Push("Mods/Character/Bot_SlowWalking.ani");
    animationMovementSpeed.Push(Vector3(0, 0, 15.0f));

    animationList.Push("Mods/Character/Bot_Run.ani");
    animationMovementSpeed.Push(Vector3(0, 0, 320.0f));

    animationList.Push("Mods/Character/Bot_RunLeft.ani");
    animationMovementSpeed.Push(Vector3(-320.0f, 0, 0.0f));

    animationList.Push("Mods/Character/Bot_RunBack.ani");
    animationMovementSpeed.Push(Vector3(0, 0, -320.0f));

    animationList.Push("Mods/Character/Bot_RunRight.ani");
    animationMovementSpeed.Push(Vector3(320.0f, 0, 0.0f));

    animationList.Push("Mods/Character/Bot_Walk.ani");
    animationMovementSpeed.Push(Vector3(0.0f, 0, 160.0f));

    animationList.Push("Mods/Character/Bot_WalkLeft.ani");
    animationMovementSpeed.Push(Vector3(-160.0f, 0, 0));

    animationList.Push("Mods/Character/Bot_WalkBack.ani");
    animationMovementSpeed.Push(Vector3(0.0f, 0, -120.0f));

    animationList.Push("Mods/Character/Bot_WalkRight.ani");
    animationMovementSpeed.Push(Vector3(160.0f, 0, 0));

    // animationList.Push("Mods/Character/Bot_Fall.ani");
    // animationMovementSpeed.Push(Vector3(0, 0, 0.0f));

    CreateCharacter();

    SubscribeToEvent("LevelChangingFinished", "HandleLevelLoaded");
    SubscribeToEvent("LevelChangingInProgress", "HandleLevelDestroyed");

    VariantMap data;
    data["ConsoleCommandName"]        = "character_speed";
    data["ConsoleCommandEvent"]       = "CharacterSpeed";
    data["ConsoleCommandDescription"] = "Change character speed";
    SendEvent("ConsoleCommandAdd", data);
    SubscribeToEvent("CharacterSpeed", "HandleSpeedChanged");

    data["ConsoleCommandName"]        = "character_animation";
    data["ConsoleCommandEvent"]       = "CharacterAnimation";
    data["ConsoleCommandDescription"] = "Change character animation";
    SendEvent("ConsoleCommandAdd", data);
    SubscribeToEvent("CharacterAnimation", "HandleCharacterAnimation");
}

void Stop()
{
    // RemoveCharacter();
}

void RemoveCharacter()
{
    if (scene is null) {
        return;
    }
    Array<Node@> nodes = scene.GetChildrenWithTag("Character",true);
    for(uint i = 0; i<nodes.length; i++)
    {
        nodes[i].Remove();
    }
    characterNode = null;
    animCtrl = null;
}

void HandleLevelDestroyed(StringHash eventType, VariantMap& eventData)
{
    if (scene is null || network.serverRunning == false) {
        log.Info("Cannot remove character");
        return;
    }
    RemoveCharacter();
}

void HandleSpeedChanged(StringHash eventType, VariantMap& eventData)
{
    if (animCtrl is null) {
        return;
    }
    log.Info("speed change");
    Array<String>@ parameters = eventData["Parameters"].GetStringVector();
    log.Info("Params " + String(parameters.length));
    if (parameters.length == 2) {
        animCtrl.SetSpeed(animationList[activeAnimation], parameters[1].ToFloat());
    }
}

void HandleCharacterAnimation(StringHash eventType, VariantMap& eventData)
{
    log.Info("animation change");
    Array<String>@ parameters = eventData["Parameters"].GetStringVector();
    log.Info("Params " + String(parameters.length));
    if (parameters.length == 2) {
        ChangeAnimation(parameters[1].ToInt());
    }
}

void ChangeAnimation(int index)
{
    if (animCtrl is null) {
        return;
    }
    if (index >= animationMovementSpeed.length) {
        index = 0;
    }
    activeAnimation = index;
    moveSpeed = animationMovementSpeed[activeAnimation] * scale;

    animCtrl.Play(animationList[activeAnimation], 0, true, 0.2f);
    // animCtrl.PlayExclusive(animationList[2], 0, true, 0.1f);
    // animCtrl.SetWeight(animationList[1], 0.0f);
    // if (activeAnimation > 0) {
    //     animCtrl.SetWeight(animationList[activeAnimation], 0.5f);
    //     animCtrl.SetWeight(animationList[activeAnimation-1], 0.5f);
    //     animCtrl.Play(animationList[activeAnimation-1], 0, true, 0.2f);
    // } else {
        animCtrl.SetWeight(animationList[activeAnimation], 1.0f);
    // }
    animCtrl.SetSpeed(animationList[activeAnimation], animationSpeed);
    characterNode.MarkNetworkUpdate();
    AnimatedModel@ modelObject = characterNode.GetComponent("AnimatedModel");
    modelObject.model = cache.GetResource("Model", "Mods/Character/Bot.mdl");
}

void CreateCharacter()
{
    Connection@ connection = network.serverConnection;
    if (scene is null || connection !is null) {
        log.Info("Cannot create character");
        return;
    }
    log.Info("Creating character");
    RemoveCharacter();

    characterNode = scene.CreateChild("Character", REPLICATED);
    characterNode.AddTag("Character");
    characterNode.position = Vector3(0, 1, 0);
    characterNode.scale = Vector3(scale, scale, scale);

    AnimatedModel@ modelObject = characterNode.CreateComponent("AnimatedModel");
    modelObject.model = cache.GetResource("Model", "Mods/Character/Bot.mdl");
    modelObject.ApplyMaterialList();
    modelObject.castShadows = true;
    modelObject.updateInvisible = true;

    log.Info("Character node " + String(characterNode.id));
    log.Info("Character node model " + String(modelObject.id));
    animCtrl = characterNode.CreateComponent("AnimationController");
    log.Info("Character node anim " + String(animCtrl.id));

    body = characterNode.CreateComponent("RigidBody");
    body.mass = 40.0f;
    body.angularFactor = Vector3::ZERO;
    CollisionShape@ shape = characterNode.CreateComponent("CollisionShape");
    shape.SetCapsule(60.0f, 180.0f, Vector3(0.0f, 90.0f, -10.0f));

    ChangeAnimation(0);

    // Tell the player's camera to follow this character for some amount of time
    VariantMap data;
    data["ID"] = 0;
    data["Node"] = characterNode;
    data["Distance"] = 3.0f;
    SendEvent("SetPlayerCameraTarget", data);

    followCameraTimer.Reset();

    SubscribeToEvent("Update", "HandleUpdate");
}

void FindCharacter()
{
    if (scene !is null) {
        characterNode = scene.GetChild("Character", true);
        if (characterNode !is null)
        {
            log.Info("Character found!");
        } else {
            log.Warning("Character not yet found");
        }
    }
}

/**
 * Apply some animation to the logo
 */
void HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    // if (characterNode is null)
    // {
    //     FindCharacter();
    // }
    if (scene is null || !scene.updateEnabled || characterNode is null || animCtrl is null || animCtrl.animations[0] is null) {
        return;
    }
    float timestep = eventData["TimeStep"].GetFloat();
    characterNode.Translate(Vector3(-timestep * animCtrl.animations[0].speed, -timestep * animCtrl.animations[0].speed, -timestep * animCtrl.animations[0].speed) * moveSpeed);
    characterNode.Yaw(timestep * 10.0f);

    if (characterNode.worldPosition.y < -30) {
        characterNode.position = Vector3(0, 1, 0);
    }
    // body.ApplyImpulse(Vector3(0.0, 0.0, -120.00f * timestep));
    body.Activate();
    // Vector3 position = modelNode.position;
    // position.z -= timestep;
    // modelNode.position = position;

    if (timer.GetMSec(false) > 3000) {
        ChangeAnimation(++activeAnimation);
        timer.Reset();
    }

    if (followCameraTimer.GetMSec(false) > 200000) {
        // Reset player camera to follow player controlled node instead
        VariantMap data;
        data["ID"] = 0;
        SendEvent("SetPlayerCameraTarget", data);
    }
}


/**
 * Show notification with the level that was loaded
 */
void HandleLevelLoaded(StringHash eventType, VariantMap& eventData)
{
    String previousLevelName = eventData["From"].GetString();
    String levelName = eventData["To"].GetString();

    if (levelName == "MainMenu" || levelName == "Level") {
        if (network.serverRunning) {
            CreateCharacter();
        }
    } else {
        UnsubscribeFromEvent("Update");
    }
}