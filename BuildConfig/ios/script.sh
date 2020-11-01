#!/bin/bash

# Clone the latest engine version
git clone https://github.com/Urho3D/Urho3D.git --depth=1 Urho3D-ios

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
cp -rf BuildConfig/CMakeLists.txt Urho3D/Source/ProjectTemplate/CMakeLists.txt

# Use custom CMake file to build Urho3D and this project
cp -rf BuildConfig/Urho3DCMakeLists.txt Urho3D/Source/CMakeLists.txt

cd Urho3D

# Download Google Admob sdk
wget https://dl.google.com/googleadmobadssdk/googlemobileadssdkios.zip

./script/cmake_ios.sh build
cd build
sudo gem install cocoapods
cp ../../BuildConfig/ios/Podfile ./Podfile
pod update

# TODO: implement command line builds for ios app
cd ..
