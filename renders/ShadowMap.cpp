#include "ShadowMap.h"

#include <glm/ext.hpp>

ShadowMap::ShadowMap(Player* player, Light* light, GLuint textureSize)
    : FrameBuffer(textureSize, textureSize), player(player), textureSize(textureSize) {
    projection = glm::ortho<float>(-30, 30, -30, 30, -50, 150);
    lightDir = glm::vec3(light->position.x, light->position.y, light->position.z);

    addDepthTexture();
    if (!isOkay()) {
        std::cerr << "FrameBuffer failed" << std::endl;
        exit(1);
    }
}

void ShadowMap::bind() {
    FrameBuffer::bind();
    view = glm::lookAt(player->getPosition() + lightDir, player->getPosition(), glm::vec3(0, 1, 0));
}