# Set project name
project (ProjectTemplate)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

# Define target name
set (TARGET_NAME ProjectTemplate)

# Find Urho3D library
find_package (Urho3D REQUIRED)
include_directories (${URHO3D_INCLUDE_DIRS})

define_source_files (GLOB_CPP_PATTERNS *.c* GLOB_H_PATTERNS *.h* RECURSE GROUP)

# Setup target with resource copying
setup_main_executable ()

ADD_DEFINITIONS (-std=c++11)
