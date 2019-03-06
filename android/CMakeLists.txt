# Set project name
project (EmptyProject)
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
# include (UrhoCommon)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

# Find Urho3D library
# find_package (Urho3D REQUIRED)
# include_directories (${URHO3D_INCLUDE_DIRS})

# Define target name
set (TARGET_NAME EmptyProject)

define_source_files (GLOB_CPP_PATTERNS *.c* GLOB_H_PATTERNS *.h* RECURSE GROUP)

#set(INCLUDE_DIRS include rapidjson/include)
set(INCLUDE_DIRS include)

# Setup target with resource copying
setup_main_executable ()


ADD_DEFINITIONS(
    -std=c++11 # Or -std=c++0x
    # Other flags
)
