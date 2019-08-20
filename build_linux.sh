#!/usr/bin/env bash
cd /var/sample/
./script/cmake_generic.sh build -DURHO3D_HOME=/Urho3D/build -DCMAKE_BUILD_TYPE=Release -DURHO3D_HASH_DEBUG=0 -DURHO3D_PROFILING=0 -DURHO3D_DEPLOYMENT_TARGET=generic || true
cd build
make -j $(nproc)

mkdir archive
cp build/bin/EmptyProject archive/EmptyProject || true
cp build/bin/EmptyProject_d archive/EmptyProject_d || true
cp -r bin/Data archive/Data
cp -r bin/CoreData archive/CoreData
cp -r bin/EmptyProject.desktop archive/EmptyProject.desktop
rm -rf archive/Data/Saves/Achievements.json
chmod -R 777 archive
chmod a+x archive/EmptyProject.desktop
cd archive
zip -r "build.zip" *  > /dev/null