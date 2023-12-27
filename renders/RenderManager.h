#pragma once

#include "EntityRenderer.h"
#include "TerrainRenderer.h"
#include "SkyboxRenderer.h"
#include "../entity/Entity.h"
#include "../entity/Light.h"
#include "../entity/Camera.h"
#include "../entity/Terrain.h"
#include "FrameBuffer.h"
#include "ShadowMap.h"

#include <vector>
#include <glm/glm.hpp>

class RenderManager {
  private:
    EntityRenderer renderer;
    TerrainRenderer terrainRenderer;

  public:
    RenderManager() = default;
    
    void render(const std::vector<Entity*>& entities, const std::vector<Light*>& lights, Terrain* terrain,
        SkyboxRenderer& skybox, ShadowMap& shadowMap, MY_Camera* cam, const glm::mat4& projection,
        int winX, int winY);
};
