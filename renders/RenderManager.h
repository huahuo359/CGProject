#pragma once

#include "EntityRenderer.h"
#include "TerrainRenderer.h"
#include "SkyboxRenderer.h"
#include "WaterRenderer.h"
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
    FrameBuffer reflectionBuffer;
    FrameBuffer refractionBuffer;

    EntityRenderer renderer;
    TerrainRenderer terrainRenderer;
    WaterRenderer waterRenderer;

  public:
    RenderManager();

    // Renders the entire scene, including 4 passes (shadow, refraction, reflection, total)
    // Lots of parameters, some of these could be maybe moved to the constructor as they do not change.
    // Not sure what a better way to manage all of these parameters is
    void render(const std::vector<Entity*>& entities, const std::vector<Light*>& lights, Terrain* terrain,
        Entity* water, SkyboxRenderer& skybox, ShadowMap& shadowMap, MY_Camera* cam, const glm::mat4& projection,
        int winX, int winY);
};
