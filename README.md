![Builds](https://github.com/ArnisLielturks/Urho3D-Project-Template/workflows/Builds/badge.svg)

# Urho3D-Project-Template
App template with the following features:
* Continuous integration with artifact publishing to itch.io (Android, OSX, Windows, Linux, WEB)
* Level management
* UI Window management
* LUA/AS mods with hot-reload
* Sound management
* Controller handling with input mapping
* Splitscreen support
* Achievement logic
* Console command management
* INI configuration file loading/saving
* Settings window to configure all aspect of the engine

## How to build
Make sure that the URHO3D_HOME environment variable is set and points to the right directory. Build this the same way how you would build Urho3D engine itself

### Ubuntu:
```
git clone https://github.com/ArnisLielturks/Urho3D-Empty-Project.git
cd Urho3D-Empty-Project
./script/cmake_generic.sh build
cd  build
make
```


If everything worked, build/bin directory should contain `ProjectTemplate` executable.


### Few screenshots
![MainMenu](https://github.com/ArnisLielturks/Urho3D-Empty-Project/blob/master/Screenshots/MainMenu.png)
![Settings](https://github.com/ArnisLielturks/Urho3D-Empty-Project/blob/master/Screenshots/Settings.png)
![Achievements](https://github.com/ArnisLielturks/Urho3D-Empty-Project/blob/master/Screenshots/Achievements.png)
![Credits](https://github.com/ArnisLielturks/Urho3D-Empty-Project/blob/master/Screenshots/Credits.png)
![Loading](https://github.com/ArnisLielturks/Urho3D-Empty-Project/blob/master/Screenshots/Loading.png)
![Level](https://github.com/ArnisLielturks/Urho3D-Empty-Project/blob/master/Screenshots/Level.png)


# How to extend the functionality?
Read the wiki: https://github.com/ArnisLielturks/Urho3D-Empty-Project/wiki
