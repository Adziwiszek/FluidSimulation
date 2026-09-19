#pragma once

#include "AbstractSolver.hpp"
#include <Common.hpp>

namespace Solver {

/**
 * Gauss-Seidel method implementation.
 */
class GaussSeidel : public AbstractSolver {
public:
  ~GaussSeidel();
  static void solvePressure(int numIter, float dt, float *pressure, float *s, float *v, float *u, simConstants constants);
};

}