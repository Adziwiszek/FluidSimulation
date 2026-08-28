#[[
Creates a Solver library
]]
add_library(Solver
  GaussSeidel.cpp
)

#[[
Specifies which include directories will be used when compiling this target
]]
target_include_directories(Solver
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)