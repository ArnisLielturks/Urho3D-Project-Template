#!/bin/bash
# For debugging remove the Urho3D folder every time we run the script
#rm -rf Urho3D

# Clone the latest engine version
git clone https://github.com/Urho3D/Urho3D.git --depth=1
#git clone https://gitlab.com/ArnisLielturks/urho3d-websockets.git --depth=1 Urho3D -b ws-only

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
cp BuildConfig/android/CMakeLists.txt Urho3D/Source/ProjectTemplate/
echo "add_subdirectory (ProjectTemplate)" > Urho3D/Source/CMakeLists.txt

# Remove original Urho3D android application files
rm -rf Urho3D/android/launcher-app

# Copy our versions of the apps
cp -r BuildConfig/android/launcher-app Urho3D/android/

cp -r BuildConfig/android/SDL/android-project/* Urho3D/Source/ThirdParty/SDL/android-project/
cp -rf script/dockerized.sh Urho3D/script/dockerized.sh

cd Urho3D


# Run dockerized android build to genereate libProjectTemplate.so file
# Release mode
./script/dockerized.sh android ./gradlew build --stacktrace -P URHO3D_LUA=0 -P URHO3D_LIB_TYPE=SHARED -P URHO3D_TOOLS=0 -P ANDROID_ABI=armeabi-v7a -P URHO3D_SAMPLES=0 -P CMAKE_BUILD_TYPE=Release

# Debug mode
#./script/dockerized.sh android ./gradlew --stacktrace -P URHO3D_LUA=0 -P URHO3D_LIB_TYPE=SHARED -P URHO3D_TOOLS=0 -P ANDROID_ABI=armeabi-v7a -P URHO3D_SAMPLES=0 -P CMAKE_BUILD_TYPE=Release assembleDebug

# Finally do a check if the APK is there
cp $(find ./android/launcher-app/build/outputs/apk/debug/ -name "*.apk") ./../ProjectTemplate.apk
FILE=./../ProjectTemplate.apk
if test -f "$FILE"; then
    echo "$FILE exists"
else
    echo "$FILE not found"
    # Mark our build as failed
    exit 1
fi

cd ..
