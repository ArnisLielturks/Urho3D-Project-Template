#!/bin/bash
# For debugging remove the Urho3D folder every time we run the script
rm -rf Urho3D

# Clone the latest engine version
git clone https://github.com/Urho3D/Urho3D.git --depth=1

# Remove original Urho3D asset directories
rm -rf Urho3D/bin/Data
rm -rf Urho3D/bin/CoreData

# Copy our own asset directories
cp -r bin/Data Urho3D/bin/Data
cp -r bin/CoreData Urho3D/bin/CoreData

# Create our project subdirectory
mkdir Urho3D/Source/ProjectTemplate

# Copy our sample to the Urho3D subdirectory to build it the same ways as the samples are built
cp -rf Source/* Urho3D/Source/ProjectTemplate/

# Add our custom CMakeLists to allow to build our subproject
cp android/CMakeLists.txt Urho3D/Source/ProjectTemplate/

# Remove original Urho3D android application files
#rm -rf Urho3D/android/urho3d-lib
rm -rf Urho3D/android/launcher-app

# Copy our versions of the apps
#cp -r android/urho3d-lib Urho3D/android/urho3d-lib
cp -r android/launcher-app Urho3D/android/launcher-app

cp -r android/SDL/android-project/* Urho3D/ThirdParty/SDL/android-project/

cd Urho3D

# Run dockerized android build to genereate libProjectTemplate.so file
./script/dockerized.sh android ./gradlew -P URHO3D_LIB_TYPE=SHARED -P URHO3D_TOOLS=0 -P ANDROID_ABI=armeabi-v7a -P URHO3D_SAMPLES=0 -P CMAKE_BUILD_TYPE=Release assembleDebug

cd ..
