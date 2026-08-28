add_subdirectory(Solver)
add_subdirectory(Renderer)

add_executable(${PROJECT_NAME}    
  main.cpp
  FluidGrid.cpp
  Common.cpp
  SimSettings.cpp
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    Solver
    Renderer
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)