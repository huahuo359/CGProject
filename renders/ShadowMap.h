#pragma once

#include "../entity/Light.h"
#include "../entity/Player.h"
#include "FrameBuffer.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

class ShadowMap : public FrameBuffer {
  private:
    Player* player;

    const GLuint textureSize;

    glm::vec3 lightDir;
    glm::mat4 projection;
    glm::mat4 view;

  public:
    ShadowMap(Player* player, Light* light, GLuint textureSize = 1024);

    inline GLuint getTextureID() { return getDepthTexture(); };
    inline GLuint getTextureSize() { return textureSize; };
    inline glm::mat4 getView() { return view; };
    inline glm::mat4 getProjection() const { return projection; };
    
    void bind() override;
};
