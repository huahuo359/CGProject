//
// Created by sirui lai on 2023/11/24.
//

#ifndef MAIN_PARTICLEMANAGER_H
#define MAIN_PARTICLEMANAGER_H
#include "../particles/particle.h"
#include "particleRenderer.h"
#include <glm/glm.hpp>
#include <vector>

class ParticleManager {
private:
    static ParticleManager* particleManager;
    ParticleManager() = default;

    ParticleRenderer renderer;
    std::vector<Particle*> particles;

public:
    static ParticleManager* getParticleManager();

    // Should be called once per frame
    void update(Terrain* terrain);

    void addParticle(Particle*);
    void render(const glm::mat4& view, const glm::mat4& proj);
};


#endif //MAIN_PARTICLEMANAGER_H
