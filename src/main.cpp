#include <glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <Shader.hpp>
#include <iostream>
#include <vector>

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

  Shader shader("shaders/vertexShader.glsl", "shaders/fragmentShader.glsl");

  buildCircle(0.1, 32);

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


  glViewport(0, 0, 800, 600);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    shader.use();

    glBindVertexArray(vertexArray);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
