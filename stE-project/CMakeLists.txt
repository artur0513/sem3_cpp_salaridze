cmake_minimum_required (VERSION 3.8)

project(stE-project)
include (CTest)

add_executable (stE-project "main.cpp" "Car.cpp" "Car.h" "simple-opengl-loader.h" "Common.cpp" "Common.h" "Console.cpp" "Console.h" "Graphics.cpp" "Graphics.h"
							"LevelGenerator.cpp" "LevelGenerator.h" "Weather.cpp" "Weather.h")



set(BUILD_SHARED_LIBS FALSE) # static library, because i dont want to copy .dll from compiled library to .exe (the path to compiled lib may change)
add_subdirectory(SFML_dir SFML)

include_directories(./src)
include_directories(./src/headers)

target_link_libraries(stE-project sfml-graphics sfml-system sfml-main)
target_compile_features(stE-project PUBLIC cxx_std_20)

if (BUILD_TESTING) #тест
add_executable (stE-testing "test.cpp" "Car.cpp" "Car.h" "simple-opengl-loader.h" "Common.cpp" "Common.h" "Console.cpp" "Console.h" "Graphics.cpp" "Graphics.h"
							"LevelGenerator.cpp" "LevelGenerator.h" "Weather.cpp" "Weather.h")
target_link_libraries(stE-testing sfml-graphics sfml-system sfml-main)
target_compile_features(stE-testing PUBLIC cxx_std_20)
add_test(FirstTest stE-testing)
set_tests_properties(FirstTest PROPERTIES
TIMEOUT 2
PASS_REGULAR_EXPRESSION 0
FAIL_REGULAR_EXPRESSION 1)

file(COPY "${CMAKE_SOURCE_DIR}/resources" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}") #почему то при запуске теста, рабочая директория запускаемого exe-шника перемещается
file(COPY "${CMAKE_SOURCE_DIR}/config" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}") #из ${CMAKE_CURRENT_BINARY_DIR}/debug в ${CMAKE_CURRENT_BINARY_DIR}, поэтому туда нужно отдельно запихнуть ресурсы

endif()


