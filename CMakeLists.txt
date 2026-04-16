cmake_minimum_required(VERSION 3.20)

set(PROJECT_NAME FluidSimulation)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 23)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

add_subdirectory(extern)

add_executable(${PROJECT_NAME}    
  src/main.cpp
  src/Shader.cpp
  src/GridRenderer.cpp
  src/FluidGrid.cpp
  src/Common.cpp
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    glad_loader
    OpenGL::GL
    glfw
)
