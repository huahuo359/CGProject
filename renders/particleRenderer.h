//
// Created by sirui lai on 2023/11/24.
//

#ifndef MAIN_PARTICLERENDERER_H
#define MAIN_PARTICLERENDERER_H
#include "../shaders/particleShader.h"
#include "../entity/MY_Model.h"

#include <vector>
#include <glm/glm.hpp>

class ParticleRenderer {
private:
    ParticleShader shader;
    ModelComponent quad;

public:
    ParticleRenderer();

    void render(std::vector<Particle*> particles, glm::mat4 view, glm::mat4 proj);
};


#endif //MAIN_PARTICLERENDERER_H
