#include <Common.hpp>

namespace utils {
  float euclid_dist(float x1, float y1, float x2, float y2) {
    return std::sqrtf(
        (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)   
      );
  }

  void CursorPos::toWorldCoordinates() {
    float xRatio = x / static_cast<float>(WINDOW_WIDTH);
    float yRatio = y / static_cast<float>(WINDOW_HEIGHT);
    double worldX = 
      static_cast<double>(xRatio * (float)N_REAL[0]);
    double worldY = 
      static_cast<double>(yRatio * (float)N_REAL[1]);
    x = worldX; 
    y = static_cast<float>(N_REAL[1]) - worldY;
  }
}
