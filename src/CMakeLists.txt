add_subdirectory(Solver)

add_executable(${PROJECT_NAME}    
  main.cpp
  Shader.cpp
  GridRenderer.cpp
  FluidGrid.cpp
  Common.cpp
  SimSettings.cpp
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    Solver
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)