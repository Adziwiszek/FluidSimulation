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

  newU = new float[numCells];
  newV = new float[numCells];
  newM = new float[numCells];

  std::fill(s, s + numCells, 1.0);
  std::fill(u, u + numCells, 0.0);
  std::fill(v, v + numCells, 0.0);
  std::fill(m, m + numCells, 0.0);

  // set solid values for border fields
  int n = this->numY;
  // top + bottom
  for (int j = 0; j < this->numY; j++) {
    s[0 * n + j] = 0.0f;
    s[(this->numX - 1) * n + j] = 0.0f;
  }

  // left + right
  for (int i = 0; i < this->numX; i++) {
    s[i * n + 0] = 0.0f;
    // s[i * n + (this->numY - 1)] = 0.0f;
  }
}

int FluidGrid::getNumX() const { return numX; }
int FluidGrid::getNumY() const { return numY; }

void FluidGrid::integrate(float dt, float gravity) {
  auto n = numY;
  for (int i = 1; i < numX - 1; i++) {
    for (int j = 1; j < numY - 1; j++) {
      if (s[i * n + j] != 0.0 && s[i * n + j - 1] != 0.0)
        v[i * n + j] += gravity * dt;
    }
  }
}

void FluidGrid::solveIncompressibility(int numIter, float dt) {
  int n = numY;

  for (int ni = 0; ni < numIter; ni++) {
    float total_div = 0;
    for (int i = 1; i < numX - 1; i++) {
      for (int j = 1; j < numY - 1; j++) {
        float currentS = s[i * n + j];
        if (currentS == 0)
          continue;

        float sx0 = s[(i - 1) * n + j];
        float sx1 = s[(i + 1) * n + j];
        float sy0 = s[i * n + j - 1];
        float sy1 = s[i * n + j + 1];
        currentS = sx0 + sx1 + sy0 + sy1;

        if (currentS == 0.0)
          continue;

        float div =
            u[(i + 1) * n + j] - u[i * n + j] + v[i * n + j + 1] - v[i * n + j];
        total_div += div;

        float p = -div / currentS;
        p *= overRelaxation;

        u[i * n + j] -= sx0 * p;
        u[(i + 1) * n + j] += sx1 * p;
        v[i * n + j] -= sy0 * p;
        v[i * n + j + 1] += sy1 * p;
      }
    }
    printf("Divergence = %f\n", total_div);
  }
}

/* Extrapolates velocity values near the border to border cells. */
void FluidGrid::extrapolate() {
  int n = numY;
  for (int i = 0; i < numX; i++) {
    u[i * n + 0] = u[i * n + 1];
    u[i * n + numY - 1] = u[i * n + numY - 2];
  }
  for (int j = 0; j < numY; j++) {
    v[0 * n + j] = v[1 * n + j];
    v[(numX - 1) * n + j] = v[(numX - 2) * n + j];
  }
}

float FluidGrid::sampleField(float x, float y, FieldType field) {
  int n = numY;
  float h1 = 1.0 / h;
  float h2 = 0.5 * h;

  // x = std::max(std::min(x, static_cast<float>(N_REAL[0] - 1) * h), h);
  // y = std::max(std::min(y, static_cast<float>(N_REAL[1] - 1) * h), h);
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
    // maybe todo
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

  float val = sx * sy * f[x0 * n + y0] + tx * sy * f[x1 * n + y0] +
              tx * ty * f[x1 * n + y1] + sx * ty * f[x0 * n + y1];

  return val;
}

float FluidGrid::avgU(int i, int j) {
  int n = numY;
  return (u[i * n + j - 1] + u[i * n + j] + u[(i + 1) * n + j - 1] +
          u[(i + 1) * n + j]) *
         0.25;
}

float FluidGrid::avgV(int i, int j) {
  int n = numY;
  return (v[(i - 1) * n + j] + v[i * n + j] + v[(i - 1) * n + j + 1] +
          v[i * n + j + 1]) *
         0.25;
}

void FluidGrid::advectVelocity(float dt) {
  std::memcpy(newU, u, numCells * sizeof(float));
  std::memcpy(newV, v, numCells * sizeof(float));

  int n = numY;
  float h2 = 0.5 * h;

  for (int i = 1; i < numX; i++) {
    for (int j = 1; j < numY; j++) {
      // horizontal component
      if (s[i * n + j] != 0.0 && s[(i - 1) * n + j] != 0.0 && j < numY - 1) {
        float x = i * h;
        float y = j * h + h2;
        float u = this->u[i * n + j];
        float v = avgV(i, j);
        x = x - dt * u;
        y = y - dt * v;
        u = sampleField(x, y, U_FIELD);
        newU[i * n + j] = u;
      }
      // vertical component
      if (s[i * n + j] != 0.0 && s[i * n + j - 1] != 0.0 && i < numX - 1) {
        float x = i * h + h2;
        float y = j * h;
        float u = avgU(i, j);
        float v = this->v[i * n + j];
        x = x - dt * u;
        y = y - dt * v;
        v = sampleField(x, y, V_FIELD);
        newV[i * n + j] = v;
      }
    }
  }

  std::swap(u, newU);
  std::swap(v, newV);
}

void FluidGrid::advectSmoke(float dt) {
  std::memcpy(newM, m, numCells * sizeof(float));

  int n = numY;
  float h2 = 0.5 * h;

  for (int i = 1; i < numX - 1; i++) {
    for (int j = 1; j < numY - 1; j++) {
      if (s[i * n + j] != 0.0) {
        float u = (this->u[i * n + j] + this->u[(i + 1) * n + j]) * 0.5;
        float v = (this->v[i * n + j] + this->v[i * n + j + 1]) * 0.5;
        float x = i * h + h2 - dt * u;
        float y = j * h + h2 - dt * v;
        newM[i * n + j] = sampleField(x, y, S_FIELD);
      }
    }
  }
  std::swap(m, newM);
}

/*
void FluidGrid::placeSolid(float x, float y, float radius) {
  int topLeftX = utils::max<int>(x - radius, 1.0f);
  int topLeftY = utils::max<int>(y - radius, 1.0f);
  int rightBound = utils::min<int>(x + radius, N_REAL[0] - 1);
  int downBound = utils::min<int>(y + radius, N_REAL[1] - 1);

  for (int row = topLeftY; row <= downBound; row++) {
    for (int col = topLeftX; col <= rightBound; col++) {
      if (utils::euclid_dist(x, y, col, row) >= radius)
        continue;
      solid[row][col] = 1;
    }
  }
}

void FluidGrid::placeFluidRect(float x, float y, float len) {
  int topLeftX = utils::max<int>(x, 1.0f);
  int topLeftY = utils::max<int>(y, 1.0f);
  int rightBound = utils::min<int>(topLeftX + len, N_REAL[0] - 1);
  int downBound = utils::min<int>(topLeftY + len, N_REAL[1] - 1);

  for (int row = topLeftY; row <= downBound; row++) {
    for (int col = topLeftX; col <= rightBound; col++) {
      smoke[row][col] = 1;
    }
  }
}

void FluidGrid::placeFluid(float x, float y, float radius) {
  int topLeftX = utils::max<int>(x - radius, 1.0f);
  int topLeftY = utils::max<int>(y - radius, 1.0f);
  int rightBound = utils::min<int>(x + radius, N_REAL[0] - 1);
  int downBound = utils::min<int>(y + radius, N_REAL[1] - 1);

  for (int row = topLeftY; row <= downBound; row++) {
    for (int col = topLeftX; col <= rightBound; col++) {
      if (utils::euclid_dist(x, y, col, row) >= radius ||
          solid[row][col] > 0)
        continue;
      smoke[row][col] = 1;
    }
  }
}
*/

void FluidGrid::injectInlet(float speed) {
  int n = numY;
  int r = 16;
  int mid = numX / 2;
  for (int i = mid - r; i < mid + r; i++) {
    u[i * n + 1] = speed;
    m[i * n + 1] = 1.0f;
  }
}

void FluidGrid::simulate(float dt, float gravity, int numIters) {
  injectInlet(10);

  integrate(dt, gravity);

  solveIncompressibility(numIters, dt);

  extrapolate();

  advectVelocity(dt);
  advectSmoke(dt);
}
