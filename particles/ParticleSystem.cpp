//
// Created by sirui lai on 2023/11/24.
//
#include "particleSystem.h"

#include "particle.h"
#include "../renders/ParticleManager.h"
#include "../entity/GameTime.h"

ParticleSystem::ParticleSystem(float particlesPerSecond, float particleSpeed,
                               float gravityFactor, float lifeDuration, GLuint textureid) {
    this->particlesPerSecond = particlesPerSecond;
    this->particleSpeed = particleSpeed;
    this->gravityFactor = gravityFactor;
    this->lifeDuration = lifeDuration;
    this->textureid = textureid;
    this->renderQueue = 0.0f;
}

void ParticleSystem::generateParticles(glm::vec3 emissionPoint, GLfloat scalefactor) {
    renderQueue += particlesPerSecond * GameTime::getGameTime()->getDt();
    while (renderQueue > 1.0f) {
        glm::vec3 velo((rand() % 100) / 50.f - 1.0f, 1.0f, (rand() % 100) / 50.f - 1.0f);
        velo = glm::normalize(velo);
        velo *= particleSpeed;
        auto* p = new Particle(emissionPoint, velo, gravityFactor, lifeDuration, textureid);
        float scaleVal = (rand() % 100) / scalefactor;
        p->setScale(glm::vec3(scaleVal, scaleVal, scaleVal));

        ParticleManager::getParticleManager()->addParticle(p);
        renderQueue -= 1.0f;
    }
}