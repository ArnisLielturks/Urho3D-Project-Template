# Define target name
set (TARGET_NAME ProjectTemplate)

# Find Urho3D library
find_package (Urho3D REQUIRED)
include_directories (${URHO3D_INCLUDE_DIRS})

define_source_files (GLOB_CPP_PATTERNS ${CMAKE_SOURCE_DIR}/Source/ProjectTemplate/*.c* GLOB_H_PATTERNS  ${CMAKE_SOURCE_DIR}/Source/ProjectTemplate/*.h* RECURSE GROUP)

# Custom emscripten HTML template
if (WEB)
	define_resource_dirs (HTML_SHELL ${CMAKE_SOURCE_DIR}/Source/ProjectTemplate/application.html )
endif()

# Setup target with resource copying
setup_main_executable ()

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --bind")
