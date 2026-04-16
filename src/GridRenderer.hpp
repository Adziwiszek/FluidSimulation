#pragma once

#include <glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <vector>

#include <FluidGrid.hpp>

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
  GridRenderer(const FluidGrid& grid);

  unsigned int getSmokeTexture() const;
  unsigned int getSolidTexture() const;
   
  void buildGrid(); 
  void updateTexture(); 
  void draw(); 
};
