cmake_minimum_required(VERSION 3.16)

add_subdirectory("thirdparty/SDL")
add_subdirectory("thirdparty/stb_image")
add_subdirectory("thirdparty/cglm")

project("software-renderer")

set(CMAKE_C_FLAGS "-std=c11 ${CMAKE_C_FLAGS} -Wall -Wpedantic -Wsign-compare -Wno-missing-braces -O3 -ffast-math -msse4.1")

add_executable(${CMAKE_PROJECT_NAME} src/main.c src/rendering.c src/camera.c src/window.c src/util.c src/world.c src/noise.c src/player.c src/xorshift.c src/thread_pool.c)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE SDL3-shared stb_image cglm m)