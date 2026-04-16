#include "GridRenderer.hpp"

GridRenderer::GridRenderer(const FluidGrid& grid) : grid{grid} {
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

unsigned int GridRenderer::getSmokeTexture() const {
  return smokeTexture;
}

unsigned int GridRenderer::getSolidTexture() const {
  return smokeTexture;
}
 
void GridRenderer::buildGrid() {
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

void GridRenderer::updateTexture() {
  std::vector<float> smokeData(N[0] * N[1]);
  for (unsigned int j = 0; j < N[1]; j++)
      for (unsigned int i = 0; i < N[0]; i++)
          smokeData[j * N[0] + i] = static_cast<float>(grid.smoke[j][i]);

  glBindTexture(GL_TEXTURE_2D, smokeTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, N[0], N[1], 0,
               GL_RED, GL_FLOAT, smokeData.data());
}

void GridRenderer::draw() {
  glBindVertexArray(vertexArray);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}
