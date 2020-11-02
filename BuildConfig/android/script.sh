#!/bin/bash
set -e

# Clode the Urho repo
git clone https://github.com/Urho3D/Urho3D.git Urho3D-android --depth=1 || true

cd Urho3D-android

# Copy our configration file which has increased ram
cp ../BuildConfig/android/gradle.properties ./

export LIB_TYPE=shared
export URHO3D_LIB_TYPE=SHARED
export BUILD_TYPE=dbg
export DBE_TAG=latest
# export ARCH=all
export ARCH=armeabi-v7a
# export ARCH=armeabi-v7a,arm64-v8a
export PLATFORM=android

function step {
    echo ""
    echo "---------- $@ ----------"
    echo ""
}

# Build the engine
BUILD_URHO=1
# Generate ProjectTemplate apk
USE_OUR_PROJECT=1

if [[ "1" == "$BUILD_URHO" ]];
then
    # Copy our own android build settings to disable launcher-app from building
    cp ../BuildConfig/android/settings.gradle.kts ./

    # Copy SDL android activity which supports 2 way communication between Java <-> C++
    cp -r ../BuildConfig/android/SDL/android-project/* Source/ThirdParty/SDL/android-project/

    # Generate env from system env variables
    step Updating .env
    rake update_dot_files && script/dockerized.sh ${PLATFORM/-*} env

    # Build Urho lib
    step Building
    script/dockerized.sh ${PLATFORM/-*} rake build

    # Install build artifacts
    step Install build
    script/dockerized.sh ${PLATFORM/-*} rake install[~/stage]
fi

# Remove previous project dir if it exists
step Remove previous scaffolding folder
rm -rf $(pwd)/ProjectTemplate

# Create our app directory
step Create new scaffolding folder
script/dockerized.sh ${PLATFORM/-*} rake new[ProjectTemplate,$(pwd)]

if [[ "1" == "$USE_OUR_PROJECT" ]];
then
    # Add our assets directories
    rm -rf $(pwd)/ProjectTemplate/bin
    cp -r ../bin $(pwd)/ProjectTemplate/bin

    # Copy our custom Android manifest
    cp ../BuildConfig/android/launcher-app/src/main/AndroidManifest.xml $(pwd)/ProjectTemplate/app/src/main/AndroidManifest.xml

    # Copy our own Android app assets - icons, translations etc
    rm -rf $(pwd)/ProjectTemplate/app/src/main/res
    cp -r ../BuildConfig/android/launcher-app/src/main/res $(pwd)/ProjectTemplate/app/src/main/res

    # Our ouwn android activity
    rm -rf $(pwd)/ProjectTemplate/app/src/main/java
    cp -r ../BuildConfig/android/launcher-app/src/main/java $(pwd)/ProjectTemplate/app/src/main/java

    # Copy our own tweaked gradle config with additional libraries and naming tweaks
    cp ../BuildConfig/android/launcher-app/build.gradle.kts $(pwd)/ProjectTemplate/app/build.gradle.kts


    # # Copy our c++ files
    rm -rf $(pwd)/ProjectTemplate/app/src/main/cpp/*
    cp -r ../Source/* $(pwd)/ProjectTemplate/app/src/main/cpp/
fi

# Build our android project
step Build scaffolding project
cd $(pwd)/ProjectTemplate
URHO3D_HOME=/home/urho3d/stage/usr/local script/dockerized.sh ${PLATFORM/-*}
