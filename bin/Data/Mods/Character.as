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
Timer timer2;

void Start()
{
    animationList.Push("Mods/Character/Bot_SlowWalking.ani");
    animationMovementSpeed.Push(Vector3(0, 0, 15.0f));

    animationList.Push("Mods/Character/Bot_Run.ani");
    animationMovementSpeed.Push(Vector3(0, 0, 320.0f));

    animationList.Push("Mods/Character/Bot_RunLeft.ani");
    animationMovementSpeed.Push(Vector3(-320.0f, 0, 0.0f));

    animationList.Push("Mods/Character/Bot_RunRight.ani");
    animationMovementSpeed.Push(Vector3(320.0f, 0, 0.0f));

    animationList.Push("Mods/Character/Bot_Fall.ani");
    animationMovementSpeed.Push(Vector3(0, 0, 0.0f));

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
    RemoveCharacter();
}

void HandleSpeedChanged(StringHash eventType, VariantMap& eventData)
{
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
    if (index >= animationMovementSpeed.length) {
        index = 0;
    }
    activeAnimation = index;
    moveSpeed = animationMovementSpeed[activeAnimation] * scale;

    animCtrl.PlayExclusive(animationList[activeAnimation], 0, true, 0.2f);
    // animCtrl.PlayExclusive(animationList[2], 0, true, 0.1f);
    // animCtrl.SetWeight(animationList[1], 0.0f);
    // animCtrl.SetWeight(animationList[2], 1.0f);
    animCtrl.SetSpeed(animationList[activeAnimation], animationSpeed);
}

void CreateCharacter()
{
    if (scene is null || network.serverRunning == false) {
        return;
    }

    RemoveCharacter();
    // Node@ modelNode = scene.CreateChild();
    // modelNode.AddTag("Character");
    // modelNode.position = Vector3(0, 0.0, 0);
    // modelNode.scale = Vector3(scale, scale, scale);

    // AnimatedModel@ modelObject = modelNode.CreateComponent("AnimatedModel");
    // modelObject.model = cache.GetResource("Model", "Mods/Character/Bot.mdl");
    // modelObject.ApplyMaterialList();
    // // modelObject.material = cache.GetResource("Material", "Materials/Wood.xml");
    // modelObject.castShadows = true;
    // modelObject.updateInvisible = true;


    characterNode = scene.CreateChild();
    characterNode.AddTag("Character");
    characterNode.position = Vector3(0, 1, 0);
    characterNode.scale = Vector3(scale, scale, scale);

    // StaticModel@ boxObject = characterNode.CreateComponent("StaticModel");
    // boxObject.model = cache.GetResource("Model", "Models/Box.mdl");
    // boxObject.material = cache.GetResource("Material", "Materials/Stone.xml");

    AnimatedModel@ modelObject = characterNode.CreateComponent("AnimatedModel");
    modelObject.model = cache.GetResource("Model", "Mods/Character/Bot.mdl");
    modelObject.ApplyMaterialList();
    modelObject.castShadows = true;
    modelObject.updateInvisible = true;


    // Create an AnimationState for a walk animation. Its time position will need to be manually updated to advance the
    // animation, The alternative would be to use an AnimationController component which updates the animation automatically,
    // but we need to update the model's position manually in any case
    animCtrl = characterNode.CreateComponent("AnimationController");

    body = characterNode.CreateComponent("RigidBody");
    body.mass = 40.0f;
    body.angularFactor = Vector3::ZERO;
    CollisionShape@ shape = characterNode.CreateComponent("CollisionShape");
    shape.SetCapsule(60.0f, 200.0f, Vector3(0.0f, 85.0f, 0.0f));

    // AnimationState@ state = modelObject.AddAnimationState(walkAnimation);
    // // Enable full blending weight and looping
    // state.weight = 1.0f;
    // state.looped = true;
    // state.time = Random(walkAnimation.length);

    // Create our Mover script object that will move & animate the model during each frame's update. Here we use a shortcut
    // script-only API function, CreateScriptObject, which creates a ScriptInstance component into the scene node, then uses
    // it to instantiate the object (using the script file & class name provided)
    // Mover@ mover = cast<Mover>(modelNode.CreateScriptObject(scriptFile, "Mover"));
    // mover.SetParameters(MODEL_MOVE_SPEED, MODEL_ROTATE_SPEED, bounds);
    SubscribeToEvent("Update", "HandleUpdate");

    ChangeAnimation(0);
}

/**
 * Apply some animation to the logo
 */
void HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    if (scene is null || !scene.updateEnabled || characterNode is null || animCtrl.animations[0] is null) {
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

    if (timer.GetMSec(false) > 5000) {
        ChangeAnimation(++activeAnimation);
        timer.Reset();
    }

    if (timer2.GetMSec(false) > 20000) {
        timer2.Reset();
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