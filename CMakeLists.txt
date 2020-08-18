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

# Setup RPATH settings
if (URHO3D_LIB_TYPE STREQUAL SHARED AND NOT WIN32 AND NOT ANDROID AND NOT IOS AND NOT TVOS AND NOT WEB)
    # Add RPATH entries when building
    set (CMAKE_SKIP_BUILD_RPATH FALSE)
    # And set the RPATH entries so that the executable works both in the build tree and install destination
    set (CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    if (APPLE)
        set (CMAKE_MACOSX_RPATH TRUE)
        set (ORIGIN @loader_path)
    else ()
        set (ORIGIN $ORIGIN)
    endif ()
    # Library location relative to the executable in the build tree
    set (CMAKE_INSTALL_RPATH ${ORIGIN}/../lib)
    # Library location relative to the executable in the tool directory in the build tree
    list (APPEND CMAKE_INSTALL_RPATH ${ORIGIN}/../../lib)                               # The tools are installed one directory further down from normal executable
    # Library location relative to the executable in the install destination
    list (APPEND CMAKE_INSTALL_RPATH ${ORIGIN}/../lib${LIB_SUFFIX}/${PATH_SUFFIX})      # The library location is based on SDK install destination
    # Library location relative to the executable in the tool directory in the install destination
    list (APPEND CMAKE_INSTALL_RPATH ${ORIGIN}/../../lib${LIB_SUFFIX}/${PATH_SUFFIX})
    # The last entry to the install destination of the library, if the destination location is not in the system default search path, e.g. /usr/local/lib
    list (FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX} isSystemDir)
    if (isSystemDir STREQUAL -1)
        list (APPEND CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${DEST_LIBRARY_DIR})
    endif ()
endif ()

# HTTPS support
#set(URHO3D_SSL 1)

if (URHO3D_ANGELSCRIPT)
 add_definitions(-DURHO3D_ANGELSCRIPT=1)
endif()

if (URHO3D_LUA)
 add_definitions(-DURHO3D_LUA=1)
endif()

if (NOT ANDROID AND NOT IOS)
    # Dirty hack to get the WS to work since it relies on 3rd party static libs
    add_subdirectory (HumbleNet)
    set (LIBS ${LIBS} humblenet)
endif ()

# Define target name
set (TARGET_NAME ProjectTemplate)

define_source_files (GLOB_CPP_PATTERNS Source/*.c* Source/*.m* GLOB_H_PATTERNS Source/*.h* RECURSE GROUP)

# Setup target with resource copying
setup_main_executable ()

if (MSVC)
    target_link_libraries(ProjectTemplate Setupapi.lib)
endif()

# Enable voxel sample to be built
set (VOXEL_SUPPORT true)
if (VOXEL_SUPPORT)
    add_definitions(-DVOXEL_SUPPORT=1)
endif ()

# Enable nakama networking library
set (NAKAMA_SUPPORT false)
if (NAKAMA_SUPPORT)
    add_definitions(-DNAKAMA_SUPPORT=1)
    include_directories(nakama-cpp-sdk/include)
    set(NAKAMA_SHARED_LIBRARY TRUE)
    add_subdirectory(${CMAKE_SOURCE_DIR}/nakama-cpp-sdk)
    target_link_libraries(${TARGET_NAME} ext_nakama-cpp)
    CopyNakamaSharedLib(${TARGET_NAME})
endif ()
