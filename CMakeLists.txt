cmake_minimum_required(VERSION 3.20)

set(PROJECT_NAME FluidSimulation)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 23)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

add_subdirectory(extern/glad)

add_executable(${PROJECT_NAME}    
  src/main.cpp
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    glad_loader
    OpenGL::GL
    glfw
)
