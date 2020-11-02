cmake_minimum_required(VERSION 3.10.2)
project(ProjectTemplate)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

# Find Urho3D library
find_package (Urho3D REQUIRED)
include_directories (${URHO3D_INCLUDE_DIRS})

include(UrhoCommon)

add_subdirectory(app)
