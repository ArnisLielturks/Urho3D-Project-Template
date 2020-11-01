#!/bin/bash
# For debugging remove the Urho3D folder every time we run the script
#rm -rf Urho3D

CUR_DIR=$(pwd)
# Clone the latest engine version
git clone https://github.com/Urho3D/Urho3D.git Urho3D-android --depth=1
#git clone https://gitlab.com/ArnisLielturks/urho3d-websockets.git --depth=1 Urho3D -b ws-only

# Remove original Urho3D asset directories
# rm -rf Urho3D-android/bin/Data
# rm -rf Urho3D-android/bin/CoreData

# Copy our own asset directories
# cp -r bin/Data Urho3D-android/bin/Data
# cp -r bin/CoreData Urho3D-android/bin/CoreData

# rm -rf Urho3D-android/Source/ProjectTemplate
# Create our project subdirectory
# mkdir Urho3D-android/Source/ProjectTemplate

# Copy our sample to the Urho3D subdirectory to build it the same ways as the samples are built
# cp -rf Source/* Urho3D-android/Source/ProjectTemplate/

# Add our custom CMakeLists to allow to build our subproject
# cp BuildConfig/android/CMakeLists.txt Urho3D-android/Source/ProjectTemplate/
#if grep -q "add_subdirectory (ProjectTemplate)" "Urho3D/Source/CMakeLists.txt"; then
#	echo "Already added to CMakeLists file"
#else
#	echo "add_subdirectory (ProjectTemplate)" >> Urho3D/Source/CMakeLists.txt
#fi

# Remove original Urho3D android application files
# rm -rf Urho3D-android/android/launcher-app

# Copy our versions of the apps
# cp -r BuildConfig/android/launcher-app Urho3D-android/android/

# cp -r BuildConfig/android/SDL/android-project/* Urho3D-android/Source/ThirdParty/SDL/android-project/

cd Urho3D-android


# Run dockerized android build to genereate libProjectTemplate.so file
# Release mode
# ./script/dockerized.sh android ./gradlew build --stacktrace -P URHO3D_LUA=0 -P URHO3D_LIB_TYPE=SHARED -P URHO3D_TOOLS=0 -P ANDROID_ABI=armeabi-v7a -P URHO3D_SAMPLES=0 -P CMAKE_BUILD_TYPE=Release
export LIB_TYPE=shared
export BUILD_TYPE=dbg
export DBE_TAG=latest
#rel #dbg
export ARCH=armeabi-v7a
#all
export PLATFORM=android

#Build Urho
script/dockerized.sh ${PLATFORM/-*} rake build

# Install scaffolding app requirements in /home/urho3d/stage/usr/local
script/dockerized.sh ${PLATFORM/-*} rake install[~/stage]

# Create new project in the current directory
script/dockerized.sh ${PLATFORM/-*} rake new[ProjectTemplate, $(pwd)]

# Add our assets directories
rm -rf $(pwd)/ProjectTemplate/bin
cp -r ../bin $(pwd)/ProjectTemplate/bin

# Copy our java files
cp -r ../BuildConfig/android/launcher-app/src $(pwd)/ProjectTemplate/src

# Build our app
cd $(pwd)/ProjectTemplate URHO3D_HOME=/home/urho3d/stage/usr/local script/dockerized.sh ${PLATFORM/-*}

# Debug mode
#./script/dockerized.sh android ./gradlew --stacktrace -P URHO3D_LUA=0 -P URHO3D_LIB_TYPE=SHARED -P URHO3D_TOOLS=0 -P ANDROID_ABI=armeabi-v7a -P URHO3D_SAMPLES=0 -P CMAKE_BUILD_TYPE=Release assembleDebug

# Finally do a check if the APK is there
# cp $(find ./android/launcher-app/build/outputs/apk/debug/ -name "*.apk") ./../ProjectTemplate.apk
# FILE=./../ProjectTemplate.apk
# if test -f "$FILE"; then
#     echo "$FILE exists"
# else
#     echo "$FILE not found"
#     # Mark our build as failed
#     exit 1
# fi

cd $CUR_DIR
