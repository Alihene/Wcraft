cmake_minimum_required(VERSION 3.16)

add_subdirectory("thirdparty/SDL")
add_subdirectory("thirdparty/stb_image")

project("software-renderer")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

add_executable(${CMAKE_PROJECT_NAME} src/main.c)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE SDL3-shared stb_image m)