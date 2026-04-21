#pragma once

#include <glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <array>

#include <Common.hpp>

/* Class to manage simulation grid.
 *
 * u and velocityY store velocities at borders of the grid squares
 * (that's why there is N_REAL[0]).
 *
 * solid tells us if given square is a solid. If so we don't take use it in the
 * simulation. 
 *
 * Inspiration and reference: https://www.youtube.com/watch?v=iKAVRgIrUOU
 * */
class FluidGrid {
  float overRelaxation{1.7};
  float h;
  int numX;
  int numY;
  int numCells;

  enum FieldType {
    U_FIELD,
    V_FIELD,
    S_FIELD,
  };
public:
  float* u;
  float* v;
  float* newU;
  float* newV;
  int* solid;
  float* smoke;
  float* s;
  float* m;
  float* newM;

  FluidGrid(float h, float overRelaxation, int numX, int numY);

  int getNumX() const;
  int getNumY() const;

  void integrate(float dt, float gravity);
  void injectInlet(float speed);

  void solveIncompressibility(int numIter, float dt);
  /* Extrapolates velocity values near the border to border cells. */
  void extrapolate();

  float avgV(int row, int col);
  float avgU(int row, int col);

  float sampleField(float x, float y, FieldType field);
  void advectVelocity(float dt);
  void advectSmoke(float dt);

  void placeFluid(float x, float y, float radius); 
  void placeFluidRect(float x, float y, float radius); 
  void placeSolid(float x, float y, float len); 

  void zeroSolidVelocities();

  void simulate(float dt, float gravity, int numIters);
};
