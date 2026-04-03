cmake_minimum_required(VERSION 3.20)

add_library(glad_loader STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/glad.c)

target_include_directories(glad_loader
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(glad_loader
  PUBLIC
    OpenGL::GL
)
