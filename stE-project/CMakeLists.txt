cmake_minimum_required (VERSION 3.8)

project(stE-project)

add_executable (stE-project "main.cpp" "Car.cpp" "Car.h" "simple-opengl-loader.h" "Common.cpp" "Common.h" "Console.cpp" "Console.h" "Graphics.cpp" "Graphics.h"
							"LevelGenerator.cpp" "LevelGenerator.h" "Weather.cpp" "Weather.h")
set(BUILD_SHARED_LIBS FALSE) # static library, because i dont want to copy .dll from compiled library to .exe (the path to compiled lib may change)
add_subdirectory(SFML_dir SFML)

include_directories(./src)
include_directories(./src/headers)

target_link_libraries(stE-project sfml-graphics sfml-system sfml-main)
target_compile_features(stE-project PUBLIC cxx_std_20)
