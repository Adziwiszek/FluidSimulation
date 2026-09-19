#include "GaussSeidel.hpp"

namespace Solver {

GaussSeidel::~GaussSeidel() {

}

static void solvePressure(int numIter, float dt, float *pressure, float *s, float *v, float *u, simConstants constants) {
  const int n = constants.numX;

  std::fill(pressure, pressure + constants.numCells, 0.0f);

  for (int ni = 0; ni < numIter; ni++) {
    float totalDiv = 0.0f;
    for (int i = 1; i < constants.numX - 1; i++) {
      for (int j = 1; j < constants.numY - 1; j++) {
        if (s[j * n + i] == 0.0f)
          continue;

        const float fluidLeft = s[j * n + i - 1];
        const float fluidRight = s[j * n + i + 1];
        const float fluidBottom = s[(j - 1) * n + i];
        const float fluidTop = s[(j + 1) * n + i];
        const float stotal = fluidLeft + fluidRight + fluidBottom + fluidTop;

        if (stotal == 0.0f)
          continue;

        const float uR = u[j * n + i + 1] * fluidRight;
        const float uL = u[j * n + i] * fluidLeft;
        const float vT = v[(j + 1) * n + i] * fluidTop;
        const float vB = v[j * n + i] * fluidBottom;
        const float pressureLeft = pressure[j * n + i - 1] * fluidLeft;
        const float pressureRight = pressure[j * n + i + 1] * fluidRight;
        const float pressureBottom = pressure[(j - 1) * n + i] * fluidBottom;
        const float pressureTop = pressure[(j + 1) * n + i] * fluidTop;

        const float divergence = uR - uL + vT - vB;
        totalDiv += std::abs(divergence);

        const float pressureSum =
            pressureLeft + pressureRight + pressureTop + pressureBottom;
        const float rhs = (constants.density * constants.h / dt) * divergence;
        const float newPressure = (pressureSum - rhs) / stotal;

        const float oldPressure = pressure[j * n + i];
        pressure[j * n + i] = oldPressure + constants.overRelaxation * (newPressure - oldPressure);
      }
    }
  }
}

}