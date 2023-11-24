//
// Created by sirui lai on 2023/11/24.
//

#ifndef MAIN_PARTICLE_H
#define MAIN_PARTICLE_H
#include "../entity/Entity.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

class Particle : public Entity {
protected:
    glm::vec3 velocity;
    float gravityEffect;
    float lifeDuration;
    float elapsedTime;
    GLuint texid;

public:
    Particle(const glm::vec3& position, const glm::vec3& velocity, float gravityEffect, float lifeDuration, GLuint texture);
    bool update() override;
    GLuint getTextureID() const;
};

#endif //MAIN_PARTICLE_H
