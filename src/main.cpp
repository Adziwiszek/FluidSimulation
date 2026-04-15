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

using std::printf;

void ASSERT(bool expr, std::string msg) {
  if(!expr) {
    std::cout << "Failed assert: " << msg << std::endl;
    exit(1);
  }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

bool space_clicked = false;

void processInput(GLFWwindow *window, bool *simulating) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }

  
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !space_clicked) {
    *simulating = !*simulating;
    space_clicked = true;
  }
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE && space_clicked) {
    space_clicked = false;
  }
}

/* 0 -> X
 * 1 -> Y */
constexpr int NDIM = 2;

/* Origin */
constexpr std::array<float, NDIM> Origin = {0.0f, 0.0f};
/* Size of the grid */
constexpr std::array<unsigned int, NDIM> L = {10, 10};
/* Number of cells in each coordinate */
constexpr std::array<unsigned int, NDIM> N = {120, 80};

constexpr size_t WINDOW_WIDTH = L[0] * N [0];
constexpr size_t WINDOW_HEIGHT = L[1] * N [1];

/* Size of each voxel */
constexpr auto D = [] {
  std::array<float, NDIM> d{};
  for (int i = 0; i < NDIM; i++)
    d[i] = static_cast<float>(L[i]) / N[i];
  return d;
}();

glm::mat4 proj = glm::ortho(Origin[0], Origin[0] + L[0], Origin[1], Origin[1] + L[1]);

/* Class to manage simulation grid.
 *
 * velocityX and velocityY store velocities at borders of the grid squares
 * (that's why there is N[0] + 1).
 *
 * solid tells us if given square is a solid. If so we don't take use it in the
 * simulation. 
 *
 * Inspiration and reference: https://www.youtube.com/watch?v=iKAVRgIrUOU
 * */
class FluidGrid {
  float overRelaxation{1.7};
  float h;

  enum FieldType {
    U_FIELD,
    V_FIELD,
    S_FIELD,
  };
public:
  std::array<std::array<float, N[0] + 2>, N[1] + 2> velocityX{};
  std::array<std::array<float, N[0] + 2>, N[1] + 2> velocityY{};
  std::array<std::array<int, N[0] + 2>, N[1] + 2> solid{};
  std::array<std::array<float, N[0] + 2>, N[1] + 2> smoke{};

  FluidGrid(float h, float overRelaxation) 
    : h{h}, overRelaxation{overRelaxation} 
  {
    // set solid values for border fields
    for(int row = 0; row <= N[1]; row++) {
      solid[row][0] = 1;
      solid[row][N[0]] = 0;
    }
    for(int col = 0; col <= N[0]; col++) {
      solid[0][col] = 1;
      solid[N[1]][col] = 1;
    }
    for (int row = 1; row < N[1]; row++) {
      for (int col = 1; col < N[0]; col++) {
          solid[row][col] = 0;
          smoke[row][col] = 0.0;
          //smoke[row][col] = ((double)rand()) / RAND_MAX;
          velocityX[row][col] = 0.0;
          velocityY[row][col] = 0.0;
      }
    }
    //injectInlet(1.0);
    //velocityY[3][3] = -1.0;
  }

  void integrate(float dt, float gravity) {
    for (int row = 1; row < N[1]; row++) {
      for (int col = 1; col < N[0]; col++) {
        if(solid[row][col] == 0.0 && solid[row - 1][col] == 0.0) {
          velocityY[row][col] -= gravity * dt;
        }
      }
    }
  }

  void solveIncompressibility(int numIter, float dt) {
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
                    velocityY[row - 1][col] - velocityY[row][col];
          //printf("d = %f\n", d);
          total_div += d;
          float p = overRelaxation * (-d / s);

          velocityX[row][col] += sr * p;
          velocityX[row][col - 1] -= sl * p;
          velocityY[row - 1][col] += su * p;
          velocityY[row][col] -= sd * p;
        }
      }
      std::cout << "divergence = " << total_div << "\n";
    }
  }

  /* Extrapolates velocity values near the border to border cells. */
  void extrapolate() {
    for(int row = 0; row <= N[1]; row++) {
      velocityX[row][0] = velocityX[row][1];
      velocityX[row][N[0]] = velocityX[row][N[0] - 1];
    }
    for(int col = 0; col <= N[0]; col++) {
      velocityY[0][col] = velocityY[1][col];
      velocityY[N[1]][col] = velocityY[N[1] - 1][col];
    }
  }

  float sampleField(float x, float y, FieldType field) {
    float h1 = 1.0 / h;
    float h2 = 0.5 * h;

    x = std::max(std::min(x, static_cast<float>(N[0]) * h), h); 
    y = std::max(std::min(y, static_cast<float>(N[1]) * h), h); 

    float dx = 0.0;
    float dy = 0.0;

    switch(field) {
      case U_FIELD: dy = h2; break;
      case V_FIELD: dx = h2; break;
      case S_FIELD: dx = h2; dy = h2; break;
    }

    int x0 = std::min(
        static_cast<int>(std::floor((x - dx) * h1)), 
        static_cast<int>(N[0]) - 1
        );
    float tx = ((x - dx) - x0 * h) * h1;
    int x1 = std::min(x0 + 1, static_cast<int>(N[0]) - 1);

    int y0 = std::min(
        static_cast<int>(std::floor((y - dy) * h1)), 
        static_cast<int>(N[1]) - 1
        );
    float ty = ((y - dy) - y0 * h) * h1;
    int y1 = std::min(y0 + 1, static_cast<int>(N[1]) - 1);

    float sx = 1.0 - tx;
    float sy = 1.0 - ty;

    switch(field) {
      case U_FIELD:
        return
          sx * sy * (velocityX[y0][x0]) +
          tx * sy * (velocityX[y0][x1]) +
          tx * ty * (velocityX[y1][x1]) +
          sx * ty * (velocityX[y1][x0]);
      case V_FIELD:
        return
          sx * sy * (velocityY[y0][x0]) +
          tx * sy * (velocityY[y0][x1]) +
          tx * ty * (velocityY[y1][x1]) +
          sx * ty * (velocityY[y1][x0]);
      case S_FIELD:
        return
          sx * sy * (smoke[y0][x0]) +
          tx * sy * (smoke[y0][x1]) +
          tx * ty * (smoke[y1][x1]) +
          sx * ty * (smoke[y1][x0]);
    } 
    assert(false);
  }

  void advectVelocity(float dt) {
    std::array<std::array<float, N[0] + 2>, N[1] + 2> newVelocityX{};
    std::array<std::array<float, N[0] + 2>, N[1] + 2> newVelocityY{};

    float h2 = 0.5 * h;

    for (int row = 1; row < N[1]; row++) {
      for (int col = 1; col < N[0]; col++) {
        // horizontal component
        if (solid[row][col] == 0.0 && solid[row][col - 1] == 0.0 &&
            row < N[1] - 1) {
          float x = col * h ;
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

  void advectSmoke(float dt) {
    std::array<std::array<float, N[0] + 2>, N[1] + 2> newSmoke{};
    float h2 = 0.5 * h;

    for (int row = 1; row < N[1]; row++) {
      for (int col = 1; col < N[0]; col++) {
        if(solid[row][col] == 0) {
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

  void placeSmoke(float x, float y, float radius) {
  }

  float avgVelocityY(int row, int col) {
    return (
      velocityY[row][col] +
      velocityY[row + 1][col] +
      velocityY[row][col - 1] +
      velocityY[row + 1][col - 1]
      ) * 0.25;
  }

  float avgVelocityX(int row, int col) {
    return (
      velocityX[row][col] +
      velocityX[row][col + 1] +
      velocityX[row - 1][col] +
      velocityX[row - 1][col + 1]
      ) * 0.25;
  }

  void injectInlet(float speed) {
    int mid = N[1] / 2;
    int r = N[1] / 16;
    for (int row = mid - r; row <= mid + r; row++) {
        velocityX[row][0] = speed;
        velocityY[row][0] = 0.0;
        smoke[row][1] = 1;
    }
  }

  void applyBoundaryConditions() {
    for (int i = 0; i <= N[0]; i++) {
      velocityY[0][i] = 0;
      velocityY[N[1]][i] = 0;
    }
    for (int j = 0; j <= N[1]; j++) {
      velocityX[j][0] = 0;
      velocityX[j][N[0]] = 0;
    }
  }

  void simulate(float dt, float gravity, int numIters) {
    injectInlet(10);

    integrate(dt, gravity);

    solveIncompressibility(numIters, dt);
    //applyBoundaryConditions();

    extrapolate();
    advectVelocity(dt);
    advectSmoke(dt);
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
  unsigned int solidTexture;
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

  unsigned int getSmokeTexture() const {
    return smokeTexture;
  }

  unsigned int getSolidTexture() const {
    return smokeTexture;
  }
   
  void buildGrid() {
    indices.clear();
    vertices.clear();

    for(unsigned int i = 0; i <= N[0]; i++) {
      for(unsigned int j = 0; j <= N[1]; j++) {
        float x = Origin[0] + i * D[0];
        float y = Origin[1] + j * D[1];
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
      glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello OpenGL", nullptr, nullptr);
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

  glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  Shader shader("shaders/vertexShader.glsl", "shaders/fragmentShader.glsl");

  FluidGrid simulation(D[0], 1.9);
  GridRenderer renderer(simulation);
  renderer.buildGrid();

  bool simulating = true;
  float gravity = 0.0;
  int numIters = 10;
  auto prevTime = std::chrono::high_resolution_clock::now();

  while (!glfwWindowShouldClose(window)) {
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> dtChrono = currentTime - prevTime;
    prevTime = currentTime;
    float dt = dtChrono.count();
    float sim_dt = dt;
    std::cout << "delta time = " << sim_dt << ", FPS = " << 1.0 / dt << "\n";

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    processInput(window, &simulating);

    if (simulating) {
      simulation.simulate(sim_dt, gravity, numIters); 
    }

    renderer.updateTexture();

    shader.use();
    shader.setMat4("proj", proj);
    shader.setInt("smokeMap", 0);


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderer.getSolidTexture());

    renderer.draw();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
