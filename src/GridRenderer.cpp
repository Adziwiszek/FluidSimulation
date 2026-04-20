#include "GridRenderer.hpp"
#include "Common.hpp"

#include <print>
using std::printf;

GridRenderer::GridRenderer(const FluidGrid& grid) : grid{grid} {
  glGenVertexArrays(1, &vertexArray);
  glGenBuffers(1, &vertexBuffer);
  glGenBuffers(1, &elementBuffer);
  glGenBuffers(1, &uvBuffer);

  glGenTextures(1, &fluidTexture);
  glBindTexture(GL_TEXTURE_2D, fluidTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glGenTextures(1, &solidTexture);
  glBindTexture(GL_TEXTURE_2D, solidTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

}

unsigned int GridRenderer::getFluidTexture() const {
  return fluidTexture;
}

unsigned int GridRenderer::getSolidTexture() const {
  return solidTexture;
}
 
void GridRenderer::buildGrid() {
  indices.clear();
  vertices.clear();

  for(unsigned int j = 0; j <= N_REAL[1]; j++) {
    for(unsigned int i = 0; i <= N_REAL[0]; i++) {
      float x = Origin[0] + i * D[0];
      float y = Origin[1] + j * D[1];
      vertices.push_back({x, y, 0});
    }
  }
  for(unsigned int j = 0; j < N_REAL[1]; j++) {
    for(unsigned int i = 0; i < N_REAL[0]; i++) {
      unsigned int base = j * (N_REAL[0] + 1) + i;
      indices.push_back(base);
      indices.push_back(base + 1);
      indices.push_back(base + (N_REAL[0] + 1) + 1);

      indices.push_back(base);
      indices.push_back(base + (N_REAL[0] + 1));
      indices.push_back(base + (N_REAL[0] + 1) + 1);
    }
  }

  std::vector<glm::vec2> uvs;
  for (unsigned int j = 0; j <= N_REAL[1]; j++) {
    for (unsigned int i = 0; i <= N_REAL[0]; i++) {
      float u = (static_cast<float>(i)) / N_REAL[0];
      float v = (static_cast<float>(j)) / N_REAL[1];
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

void GridRenderer::updateFluidTexture() {
    glBindTexture(GL_TEXTURE_2D, fluidTexture);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, N_REAL[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, N_REAL[0], N_REAL[1], 0,
                 GL_RED, GL_FLOAT, &grid.smoke[1][1]);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}

void GridRenderer::updateSolidTexture() {
    std::vector<float> solidData(N_REAL[0] * N_REAL[1]);
    for (unsigned int j = 0; j < N_REAL[1]; j++) {
      for (unsigned int i = 0; i < N_REAL[0]; i++) {
        solidData[j * N_REAL[0] + i] =
            static_cast<float>(grid.solid[j][i]);
      }
    }

    glBindTexture(GL_TEXTURE_2D, solidTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, N_REAL[0], N_REAL[1], 0,
                 GL_RED, GL_FLOAT, solidData.data());
}

void GridRenderer::draw() {
  glBindVertexArray(vertexArray);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}
