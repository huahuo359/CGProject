//
// Created by sirui lai on 2023/11/24.
//

#ifndef MAIN_PARTICLESYSTEM_H
#define MAIN_PARTICLESYSTEM_H
#include "../renders/ParticleManager.h"

#include <glm/glm.hpp>
#include <glad/glad.h>

class ParticleSystem {
private:
    float particlesPerSecond;
    float particleSpeed;
    float gravityFactor;
    float lifeDuration;
    GLuint textureid;
    float renderQueue;

public:
    ParticleSystem(
            float particlesPerSecond, float particleSpeed, float gravityFactor, float lifeDuration, GLuint textureid);

    void generateParticles(glm::vec3 emissionPoint, Terrain& terrin);
};


#endif //MAIN_PARTICLESYSTEM_H
