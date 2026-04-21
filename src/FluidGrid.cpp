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

int FluidGrid::getNumX() const { return numX; }
int FluidGrid::getNumY() const { return numY; }

void FluidGrid::integrate(float dt, float gravity) {
  int n = numX;
  for (int i = 1; i < numX - 1; i++) {
    for (int j = 1; j < numY - 1; j++) {
      if (s[j * n + i] != 0.0 && s[(j - 1) * n + i] != 0.0)
        v[j * n + i] += gravity * dt;
    }
  }
}

void FluidGrid::solveIncompressibility(int numIter, float dt) {
  int n = numX;

  for (int ni = 0; ni < numIter; ni++) {
    float total_div = 0;
    for (int i = 1; i < numX - 1; i++) {
      for (int j = 1; j < numY - 1; j++) {
        float s = this->s[j * n + i];
        if (s == 0)
          continue;

        float sx0 = this->s[j * n + i - 1];
        float sx1 = this->s[j * n + i + 1];
        float sy0 = this->s[(j - 1) * n + i];
        float sy1 = this->s[(j + 1) * n + i];
        s = sx0 + sx1 + sy0 + sy1;

        if (s == 0.0)
          continue;

        float div =
            u[j * n + i + 1] - u[j * n + i] + v[(j + 1) * n + i] - v[j * n + i];
        total_div += div;

        float p = -div / s;
        p *= overRelaxation;

        u[j * n + i] -= sx0 * p;
        u[j * n + i + 1] += sx1 * p;
        v[j * n + i] -= sy0 * p;
        v[(j + 1) * n + i] += sy1 * p;
      }
    }
    printf("Divergence = %f\n", total_div);
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
    }
  }
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
  int n = numX;
  int r = 8;
  int mid = numY / 2;
  for (int j = 0; j < numY; j++) {
    if (j >= mid - r && j <= mid + r) {
      m[j * n + 1] = 1.0f;
    }
    u[j * n + 1] = speed;
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
