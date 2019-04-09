#!/bin/bash
#rm -rf Urho3D
git clone https://github.com/ArnisLielturks/Urho3D.git --depth 1
mkdir Urho3D/Source/ProjectTemplate

rm -rf Urho3D/bin/Data
rm -rf Urho3D/bin/CoreData
cp -r bin/Data Urho3D/bin/Data
cp -r bin/CoreData Urho3D/bin/CoreData

cp -rf Source/* Urho3D/Source/ProjectTemplate/
cp android/CMakeLists.txt Urho3D/Source/ProjectTemplate/

cp android/launcher-app/src/main/java/com/github/urho3d/launcher/LauncherActivity.kt Urho3D/android/launcher-app/src/main/java/com/github/urho3d/launcher/LauncherActivity.kt
cp android/launcher-app/src/main/AndroidManifest.xml Urho3D/android/launcher-app/src/main/AndroidManifest.xml
cp android/launcher-app/src/main/java/com/github/urho3d/launcher/MainActivity.kt Urho3D/android/launcher-app/src/main/java/com/github/urho3d/launcher/MainActivity.kt
cp android/launcher-app/src/main/res/values/strings.xml Urho3D/android/launcher-app/src/main/res/values/strings.xml
cp android/launcher-app/CMakeLists.txt Urho3D/android/launcher-app/CMakeLists.txt

#echo "add_subdirectory (\${CMAKE_CURRENT_SOURCE_DIR}/../../Source/ProjectTemplate ProjectTemplate)" >> Urho3D/android/launcher-app/CMakeLists.txt
cd Urho3D
./script/dockerized.sh android ./gradlew --stacktrace -P URHO3D_LIB_TYPE=SHARED -P URHO3D_TOOLS=0 -P ANDROID_ABI=armeabi-v7a -P URHO3D_SAMPLES=0 -P CMAKE_BUILD_TYPE=Debug assembleDebug
#
#curl -X POST \
#        -H "Content-Type: multipart/form-data" \
#        -F file=@android/launcher-app/build/outputs/apk/debug/launcher-app-armeabi-v7a-debug.apk \
#        -F build=${CIRCLE_BUILD_NUM} \
#        -F platform=android \
#        -F description="Android-automated-builds" \
#        -F token=$SITE_TOKEN \
#        $SITE_URL || true

cd ..
curl -vvv -X POST \
        -H "Content-Type: multipart/form-data" \
        -F file=@Urho3D/android/launcher-app/build/outputs/apk/debug/launcher-app-armeabi-v7a-debug.apk \
        -F build=17 \
        -F platform=android \
        -F description="Android-automated-builds-local" \
        -F token="FrameskippersToken" \
        https://mods.frameskippers.com/system/game/upload