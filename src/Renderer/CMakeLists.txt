#[[
Creates a Renderer library
]]
add_library(Renderer
  GridRenderer.cpp
  Shader.cpp
)

#[[
Specifies which include directories will be used when compiling this target
]]
target_include_directories(Renderer
  PUBLIC
    ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(Renderer
  PUBLIC
    glad_loader
    OpenGL::GL
    glfw
    glm
)