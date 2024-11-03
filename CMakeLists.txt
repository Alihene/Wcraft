cmake_minimum_required(VERSION 3.16)

add_subdirectory("thirdparty/SDL")
add_subdirectory("thirdparty/stb_image")
add_subdirectory("thirdparty/cglm")

project("software-renderer")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wpedantic -Wtype-limits -Wstrict-overflow=5 -fstrict-overflow -Wsign-compare -O3 -msse4.1")

add_executable(${CMAKE_PROJECT_NAME} src/main.c src/rendering.c src/camera.c src/window.c src/util.c src/world.c src/noise.c)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE SDL3-shared stb_image cglm m)