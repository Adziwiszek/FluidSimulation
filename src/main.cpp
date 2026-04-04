#include <glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <Shader.hpp>
#include <iostream>
#include <vector>
#include <array>

std::vector<glm::vec3> vertices;
std::vector<unsigned int> indices;

unsigned int vertexBuffer;
unsigned int vertexArray;

unsigned int elementBuffer;

/* source: https://faun.pub/draw-circle-in-opengl-c-2da8d9c2c103 
 * access: 03.04.2026 */
void buildCircle(float radius, int vCount) {
  float angle = 360.0f / vCount;

  int triangleCount = vCount - 2;

  // positions
  for (int i = 0; i < vCount; i++)
  {
      float currentAngle = angle * i;
      float x = radius * cos(glm::radians(currentAngle));
      float y = radius * sin(glm::radians(currentAngle));
      float z = 0.0f;

      vertices.push_back(glm::vec3(x, y, z));
  }

  for (int i = 0; i < triangleCount; i++)
  {
      indices.push_back(0);
      indices.push_back(i + 1);
      indices.push_back(i + 2);
  }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

/* 0 -> X
 * 1 -> Y */
constexpr int NDIM = 2;

/* Origin */
constexpr std::array<float, NDIM>        O = {0.0f, 0.0f};
/* Size of the grid */
constexpr std::array<unsigned int, NDIM> L = {10, 10};
/* Number of cells in each coordinate */
constexpr std::array<unsigned int, NDIM> N = {10, 10};

/* Size of each voxel */
constexpr auto D = [] {
  std::array<float, NDIM> d{};
  for(int i = 0; i < NDIM; i++)
    d[i] = static_cast<float>(L[i]) / N[i];
  return d;
}();

glm::mat4 proj = glm::ortho(
    O[0], O[0] + L[1],
    O[1], O[1] + L[1]
);

/* Class to manage simulation grid.
 *
 * velocityX and velocityY store velocities at borders of the grid squares
 * (that's why there is N[0] + 1).
 *
 * solid tells us if given square is a solid. If so we don't take use it in the
 * simulation. */
class FluidGrid {
private:
  float overRelaxation{1.9};
public:
  std::array<std::array<float, N[0] + 1>, N[1]> velocityX{};
  std::array<std::array<float, N[1] + 1>, N[0]> velocityY{};
  std::array<std::array<int, N[0] + 2>, N[1] + 2> solid{};

  void solveIncompressibility(int numIter, float dt) {
    for(int ni = 0; ni < numIter; ni++) {
      for(int row = 1; row < N[1]; row++) {
        for(int col = 1; col < N[0]; col++) {
          if(solid[row][col])
            continue;

          auto sl = solid[row][col-1];
          auto sr = solid[row][col+1];
          auto su = solid[row-1][col];
          auto sd = solid[row+1][col];
          auto s = sl + sr + su + sd;
          if(s == 0.0)
            continue;

          float d = velocityX[row][col] - velocityX[row][col-1] +
                   velocityY[row-1][col] - velocityY[row][col];
          float p = overRelaxation * (- d / s);

          velocityX[row][col]   -= sr * p;
          velocityX[row][col-1] -= sl * p;
          velocityY[row-1][col] -= su * p;
          velocityY[row][col]   -= sd * p;
        }
      }
    }
  }
};

int main() {
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW\n";
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(800, 800, "Hello OpenGL", nullptr, nullptr);
  if (!window) {
    std::cerr << "Failed to create window\n";
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD\n";
    return -1;
  }

  Shader shader("shaders/vertexShader.glsl", "shaders/fragmentShader.glsl");

  buildCircle(0.1, 32);
  glm::vec3 circleCenter = {5, 5, 0};

  // buffer
  glGenVertexArrays(1, &vertexArray);
  glGenBuffers(1, &vertexBuffer);

  // element buffer object
  glGenBuffers(1, &elementBuffer);

  glBindVertexArray(vertexArray);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

  // copying
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(),
      &vertices[0], GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(),
      &indices[0], GL_STATIC_DRAW);


  glViewport(0, 0, 800, 800);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  bool simulating = true;

  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if(simulating) {
    }

    shader.use();
    shader.setMat4("proj", proj);
    shader.setVec3("offset", circleCenter);

    glBindVertexArray(vertexArray);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
