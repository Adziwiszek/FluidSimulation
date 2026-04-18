#pragma once

#include <glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <array>

#include <Common.hpp>

/* Class to manage simulation grid.
 *
 * velocityX and velocityY store velocities at borders of the grid squares
 * (that's why there is N[0] + 1).
 *
 * solid tells us if given square is a solid. If so we don't take use it in the
 * simulation. 
 *
 * Inspiration and reference: https://www.youtube.com/watch?v=iKAVRgIrUOU
 * */
class FluidGrid {
  float overRelaxation{1.7};
  float h;

  enum FieldType {
    U_FIELD,
    V_FIELD,
    S_FIELD,
  };
public:
  std::array<std::array<float, N[0] + 2>, N[1] + 2> velocityX{};
  std::array<std::array<float, N[0] + 2>, N[1] + 2> velocityY{};
  std::array<std::array<int, N[0] + 2>, N[1] + 2> solid{};
  std::array<std::array<float, N[0] + 2>, N[1] + 2> smoke{};

  FluidGrid(float h, float overRelaxation) ;

  void integrate(float dt, float gravity);
  void injectInlet(float speed);

  void solveIncompressibility(int numIter, float dt);
  /* Extrapolates velocity values near the border to border cells. */
  void extrapolate();

  float avgVelocityY(int row, int col);
  float avgVelocityX(int row, int col);

  float sampleField(float x, float y, FieldType field);
  void advectVelocity(float dt);
  void advectSmoke(float dt);

  void placeFluid(float x, float y, float radius); 
  void placeFluidRect(float x, float y, float radius); 
  void placeSolid(float x, float y, float len); 

  void simulate(float dt, float gravity, int numIters);
};
