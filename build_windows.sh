#!/usr/bin/env bash

cd /var/sample/
bash ./script/cmake_mingw.sh build-windows -DURHO3D_HOME=/Urho3D/build-windows -DURHO3D_PROFILING=0 -DMINGW_PREFIX=/usr/bin/x86_64-w64-mingw32 -DDIRECTX_LIB_SEARCH_PATHS=/usr/bin/x86-w64-mingw32/lib -DCMAKE_BUILD_TYPE=Release -DURHO3D_DEPLOYMENT_TARGET=generic $@ || true
cd build-windows
make -j $(nproc)