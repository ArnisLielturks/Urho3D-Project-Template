[![CircleCI](https://circleci.com/gh/ArnisLielturks/Urho3D-Project-Template/tree/master.svg?style=svg)](https://circleci.com/gh/ArnisLielturks/Urho3D-Empty-Project/tree/master)

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
./cmake_generic.sh build
cd  build
make
```


If everything worked, build/bin directory should contain `EmptyProject` executable.


And of course here's the quick preview of how it should look in the end:
![alt tag](https://github.com/ArnisLielturks/Urho3D-Empty-Project/blob/master/Screenshots/preview.gif)


# How to extend the functionality?
Read the wiki: https://github.com/ArnisLielturks/Urho3D-Empty-Project/wiki
