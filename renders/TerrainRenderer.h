#pragma once

#include "../shaders/TerrainShader.h"
#include "../entity/Light.h"
#include "../entity/Camera.h"
#include "../entity/MY_Model.h"

#include <vector>

#include <glm/glm.hpp>
#include <glad/glad.h>

class TerrainRenderer {
  private:
    TerrainShader shader;

  public:
    TerrainRenderer() = default;

    void render(const Terrain* terrain, const std::vector<Light*>& lights, const glm::mat4& view, const glm::mat4& proj,
        const glm::vec4& clipPlane);
    void render(const Terrain* terrain, const std::vector<Light*>& lights, const glm::mat4& view, const glm::mat4& proj,
        const glm::mat4& depthView, const glm::mat4& depthProj, GLuint shadowMap, const glm::vec4& clipPlane);
};
