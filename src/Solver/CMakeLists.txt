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
    ${PROJECT_SOURCE_DIR}/src
)