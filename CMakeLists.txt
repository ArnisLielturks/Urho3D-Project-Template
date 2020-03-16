# Set project name
project (ProjectTemplate)

# Set minimum version
cmake_minimum_required (VERSION 2.8.6)
if (COMMAND cmake_policy)
    cmake_policy (SET CMP0003 NEW)
    if (CMAKE_VERSION VERSION_GREATER 2.8.12 OR CMAKE_VERSION VERSION_EQUAL 2.8.12)
        # INTERFACE_LINK_LIBRARIES defines the link interface
        cmake_policy (SET CMP0022 NEW)
    endif ()
    if (CMAKE_VERSION VERSION_GREATER 3.0.0 OR CMAKE_VERSION VERSION_EQUAL 3.0.0)
        # Disallow use of the LOCATION target property - therefore we set to OLD as we still need it
        cmake_policy (SET CMP0026 OLD)
        # MACOSX_RPATH is enabled by default
        cmake_policy (SET CMP0042 NEW)
    endif ()
endif ()

# Set CMake modules search path
set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake/Modules)

# Include Urho3D Cmake common module
include (UrhoCommon)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

# Find Urho3D library
find_package (Urho3D REQUIRED)
include_directories (${URHO3D_INCLUDE_DIRS})

# HTTPS support
set(URHO3D_SSL 1)

if (URHO3D_ANGELSCRIPT)
 add_definitions(-DURHO3D_ANGELSCRIPT=1)
endif()

if (URHO3D_LUA)
 add_definitions(-DURHO3D_LUA=1)
endif()

# Define target name
set (TARGET_NAME ProjectTemplate)

define_source_files (GLOB_CPP_PATTERNS Source/*.c* GLOB_H_PATTERNS Source/*.h* RECURSE GROUP)

# Setup target with resource copying
setup_main_executable ()

if (MSVC)
    target_link_libraries(ProjectTemplate Setupapi.lib)
endif()

ADD_DEFINITIONS(
    -std=c++11
)


