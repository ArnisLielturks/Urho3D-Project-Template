[![CircleCI](https://circleci.com/gh/ArnisLielturks/Urho3D-Project-Template/tree/master.svg?style=svg)](https://circleci.com/gh/ArnisLielturks/Urho3D-Empty-Project/tree/master)
![Builds](https://github.com/ArnisLielturks/Urho3D-Project-Template/workflows/Builds/badge.svg)

# Urho3D-Project-Template
App template with the following features:
* Level management
* UI Window management
* LUA/AS mods with hot-reload
* Sound management
* Controll mapping and input management
* Splitscreen support
* Achievement logic
* and many other features

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
