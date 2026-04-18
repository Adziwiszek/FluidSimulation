#include "Common.hpp"
#include <FluidGrid.hpp>

#include <algorithm>
#include <print>

using std::printf;

FluidGrid::FluidGrid(float h, float overRelaxation)
    : h{h}, overRelaxation{overRelaxation} {
  // set solid values for border fields
  for (int row = 0; row <= N[1]; row++) {
    solid[row][0] = 1;
    solid[row][N[0]] = 0;
  }
  for (int col = 0; col <= N[0]; col++) {
    solid[0][col] = 1;
    solid[N[1]][col] = 1;
  }
  for (int row = 1; row < N[1]; row++) {
    for (int col = 1; col < N[0]; col++) {
      solid[row][col] = 0;
      smoke[row][col] = 0.0;
      velocityX[row][col] = 0.0;
      velocityY[row][col] = 0.0;
    }
  }
}

void FluidGrid::integrate(float dt, float gravity) {
  for (int row = 1; row < N[1]; row++) {
    for (int col = 1; col < N[0]; col++) {
      if (solid[row][col] == 0.0 && solid[row - 1][col] == 0.0) {
        velocityY[row][col] -= gravity * dt;
      }
    }
  }
}

void FluidGrid::solveIncompressibility(int numIter, float dt) {
  for (int ni = 0; ni < numIter; ni++) {
    float total_div = 0;
    for (int row = 1; row < N[1]; row++) {
      for (int col = 1; col < N[0]; col++) {
        if (solid[row][col] == 1)
          continue;

        auto sl = 1 - solid[row][col - 1];
        auto sr = 1 - solid[row][col + 1];
        auto su = 1 - solid[row - 1][col];
        auto sd = 1 - solid[row + 1][col];
        auto s = sl + sr + su + sd;

        if (s == 0.0)
          continue;

        float d = velocityX[row][col] - velocityX[row][col - 1] +
                  -velocityY[row - 1][col] + velocityY[row][col];
        // printf("d = %f\n", d);
        total_div += d;
        float p = overRelaxation * (-d / s);

        velocityX[row][col] += sr * p;
        velocityX[row][col - 1] -= sl * p;
        velocityY[row - 1][col] -= su * p;
        velocityY[row][col] += sd * p;
      }
    }
    // printf("Divergence = %f\n", total_div);
  }
}

/* Extrapolates velocity values near the border to border cells. */
void FluidGrid::extrapolate() {
  for (int row = 0; row <= N[1]; row++) {
    velocityX[row][0] = velocityX[row][1];
    velocityX[row][N[0]] = velocityX[row][N[0] - 1];
  }
  for (int col = 0; col <= N[0]; col++) {
    velocityY[0][col] = velocityY[1][col];
    velocityY[N[1]][col] = velocityY[N[1] - 1][col];
  }
}

float FluidGrid::sampleField(float x, float y, FieldType field) {
  float h1 = 1.0 / h;
  float h2 = 0.5 * h;

  x = std::max(std::min(x, static_cast<float>(N[0]) * h), h);
  y = std::max(std::min(y, static_cast<float>(N[1]) * h), h);

  float dx = 0.0;
  float dy = 0.0;

  switch (field) {
  case U_FIELD:
    dy = h2;
    break;
  case V_FIELD:
    dx = h2;
    break;
  case S_FIELD:
    dx = h2;
    dy = h2;
    break;
  }

  int x0 = std::min(static_cast<int>(std::floor((x - dx) * h1)),
                    static_cast<int>(N[0]) - 1);
  float tx = ((x - dx) - x0 * h) * h1;
  int x1 = std::min(x0 + 1, static_cast<int>(N[0]) - 1);

  int y0 = std::min(static_cast<int>(std::floor((y - dy) * h1)),
                    static_cast<int>(N[1]) - 1);
  float ty = ((y - dy) - y0 * h) * h1;
  int y1 = std::min(y0 + 1, static_cast<int>(N[1]) - 1);

  float sx = 1.0 - tx;
  float sy = 1.0 - ty;

  switch (field) {
  case U_FIELD:
    return sx * sy * (velocityX[y0][x0]) + tx * sy * (velocityX[y0][x1]) +
           tx * ty * (velocityX[y1][x1]) + sx * ty * (velocityX[y1][x0]);
  case V_FIELD:
    return sx * sy * (velocityY[y0][x0]) + tx * sy * (velocityY[y0][x1]) +
           tx * ty * (velocityY[y1][x1]) + sx * ty * (velocityY[y1][x0]);
  case S_FIELD:
    return sx * sy * (smoke[y0][x0]) + tx * sy * (smoke[y0][x1]) +
           tx * ty * (smoke[y1][x1]) + sx * ty * (smoke[y1][x0]);
  }
  assert(false);
}

void FluidGrid::advectVelocity(float dt) {
  std::array<std::array<float, N[0] + 2>, N[1] + 2> newVelocityX{};
  std::array<std::array<float, N[0] + 2>, N[1] + 2> newVelocityY{};

  float h2 = 0.5 * h;

  for (int row = 1; row < N[1]; row++) {
    for (int col = 1; col < N[0]; col++) {
      // horizontal component
      if (solid[row][col] == 0.0 && solid[row][col - 1] == 0.0 &&
          row < N[1] - 1) {
        float x = col * h;
        float y = row * h + h2;
        float u = velocityX[row][col];
        float v = avgVelocityY(row, col);
        x = x - dt * u;
        y = y - dt * v;
        u = sampleField(x, y, U_FIELD);
        newVelocityX[row][col] = u;
      }
      // vertical component
      if (solid[row][col] == 0.0 && solid[row - 1][col] == 0.0 &&
          col < N[0] - 1) {
        float x = col * h + h2;
        float y = row * h;
        float u = avgVelocityX(row, col);
        float v = velocityY[row][col];
        x = x - dt * u;
        y = y - dt * v;
        v = sampleField(x, y, V_FIELD);
        newVelocityY[row][col] = v;
      }
    }
  }

  velocityX = newVelocityX;
  velocityY = newVelocityY;
}

void FluidGrid::advectSmoke(float dt) {
  std::array<std::array<float, N[0] + 2>, N[1] + 2> newSmoke{};
  float h2 = 0.5 * h;

  for (int row = 1; row < N[1]; row++) {
    for (int col = 1; col < N[0]; col++) {
      if (solid[row][col] == 0) {
        float u = avgVelocityX(row, col);
        float v = avgVelocityY(row, col);
        float x = col * h + h2 - dt * u;
        float y = row * h + h2 - dt * v;
        newSmoke[row][col] = sampleField(x, y, S_FIELD);
      }
    }
  }
  smoke = newSmoke;
}

void FluidGrid::placeSolid(float x, float y, float len) {
  int topLeftX = utils::max<int>(x, 1.0f);
  int topLeftY = utils::max<int>(y, 1.0f);
  int rightBound = utils::min<int>(topLeftX + len, N[0] - 1);
  int downBound = utils::min<int>(topLeftY + len, N[1] - 1);

  for (int row = topLeftY; row <= downBound; row++) {
    for (int col = topLeftX; col <= rightBound; col++) {
      solid[row][col] = 1;
    }
  }
}

void FluidGrid::placeFluidRect(float x, float y, float len) {
  int topLeftX = utils::max<int>(x, 1.0f);
  int topLeftY = utils::max<int>(y, 1.0f);
  int rightBound = utils::min<int>(topLeftX + len, N[0] - 1);
  int downBound = utils::min<int>(topLeftY + len, N[1] - 1);

  for (int row = topLeftY; row <= downBound; row++) {
    for (int col = topLeftX; col <= rightBound; col++) {
      smoke[row][col] = 1;
    }
  }
}

void FluidGrid::placeFluid(float x, float y, float radius) {
  int topLeftX = utils::max<int>(x - radius, 1.0f);
  int topLeftY = utils::max<int>(y - radius, 1.0f);
  int rightBound = utils::min<int>(x + radius, N[0] - 1);
  int downBound = utils::min<int>(y + radius, N[1] - 1);

  for (int row = topLeftY; row <= downBound; row++) {
    for (int col = topLeftX; col <= rightBound; col++) {
      if (utils::euclid_dist(x, y, col, row) >= radius)
        continue;
      smoke[row][col] = 1;
    }
  }
}

float FluidGrid::avgVelocityY(int row, int col) {
  return (velocityY[row][col] + velocityY[row + 1][col] +
          velocityY[row][col - 1] + velocityY[row + 1][col - 1]) *
         0.25;
}

float FluidGrid::avgVelocityX(int row, int col) {
  return (velocityX[row][col] + velocityX[row][col + 1] +
          velocityX[row - 1][col] + velocityX[row - 1][col + 1]) *
         0.25;
}

void FluidGrid::injectInlet(float speed) {
  int mid = N[1] / 2;
  int r = N[1] / 16;
  for (int row = mid - r; row <= mid + r; row++) {
    velocityX[row][0] = speed;
    velocityX[row][1] = speed;
    smoke[row][1] = 1;
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
