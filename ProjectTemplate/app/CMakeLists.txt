set(TARGET_NAME ProjectTemplate)
define_source_files(GLOB_CPP_PATTERNS src/main/cpp/*.cpp GLOB_H_PATTERNS src/main/cpp/*.h RECURSE GROUP)
setup_main_executable()
setup_test()
