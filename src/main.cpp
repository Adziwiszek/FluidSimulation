#include <glad.h>
#include <GLFW/glfw3.h>
#include <bits/stdc++.h>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include <map>
#include <memory>

#include <Common.hpp>
#include <FluidGrid.hpp>
#include <SimSettings.hpp>
#include <Renderer/GridRenderer.hpp>
#include <Renderer/Shader.hpp>

using std::printf;

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

bool didStep = false;
bool space_clicked = false;
bool lmb_pressed = false;
bool r_pressed = false;
utils::CursorPos curpos;

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

  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS &&
      !lmb_pressed) {
    lmb_pressed = true;
  }

  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE &&
      lmb_pressed) {
    lmb_pressed = false;
  }

  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !r_pressed) {
    r_pressed = true;
  }

  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE && r_pressed) {
    r_pressed = false;
    didStep = false;
  }
}

glm::mat4 proj = glm::ortho(Origin[0], (float)L[1], Origin[1], (float)L[0]);


int main(int argc, char *argv[]) {
  SimType sim = SimType::Tunnel;
  if (argc > 1) {
    std::string arg{argv[1]};
    if (arg == "tunel") {
      sim = SimType::Tunnel;
    }
    if (arg == "box") {
      sim = SimType::Box;
    }
  }

  srand(67);
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW\n";
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
                                        "Hello OpenGL", nullptr, nullptr);
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

  std::unique_ptr<Shader> shader = std::make_unique<Shader>("shaders/vertexShader.glsl", "shaders/fragmentShader.glsl");

  FluidGrid simulation(D[0], 1.9, N[0], N[1]);
  GridRenderer renderer(simulation);
  renderer.buildGrid();

  bool simulating = true;
  int numIters = 120;
  auto prevTime = std::chrono::high_resolution_clock::now();

  simulation.initialize(SETTINGS.at(sim));

  while (!glfwWindowShouldClose(window)) {
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> dtChrono = currentTime - prevTime;
    prevTime = currentTime;
    float dt = dtChrono.count();
    float sim_dt = dt;
    // std::cout << "delta time = " << sim_dt << ", FPS = " << 1.0 / dt << "\n";

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    processInput(window, &simulating);

    if (lmb_pressed) {
      glfwGetCursorPos(window, &curpos.x, &curpos.y);
      curpos.toWorldCoordinates();
      simulation.placeFluid(curpos.x, curpos.y, 10);
      // simulation.placeSolid(curpos.x, curpos.y, 10);
    }
    if (r_pressed) {
      didStep = true;
    }

    if (didStep) {
      printf("stepping\n");
      simulating = true;
    } else {
      // simulating = false;
    }

    if (simulating) {
      simulation.simulate(sim_dt, numIters);
    }

    renderer.updateFluidTexture();
    renderer.updateSolidTexture();

    shader->use();
    shader->setMat4("proj", proj);
    shader->setInt("smokeMap", 0);
    shader->setInt("solidMap", 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderer.getFluidTexture());
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, renderer.getSolidTexture());

    renderer.draw();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  shader.reset();

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
