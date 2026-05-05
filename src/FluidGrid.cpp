#include "Common.hpp"
#include <FluidGrid.hpp>

#include <algorithm>
#include <cstring>
#include <print>

using std::printf;

FluidGrid::FluidGrid(float h, float overRelaxation, int numX, int numY)
    : h{h}, overRelaxation{overRelaxation} {
  this->numX = numX + 2;
  this->numY = numY + 2;
  numCells = this->numX * this->numY;

  u = new float[numCells];
  v = new float[numCells];
  m = new float[numCells];
  s = new float[numCells];
  pressure = new float[numCells];

  newU = new float[numCells];
  newV = new float[numCells];
  newM = new float[numCells];

  std::fill(s, s + numCells, 1.0);
  std::fill(u, u + numCells, 0.0);
  std::fill(v, v + numCells, 0.0);
  std::fill(m, m + numCells, 0.0);
  std::fill(pressure, pressure + numCells, 0.0);

  // set solid values for border fields
  int n = this->numX;
  // left + right
  for (int j = 0; j < this->numY; j++) {
    s[j * n + 0] = 0.0f;
    // s[j * n + (this->numX - 1)] = 0.0f;
  }
  // top + bottom
  for (int i = 0; i < this->numX; i++) {
    s[0 * n + i] = 0.0f;
    s[(this->numY - 1) * n + i] = 0.0f;
  }

  placeSolid(75, 75, 15.0);
}

void FluidGrid::updateMaxVelocity(float vel) {
  maxVelocity = std::max(maxVelocity, std::abs(vel));
}

int FluidGrid::getNumX() const { return numX; }
int FluidGrid::getNumY() const { return numY; }

void FluidGrid::integrate(float dt, float gravity) {
  int n = numX;
  for (int i = 1; i < numX - 1; i++) {
    for (int j = 1; j < numY - 1; j++) {
      if (s[j * n + i] != 0.0 && s[(j - 1) * n + i] != 0.0 &&
          m[j * n + i] > 0.0) {
        v[j * n + i] += gravity * dt;
      }
    }
  }
}

void FluidGrid::solvePressure(int numIter, float dt) {
  int n = numX;

  std::fill(pressure, pressure + numCells, 0.0f);

  for (int ni = 0; ni < numIter; ni++) {
    float totalDiv = 0.0;
    for (int i = 1; i < numX - 1; i++) {
      for (int j = 1; j < numY - 1; j++) {
        if (s[j * n + i] == 0.0f)
          continue;

        float fluidLeft = s[j * n + i - 1];
        float fluidRight = s[j * n + i + 1];
        float fluidBottom = s[(j - 1) * n + i];
        float fluidTop = s[(j + 1) * n + i];
        float stotal = fluidLeft + fluidRight + fluidBottom + fluidTop;

        if (stotal == 0.0f)
          continue;

        float uR = u[j * n + i + 1] * fluidRight;
        float uL = u[j * n + i] * fluidLeft;
        float vT = v[(j + 1) * n + i] * fluidTop;
        float vB = v[j * n + i] * fluidBottom;
        float pressureLeft = pressure[j * n + i - 1] * fluidLeft;
        float pressureRight = pressure[j * n + i + 1] * fluidRight;
        float pressureBottom = pressure[(j - 1) * n + i] * fluidBottom;
        float pressureTop = pressure[(j + 1) * n + i] * fluidTop;

        float divergence = uR - uL + vT - vB;
        totalDiv += std::abs(divergence);

        float pressureSum =
            pressureLeft + pressureRight + pressureTop + pressureBottom;

        float newPressure =
            (pressureSum - density * h * divergence / dt) / stotal;
        pressure[j * n + i] = (1.0f - overRelaxation) * pressure[j * n + i] +
                              overRelaxation * newPressure;
      }
    }
    printf("divergence = %f\n", totalDiv);
  }
  //???
  for (int j = 0; j < numY; j++) {
    pressure[j * numX + (numX - 1)] = 0.0f;
    pressure[j * numX + (numX - 2)] = 0.0f;
  }
}

void FluidGrid::applyPressure(float dt) {
  int n = numX;
  const float K = dt / (density * h);

  for (int i = 1; i < numX - 1; i++) {
    for (int j = 1; j < numY - 1; j++) {
      if (s[j * n + i] == 0.0f)
        continue;

      // u, horizontal edges
      u[j * n + i] -= K * (pressure[j * n + i] - pressure[j * n + i - 1]);
      // v, vertical edges
      v[j * n + i] -= K * (pressure[j * n + i] - pressure[(j - 1) * n + i]);

      updateMaxVelocity(u[j * n + i]);
      updateMaxVelocity(v[j * n + i]);
    }
  }
}

/* Extrapolates velocity values near the border to border cells. */
void FluidGrid::extrapolate() {
  int n = numX;
  // left + right
  for (int j = 0; j < this->numY; j++) {
    v[j * n + 0] = v[j * n + 1];
    v[j * n + (this->numX - 1)] = v[j * n + (this->numX - 2)];
  }
  // top + bottom
  for (int i = 0; i < this->numX; i++) {
    u[0 * n + i] = u[1 * n + i];
    u[(this->numY - 1) * n + i] = u[(this->numY - 2) * n + i];
  }
}

float FluidGrid::sampleField(float x, float y, FieldType field) {
  int n = numX;
  float h1 = 1.0 / h;
  float h2 = 0.5 * h;

  x = std::clamp(x, h, (float)numX * h);
  y = std::clamp(y, h, (float)numY * h);

  float dx = 0.0;
  float dy = 0.0;
  float *f;

  switch (field) {
  case U_FIELD:
    f = u;
    dy = h2;
    break;
  case V_FIELD:
    f = v;
    dx = h2;
    break;
  case S_FIELD:
    f = m;
    dx = h2;
    dy = h2;
    break;
  }

  int x0 = std::min((int)std::floor((x - dx) * h1), numX - 1);
  float tx = ((x - dx) - x0 * h) * h1;
  int x1 = std::min(x0 + 1, numX - 1);

  int y0 = std::min((int)std::floor((y - dy) * h1), numY - 1);
  float ty = ((y - dy) - y0 * h) * h1;
  int y1 = std::min(y0 + 1, numY - 1);

  float sx = 1.0 - tx;
  float sy = 1.0 - ty;

  float val = sx * sy * f[y0 * n + x0] + tx * sy * f[y0 * n + x1] +
              tx * ty * f[y1 * n + x1] + sx * ty * f[y1 * n + x0];

  return val;
}

float FluidGrid::avgU(int i, int j) {
  int n = numX;
  return (u[(j - 1) * n + i] + u[j * n + i] + u[(j - 1) * n + i + 1] +
          u[j * n + i + 1]) *
         0.25;
}

float FluidGrid::avgV(int i, int j) {
  int n = numX;
  return (v[j * n + i - 1] + v[j * n + i] + v[(j + 1) * n + i - 1] +
          v[(j + 1) * n + i]) *
         0.25;
}

void FluidGrid::advectVelocity(float dt) {
  std::memcpy(newU, u, numCells * sizeof(float));
  std::memcpy(newV, v, numCells * sizeof(float));

  int n = numX;
  float h2 = 0.5 * h;

  for (int i = 1; i < numX; i++) {
    for (int j = 1; j < numY; j++) {
      // horizontal component
      if (s[j * n + i] != 0.0 && s[j * n + i - 1] != 0.0 && j < numY - 1) {
        float x = i * h;
        float y = j * h + h2;
        float u = this->u[j * n + i];
        float v = avgV(i, j);
        x = x - dt * u;
        y = y - dt * v;
        u = sampleField(x, y, U_FIELD);
        newU[j * n + i] = u;
      }
      // vertical component
      if (s[j * n + i] != 0.0 && s[(j - 1) * n + i] != 0.0 && i < numX - 1) {
        float x = i * h + h2;
        float y = j * h;
        float u = avgU(i, j);
        float v = this->v[j * n + i];
        x = x - dt * u;
        y = y - dt * v;
        v = sampleField(x, y, V_FIELD);
        newV[j * n + i] = v;
      }
    }
  }

  std::swap(u, newU);
  std::swap(v, newV);
}

void FluidGrid::advectSmoke(float dt) {
  std::memcpy(newM, m, numCells * sizeof(float));

  int n = numX;
  float h2 = 0.5 * h;

  for (int i = 1; i < numX - 1; i++) {
    for (int j = 1; j < numY - 1; j++) {
      if (s[j * n + i] != 0.0) {
        float u = (this->u[j * n + i] + this->u[j * n + i + 1]) * 0.5;
        float v = (this->v[j * n + i] + this->v[(j + 1) * n + i]) * 0.5;
        float x = i * h + h2 - dt * u;
        float y = j * h + h2 - dt * v;
        newM[j * n + i] = sampleField(x, y, S_FIELD);
      }
    }
  }
  std::swap(m, newM);
}

void FluidGrid::placeSolid(float cx, float cy, float radius) {
  int n = numX;
  int topLeftX = std::max((int)(cx - radius), 1);
  int topLeftY = std::max((int)(cy - radius), 1);
  int rightBound = std::min((int)(cx + radius), numX - 2);
  int downBound = std::min((int)(cy + radius), numY - 2);

  for (int j = topLeftY; j <= downBound; j++) {
    for (int i = topLeftX; i <= rightBound; i++) {
      float dx = i - cx;
      float dy = j - cy;
      if (dx * dx + dy * dy >= radius * radius)
        continue;
      s[j * n + i] = 0.0f;
      m[j * n + i] = 0.0f;
    }
  }
}

void FluidGrid::placeFluid(float cx, float cy, float radius) {
  int n = numX;
  int topLeftX = std::max((int)(cx - radius), 1);
  int topLeftY = std::max((int)(cy - radius), 1);
  int rightBound = std::min((int)(cx + radius), numX - 2);
  int downBound = std::min((int)(cy + radius), numY - 2);

  for (int j = topLeftY; j <= downBound; j++) {
    for (int i = topLeftX; i <= rightBound; i++) {
      float dx = i - cx;
      float dy = j - cy;
      if (dx * dx + dy * dy >= radius * radius || s[j * n + i] == 0.0)
        continue;
      m[j * n + i] = 1.0f;
    }
  }
}

void FluidGrid::injectInlet(float speed) {
  int n = numX;
  int r = 8;
  int mid = numY / 2;
  for (int j = 0; j < numY; j++) {
    if (j >= mid - r && j <= mid + r) {
      m[j * n + 2] = 1.0f;
    }
    u[j * n + 2] = speed;
  }
}

void FluidGrid::simulate(float dt, float gravity, int numIters) {
  // determine dt
  // add forces, modify velocity values
  injectInlet(10);
  integrate(dt, gravity);
  extrapolate();

  // projection (make the fluid incompressible)
  solvePressure(numIters, dt);
  applyPressure(dt);

  // move the velocity field (advection)
  advectVelocity(dt);
  advectSmoke(dt);
}
