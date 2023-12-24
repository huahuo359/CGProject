//
// Created by sirui lai on 2023/11/24.
//

#ifndef MAIN_PARTICLESHADER_H
#define MAIN_PARTICLESHADER_H
#include "../entity/Camera.h"

#include "../shaders/ShaderProgram.h"
#include "../particles/particle.h"

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glad/glad.h>

const std::string PARTICLE_VERTEX_SHADER = "shaders/particle.vert";
const std::string PARTICLE_FRAGMENT_SHADER = "shaders/particle.frag";

class ParticleShader : public ShaderProgram {
private:
    GLuint location_projection;
    GLuint location_model;
    GLuint location_view;

public:
    ParticleShader();

    void bindUniformLocations();
    void loadCamera(const glm::mat4& view);
    void loadParticle(const Particle *particle, const glm::mat4& viewmtx);
    void loadProjection(const glm::mat4& proj);
};

#endif //MAIN_PARTICLESHADER_H
