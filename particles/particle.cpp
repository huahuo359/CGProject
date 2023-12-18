//
// Created by sirui lai on 2023/11/24.
//
#include "particle.h"

#include "../entity/GameTime.h"

#define GRAVITY_ACCELERATION -9.81f

Particle::Particle(const glm::vec3& position, const glm::vec3& velocity, float gravityEffect, float lifeDuration, GLuint texture) {
    this->m_position = position;
    this->velocity = velocity;
    this->gravityEffect = gravityEffect;
    this->lifeDuration = lifeDuration;
    this->elapsedTime = 0.0f;
    this->texid = texture;
    this->is_hit_grd = false;
}

// Returns true if the particle is still alive after this update.
bool Particle::update(Terrain* terrain) {
    float dt = GameTime::getGameTime()->getDt();
    if(!is_hit_grd) {
        velocity.y += GRAVITY_ACCELERATION * gravityEffect * dt;
        m_position = m_position + (velocity * dt);
    }

    elapsedTime += dt;

    if(m_position.y <= terrain->getHeight(m_position.x, m_position.z)+0.1f) {
        m_position.y = terrain->getHeight(m_position.x,m_position.z)+0.1f;
        velocity = glm::vec3 (0.0f,0.0f,0.0f);
        is_hit_grd = true;
    }
    return elapsedTime < lifeDuration;
}

GLuint Particle::getTextureID() const {
    return texid;
}
