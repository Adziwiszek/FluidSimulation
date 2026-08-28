cmake_minimum_required(VERSION 3.20)

set(PROJECT_NAME FluidSimulation)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 23)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

add_subdirectory(extern)
add_subdirectory(src)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    glad_loader
    OpenGL::GL
    glfw
    glm
)

target_compile_definitions(${PROJECT_NAME} PRIVATE GLFW_INCLUDE_NONE)