#pragma once

#define _USE_MATH_DEFINES

#include "../shaders/EntityShader.h"
#include "../entity/Light.h"
#include "../entity/Camera.h"
#include "../entity/MY_Model.h"

#include <cstdio>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glad/glad.h>

class EntityRenderer {
  private:
    EntityShader m_shader;

  public:
    void render(const std::vector<Entity*>& entities, const std::vector<Light*>& lights, const glm::mat4& view,
        const glm::mat4& proj, GLuint reflectionTexture, const glm::vec4& clipPlane);
    void render(const std::vector<Entity*>& entities, const std::vector<Light*>& lights, const glm::mat4& view,
        const glm::mat4& proj, GLuint reflectionTexture, const glm::mat4& depthView, const glm::mat4& depthProj,
        GLuint shadowMap, const glm::vec4& clipPlane);
    void renderModel(const MY_Model* model);
};
