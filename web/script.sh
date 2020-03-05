#!/bin/bash
# For debugging remove the Urho3D folder every time we run the script
#rm -rf Urho3D

# Clone the latest engine version
git clone https://github.com/Urho3D/Urho3D.git --depth=1 Urho3D-web

# Remove original Urho3D asset directories
rm -rf Urho3D-web/bin/Data
rm -rf Urho3D-web/bin/CoreData

rm -rf Urho3D-web/Source/Samples/0*
rm -rf Urho3D-web/Source/Samples/1*
rm -rf Urho3D-web/Source/Samples/2*
rm -rf Urho3D-web/Source/Samples/3*
rm -rf Urho3D-web/Source/Samples/4*
rm -rf Urho3D-web/Source/Samples/5*

# Copy our own asset directories
cp -r bin/Data Urho3D-web/bin/Data
cp -r bin/CoreData Urho3D-web/bin/CoreData

# Create our project subdirectory
mkdir Urho3D-web/Source/ProjectTemplate

# Copy our sample to the Urho3D subdirectory to build it the same ways as the samples are built
cp -rf Source/* Urho3D-web/Source/ProjectTemplate/
cp -rf web/application.html Urho3D-web/Source/ProjectTemplate/application.html
cp -rf web/CMakeLists.txt Urho3D-web/Source/ProjectTemplate/CMakeLists.txt
cp -rf script/dockerized.sh Urho3D-web/script/dockerized.sh
cp -rf web/Urho3DCMakeLists.txt Urho3D-web/Source/CMakeLists.txt

cd Urho3D-web
# Release mode
./script/dockerized.sh web

cd ..
