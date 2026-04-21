#pragma once

#include <array>
#include <stddef.h>
#include <cmath>

/* 0 -> X
 * 1 -> Y */

constexpr int NDIM = 2;

constexpr size_t WINDOW_WIDTH = 1200;
constexpr size_t WINDOW_HEIGHT = 800;

/* Origin */
constexpr std::array<float, NDIM> Origin = {0.0f, 0.0f};
/* Number of cells in each coordinate */
constexpr std::array<unsigned int, NDIM> N = {200, 150};
constexpr std::array<unsigned int, NDIM> N_REAL = {N[0] + 2, N[1] + 2};
/* Size of the grid */
constexpr std::array<unsigned int, NDIM> L = {
  WINDOW_WIDTH / N_REAL[0], 
  WINDOW_HEIGHT / N_REAL[1]
};


/* Size of each voxel */
constexpr auto D = [] {
  std::array<float, NDIM> d{};
  for (int i = 0; i < NDIM; i++)
    d[i] = static_cast<float>(L[i]) / N_REAL[i];
  return d;
}();


namespace utils {
  struct CursorPos {
    double x;
    double y;
    void toWorldCoordinates(); 
  };

  template<typename RET, typename T, typename T_>
  RET max(T a, T_ b_) {
    T b = static_cast<T>(b_);
    return static_cast<RET>((a > b) ? a : b);
  }

  template<typename RET, typename T, typename T_>
  RET min(T a, T_ b_) {
    T b = static_cast<T>(b_);
    return static_cast<RET>((a < b) ? a : b);
  }

  float euclid_dist(float x1, float y1, float x2, float y2);

};
