#pragma once

#include "../shaders/SkyboxShader.h"

#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>

class SkyboxRenderer {
  private:
    SkyboxShader shader;
    GLuint vao;
    GLuint texture;
    size_t indexCount;

  public:
    SkyboxRenderer(const std::vector<std::string>& images, float sz);
    GLuint getSkyboxTexture() const {return texture; };
    void render(const glm::mat4& view, const glm::mat4&);
};
