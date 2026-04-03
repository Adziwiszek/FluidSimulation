#include <glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <Shader.hpp>
#include <iostream>
#include <vector>

std::vector<glm::vec3> vertices;

unsigned int vertexBuffer;
unsigned int vertexArray;

/* source: https://faun.pub/draw-circle-in-opengl-c-2da8d9c2c103 
 * access: 03.04.2026 */
void buildCircle(float radius, int vCount) {
  float angle = 360.0f / vCount;

  int triangleCount = vCount - 2;

  std::vector<glm::vec3> temp;
  // positions
  for (int i = 0; i < vCount; i++)
  {
      float currentAngle = angle * i;
      float x = radius * cos(glm::radians(currentAngle));
      float y = radius * sin(glm::radians(currentAngle));
      float z = 0.0f;

      temp.push_back(glm::vec3(x, y, z));
  }

  for (int i = 0; i < triangleCount; i++)
  {
      vertices.push_back(temp[0]);
      vertices.push_back(temp[i + 1]);
      vertices.push_back(temp[i + 2]);
  }
}

float vertices_t[] = {
    -0.8f, -0.6f, 0.0f,
    0.3f, -0.4f, 0.0f,
    0.0f, 0.2f, 0.0f};

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

int main() {
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW\n";
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(800, 600, "Hello OpenGL", nullptr, nullptr);
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

  Shader shader("shaders/vertexShader", "shaders/fragmentShader");

  buildCircle(0.1, 32);

  glGenVertexArrays(1, &vertexArray);
  glGenBuffers(1, &vertexBuffer);

  glBindVertexArray(vertexArray);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

  glEnableVertexAttribArray(0);


  glViewport(0, 0, 800, 600);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    shader.use();

    glBindVertexArray(vertexArray);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
