# Urho3D-Empty-Project
A basic app skeleton which uses global notifications and level management with fade-in/fade-out effects. Project uses code samples from the community.

## How to build
Make sure that the URHO3D_HOME environment variable is set and points to the right directory. Build this the same way how you would build Urho3D engine itself

### Ubuntu:
```
git clone https://github.com/ArnisLielturks/Urho3D-Empty-Project.git
cd Urho3D-Empty-Project
./cmake_generic.sh build
cd  build
make
```


If everything worked, build/bin directory should contain `EmptyProject` executable.


And of course here's the quick preview of how it should look in the end:
![alt tag](https://github.com/ArnisLielturks/Urho3D-Empty-Project/blob/master/Screenshots/preview.gif)


# How to extend the functionality?

## Messages
There are 3 types of different messages that can appear in this sample:
1. Achievements
2. Notifications
3. Pop up messages

Each of them uses different events to show them.

### Achievements (bottom left corner)
```
VariantMap data;
data["Message"] = "Entered menu!";
SendEvent("NewAchievement", data);
```

### Notifications (bottom right corner)
```
VariantMap data;
data["Message"] = "New notification!";
SendEvent("ShowNotification", data);
```

### Pop-up messages
```
VariantMap data;
data["Title"] = "Everything OK!";
data["Message"] = "Seems like everything is ok!";
SendEvent("ShowAlertMessage", data);
```

## Levels
Switching between different scenes are created by using LevelManager. It uses fade-in/fade-out animation to go from one level to the other. Level in this case can be anything - Splash screen, Main menu, Loading screen, Game level, Game exit screen.

### How to create new level?
To create new level you could look at the `Source/Levels/Splash.h` and `Source/Levels/Splash.cpp` files. New level class should extend BaseLevel class (`Source/BaseLevel.h`). BaseLevel class defines many methods that will be utilized by the LevelManager to allow unified interfaces for each of the levels. 
When your level is ready, make sure to register it in the LevelManager (`Source/LevelManager.h`) class in the `RegisterAllFactories` method. 
```
void RegisterAllFactories()
{
  context_->RegisterFactory<Levels::Splash>();
}
```
When that's done, you can switch between levels by using the following code.
```
VariantMap data;
eventData["Name"] = "Splash";
SendEvent(MyEvents::E_SET_LEVEL, eventData);
```
You can also pass additional parameters for the new level
```
VariantMap data;
eventData["Name"] = "Splash";
eventData["Param1"] = 123;
SendEvent(MyEvents::E_SET_LEVEL, eventData);
```
So when your level is initialized, LevelManager will pass all the arguments and they will become available in the `Init()` method. From there you could retrieve them like this:
```
void Splash::Init()
{
  int value = data_["Param1].GetInt();
}
```

## Global game configuration
When game is launched a config file located in `Data/Config/Game.json` is loaded. These values are globally used in the system which can specifiy all sorts of settings that users may want to change and use between game sessions. For instance it can contain all the user configured game settings - shadow quality, texture quality, resolution, sound settings etc.

Config file content looks like this:
```
{
    "UnlimitedAmmo": false,
    "Countdown": 3,
    "Gametime": 55,
    "Bots": 3,
    "MenuMusic": true,
    "DefaultServerIP": "127.0.0.1",
    "Port": 55123
}
```
`Urho3DPlayer::LoadConfig()` is the method that loads this file after starting game. To retrieve any values in the code you have to call one of these methods:
```
GetGlobalVar("MenuMusic").GetBool();
GetGlobalVar("Countdown").GetInt();
GetGlobalVar("DefaultServerIP").GetString();
```

Config file saving is still in my TODO list!

That's it!

If you have any ideas how this can be improved, let me know!
