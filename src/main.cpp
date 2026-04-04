#include <glad.h>
#include <GLFW/glfw3.h>
#include <Shader.hpp>
#include <array>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include <print>
#include <chrono>
#include <vector>
#include <bits/stdc++.h>

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

/* 0 -> X
 * 1 -> Y */
constexpr int NDIM = 2;

/* Origin */
constexpr std::array<float, NDIM> O = {0.0f, 0.0f};
/* Size of the grid */
constexpr std::array<unsigned int, NDIM> L = {10, 10};
/* Number of cells in each coordinate */
constexpr std::array<unsigned int, NDIM> N = {100, 100};

/* Size of each voxel */
constexpr auto D = [] {
  std::array<float, NDIM> d{};
  for (int i = 0; i < NDIM; i++)
    d[i] = static_cast<float>(L[i]) / N[i];
  return d;
}();

glm::mat4 proj = glm::ortho(O[0], O[0] + L[1], O[1], O[1] + L[1]);

/* Class to manage simulation grid.
 *
 * velocityX and velocityY store velocities at borders of the grid squares
 * (that's why there is N[0] + 1).
 *
 * solid tells us if given square is a solid. If so we don't take use it in the
 * simulation. */
class FluidGrid {
  float overRelaxation{1.9};
public:
  std::array<std::array<float, N[0] + 1>, N[1]> velocityX{};
  std::array<std::array<float, N[1] + 1>, N[0]> velocityY{};
  std::array<std::array<int, N[0] + 2>, N[1] + 2> solid{};
  std::array<std::array<float, N[0]>, N[1]> smoke{};

  FluidGrid() {
    for(int j = 0; j < N[1]; j++) {
      for(int i = 0; i < N[0]; i++) {
        smoke[j][i] = ((double)rand()) / RAND_MAX;
      }
    }
  }

  void solveIncompressibility(int numIter, float dt) {
    float total_div = 0;
    for (int ni = 0; ni < numIter; ni++) {
      for (int row = 1; row < N[1]; row++) {
        for (int col = 1; col < N[0]; col++) {
          if (solid[row][col])
            continue;

          auto sl = solid[row][col - 1];
          auto sr = solid[row][col + 1];
          auto su = solid[row - 1][col];
          auto sd = solid[row + 1][col];
          auto s = sl + sr + su + sd;
          if (s == 0.0)
            continue;

          float d = velocityX[row][col] - velocityX[row][col - 1] +
                    velocityY[row - 1][col] - velocityY[row][col];
          total_div += d;
          float p = overRelaxation * (-d / s);

          velocityX[row][col] -= sr * p;
          velocityX[row][col - 1] -= sl * p;
          velocityY[row - 1][col] -= su * p;
          velocityY[row][col] -= sd * p;
        }
      }
    }
    std::cout << "divergence = " << total_div << "\r";
    
  }

  void advectVelocity(float dt) {
  }

  void advectSmoke(float dt) {
  }

  void placeSmoke(float x, float y, float radius) {
  }
};

class GridRenderer {
private:
  const FluidGrid& grid;
  std::vector<glm::vec3> vertices;
  std::vector<unsigned int> indices;

  unsigned int vertexBuffer;
  unsigned int vertexArray;
  unsigned int elementBuffer;

  unsigned int smokeTexture;
  unsigned int uvBuffer;
public:
  GridRenderer(const FluidGrid& grid) : grid{grid} {
    glGenVertexArrays(1, &vertexArray);
    glGenBuffers(1, &vertexBuffer);
    glGenBuffers(1, &elementBuffer);
    glGenBuffers(1, &uvBuffer);

    glGenTextures(1, &smokeTexture);
    glBindTexture(GL_TEXTURE_2D, smokeTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  }

  unsigned int getTexture() const {
    return smokeTexture;
  }
   
  void buildGrid() {
    indices.clear();
    vertices.clear();

    for(unsigned int i = 0; i <= N[0]; i++) {
      for(unsigned int j = 0; j <= N[1]; j++) {
        float x = O[0] + i * D[0];
        float y = O[1] + j * D[1];
        vertices.push_back({x, y, 0});
      }
    }
    for(unsigned int i = 0; i < N[0]; i++) {
      for(unsigned int j = 0; j < N[1]; j++) {
        indices.push_back(i + j * (N[0]+1));
        indices.push_back(i + 1 + j * (N[0]+1));
        indices.push_back(i + 1 + (j + 1) * (N[0]+1));

        indices.push_back(i + j * (N[0]+1));
        indices.push_back(i + (j + 1) * (N[0]+1));
        indices.push_back(i + 1 + (j + 1) * (N[0]+1));
      }
    }

    std::vector<glm::vec2> uvs;
    for (unsigned int i = 0; i <= N[0]; i++) {
      for (unsigned int j = 0; j <= N[1]; j++) {
        float u = static_cast<float>(i) / N[0];
        float v = static_cast<float>(j) / N[1];
        uvs.push_back({u, v});
      }
    }

    glBindVertexArray(vertexArray);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(),
                 vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * uvs.size(),
                 uvs.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(),
                 indices.data(), GL_STATIC_DRAW);
  }

  void updateTexture() {
    std::vector<float> smokeData(N[0] * N[1]);
    for (unsigned int j = 0; j < N[1]; j++)
        for (unsigned int i = 0; i < N[0]; i++)
            smokeData[j * N[0] + i] = static_cast<float>(grid.smoke[j][i]);

    glBindTexture(GL_TEXTURE_2D, smokeTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, N[0], N[1], 0,
                 GL_RED, GL_FLOAT, smokeData.data());
  }

  void draw() {
    glBindVertexArray(vertexArray);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
  }
};

int main() {
  srand(67);
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

  glViewport(0, 0, 800, 800);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  Shader shader("shaders/vertexShader.glsl", "shaders/fragmentShader.glsl");

  FluidGrid simulation;
  GridRenderer renderer(simulation);
  renderer.buildGrid();

  bool simulating = true;
  auto prevTime = std::chrono::high_resolution_clock::now();

  while (!glfwWindowShouldClose(window)) {
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> dtChrono = currentTime - prevTime;
    prevTime = currentTime;
    float dt = dtChrono.count();
    // std::cout << "delta time = " << dt << "\r";

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (simulating) {
      simulation.solveIncompressibility(10, dt);
    }

    renderer.updateTexture();

    shader.use();
    shader.setMat4("proj", proj);
    shader.setInt("smokeMap", 0);


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderer.getTexture());

    renderer.draw();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
