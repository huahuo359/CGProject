//
// Created by sirui lai on 2023/11/24.
//
#include "particleSystem.h"

#include "particle.h"
#include "../renders/ParticleManager.h"
#include "../entity/GameTime.h"
#include "../entity/Terrain.h"

ParticleSystem::ParticleSystem(float particlesPerSecond, float particleSpeed,
                               float gravityFactor, float lifeDuration, GLuint textureid) {
    this->particlesPerSecond = particlesPerSecond;
    this->particleSpeed = particleSpeed;
    this->gravityFactor = gravityFactor;
    this->lifeDuration = lifeDuration;
    this->textureid = textureid;
    this->renderQueue = 0.0f;
}

void ParticleSystem::generateParticles(glm::vec3 emissionPoint, float sz, float vel_y) {
    renderQueue += particlesPerSecond * GameTime::getGameTime()->getDt();
    while (renderQueue > 1.0f) {
        glm::vec3 velo((rand() % 100) / 50.f - 1.0f, vel_y, (rand() % 100) / 50.f - 1.0f);
        velo = glm::normalize(velo);
        velo *= particleSpeed;
//        glm::vec3 position((rand()%200 - 100)*1.0f + emissionPoint.x, (rand()%200 - 100)*1.0f+emissionPoint.y, (-1*rand()%50)*1.0f+emissionPoint.z);
        auto* p = new Particle(emissionPoint, velo, gravityFactor, lifeDuration, textureid);
        float scaleVal = (rand() % 100);
        p->setScale(glm::vec3(sz,sz,sz));

        ParticleManager::getParticleManager()->addParticle(p);
        renderQueue -= 1.0f;
    }
}