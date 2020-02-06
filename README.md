[![CircleCI](https://circleci.com/gh/ArnisLielturks/Urho3D-Project-Template/tree/master.svg?style=svg)](https://circleci.com/gh/ArnisLielturks/Urho3D-Empty-Project/tree/master)
[![Build Status](https://travis-ci.org/ArnisLielturks/Urho3D-Project-Template.svg?branch=master)](https://travis-ci.org/ArnisLielturks/Urho3D-Project-Template)

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


And of course here's the quick preview of how it should look in the end:
![alt tag](https://github.com/ArnisLielturks/Urho3D-Empty-Project/blob/master/Screenshots/preview.gif)


# How to extend the functionality?
Read the wiki: https://github.com/ArnisLielturks/Urho3D-Empty-Project/wiki

# Prebuilt binaries
Download almost latest builds from CI (list is updated manually):

| OS | Link |
| --- | --- |
| Windows             | [Download](https://719-138001494-gh.circle-artifacts.com/0/ProjectTemplate_Windows.zip) |
| Windows (no AS/LUA) | [Download](https://715-138001494-gh.circle-artifacts.com/0/ProjectTemplate_Windows_no_scripts.zip) |
| Linux               | [Download](https://718-138001494-gh.circle-artifacts.com/0/ProjectTemplate_Linux.zip) |
| Linux (no AS/LUA)   | [Download](https://717-138001494-gh.circle-artifacts.com/0/ProjectTemplate_Linux_no_scripts.zip) |
| Android             | [Download](https://716-138001494-gh.circle-artifacts.com/0/home/circleci/project/Urho3D/android/launcher-app/build/outputs/apk/debug/launcher-app-armeabi-v7a-debug.apk) |

