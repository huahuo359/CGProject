#include "RenderManager.h"

#include <glad/glad.h>
#include "../particles/particle.h"
#include "ParticleManager.h"
#include "../particles/ParticleSystem.h"

void RenderManager::render(const std::vector<Entity*>& entities, const std::vector<Light*>& lights, Terrain* terrain,
                            SkyboxRenderer& skybox, ShadowMap& shadowMap, MY_Camera* cam, const glm::mat4& projection,
    int winX, int winY) {
    // SHADOW PASS
    glDisable(GL_CLIP_DISTANCE0);
    shadowMap.bind();
    renderer.render(entities, lights, shadowMap.getView(), shadowMap.getProjection(), skybox.getSkyboxTexture());
    shadowMap.unbind();

    // NORMAL PASS
    glDisable(GL_CLIP_DISTANCE0);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, winX, winY);

    terrainRenderer.render(terrain, lights, cam->getViewMtx(), projection, shadowMap.getView(),
        shadowMap.getProjection(), shadowMap.getTextureID());
    skybox.render(cam->getViewMtx(), projection);
    renderer.render(entities, lights, cam->getViewMtx(), projection, skybox.getSkyboxTexture(), shadowMap.getView(),
        shadowMap.getProjection(), shadowMap.getTextureID());
    ParticleManager::getParticleManager()->render(cam->getViewMtx(), projection);
}
