#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "tools/shader.h"
#include "tools/camera.h"
#include "tools/model.h"
#include "entity/Terrain.h"
#include "entity/Player.h"
#include "entity/constants.h"
#include "entity/GameTime.h"
#include "entity/Camera.h"
#include "renders/RenderManager.h"
#include "Window.h"
#include "particles/particleSystem.h"
#include "renders/ParticleManager.h"


#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glm/ext.hpp"
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
bool IsCollide(vector<Entity*> entities);
bool VertexInRange(glm::vec4 entityRange[], glm::vec4 player_vertice);

// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1200;

// camera
MY_Camera* camera;

void mainLoop() {
    InputState input;
    glm::mat4 projection;
    vector<Entity*> entities;
    vector<Light*> lights;
    Window new_window;

    MY_Model player_mod = Loader::getLoader()->loadModel("res/car/Hilux.obj");

    std::vector<std::string> terrain_img;
    terrain_img.push_back("image/terrin/blendMap.png");
    terrain_img.push_back("image/terrin/dirt.png");
    terrain_img.push_back("image/terrin/grass.jpg");
    terrain_img.push_back("image/terrin/mud.jpg");
    terrain_img.push_back("image/terrin/road.jpg");

    Terrain* terrain = Terrain::loadTerrain(terrain_img, "image/terrin/heightmap.png");
    terrain->setPosition(glm::vec3(-Terrain::TERRAIN_SIZE/2, 0.0f, -Terrain::TERRAIN_SIZE/2));

    Player* player = new Player(&player_mod, terrain, 1);
    player->setScale(glm::vec3(1.0f, 1.0f, 1.0f));

    player->setPosition(terrain->getPositionFromPixel(555,751));
    player->setRotationY((GLfloat)5.0f * constants::PI / 8.0f);
    entities.push_back(player);

    MY_Model engine = Loader::getLoader()->loadModel("res/engine/Engine.obj");
    Entity* engine_entity = new Entity(&engine);
    engine_entity->setScale(glm::vec3(10.0f,10.0f,10.0f));
    engine_entity->setPosition(terrain->getPositionFromPixel(600,500));
    engine_entity->setBoundSize(0.8f);
    engine_entity->rotateX(constants::PI/2);
    entities.push_back(engine_entity);

//    MY_Model relic = Loader::getLoader()->loadModel("res/relic/Building01.obj");
//    Entity* relic_entity = new Entity(&relic);
//    relic_entity->setScale(glm::vec3(5.0f,5.0f,5.0f));
//    relic_entity->setPosition(terrain->getPositionFromPixel(800,400));
//    relic_entity->setBoundSize(1.0f);
//    entities.push_back(relic_entity);

    MY_Model highway = Loader::getLoader()->loadModel("res/highway/TheBridge.obj");
    Entity* highway_entity = new Entity(&highway);
    highway_entity->setScale(glm::vec3(10.0f,10.0f,10.0f));
    highway_entity->setPosition(terrain->getPositionFromPixel(800,400));
    highway_entity->setBoundSize(1.0f);
    entities.push_back(highway_entity);

    new_window.set_key_callback([&](GLFWwindow* window, int key, int scancode, int action, int mods) {
        // Terminate program if escape is pressed
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        player->handleKeyboardEvents(window, key, scancode, action, mods);
    });

    new_window.set_mouse_position_callback([&](GLFWwindow* /*window*/, double x, double y) {
        input.update((float)x, (float)y);
    });

    new_window.set_mouse_scroll_callback([&](GLFWwindow* /*window*/, double xoffset, double yoffset) {
        input.updateScroll((float)xoffset, (float)yoffset);
    });

    new_window.set_mouse_button_callback([&](GLFWwindow* /*window*/, int button, int action, int /*mods*/) {
        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
            input.rMousePressed = true;
        } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
            input.rMousePressed = false;
        } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            input.lMousePressed = true;
        } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
            input.lMousePressed = false;
        }
    });

    new_window.set_window_reshape_callback([&](GLFWwindow* /*window*/, int x, int y) {
        projection = glm::perspective(constants::PI/4.0, double (x)/double (y), 1.0, 800.0);
        glViewport(0, 0, x, y);
    });

    srand(static_cast<unsigned int>(time(nullptr)));

    projection = glm::perspective(constants::PI/4.0, double (new_window.get_width())/double (new_window.get_height()), 1.0, 800.0);
    camera = new PlayerCamera(player);
   
    bool flag = true;

    std::vector<std::string> skyboxTextures = {
            "image/skybox/back.jpg",
            "image/skybox/bottom.jpg",
            "image/skybox/front.jpg",
            "image/skybox/left.jpg",
            "image/skybox/right.jpg",
            "image/skybox/top.jpg"
    };
    SkyboxRenderer skybox = SkyboxRenderer(skyboxTextures, 200.0f);

    // Create light sources
    auto* sunny = new Light();
    sunny->position = glm::vec4(-1.25 * 200.0f / 10, 2.5 * 200.0f / 10, 3 * 200.0f / 10, 0.0f);
    sunny->specular = glm::vec3(1.0f, 1.0f, 1.0f);
    sunny->diffuse = glm::vec3(0.7f, 0.7f, 0.7f);
    sunny->ambient = glm::vec3(0.1f, 0.1f, 0.1f);
    lights.push_back(sunny);

    auto* headlight = new Light();
    headlight->position = glm::vec4(4.0f, 8.0f, 0.0f, 1.0f);
    headlight->specular = glm::vec3(0.8f, 0.8f, 0.4f);
    headlight->diffuse = glm::vec3(0.8f, 0.8f, 0.4f);
    headlight->coneDirection = glm::vec3(0.0f, -1.0f, 0.0f);
    headlight->coneAngle = constants::PI / 4.f;
    headlight->radius = 10.0f;
    lights.push_back(headlight);

    auto* headlight2 = new Light();
    headlight2->position = glm::vec4 (0.0f,8.0f,0.0f,1.0f);
    headlight2->specular = glm::vec3 (0.8f,0.8f,0.4f);
    headlight2->diffuse = glm::vec3(0.8f, 0.8f, 0.4f);
    headlight2->coneDirection = glm::vec3(0.0f, -1.0f, 0.0f);
    headlight2->coneAngle = constants::PI / 4.f;
    headlight2->radius = 10.0f;
    lights.push_back(headlight2);

    for(auto it : entities) {
        it->placeBottomEdge(terrain->getHeight(it->getPosition().x, it->getPosition().z));
    }

    // Create the large lake
    auto* water = new Entity();
    water->setScale(glm::vec3(100.0f, 1.0f, 50.0f));
    water->setPosition(terrain->getPositionFromPixel(650, 826));
    water->setPosition(glm::vec3(water->getPosition().x, 0.4f, water->getPosition().z));

    ShadowMap shadowmap(player, lights[0], 4096);
    RenderManager manager;

    GLuint dust_texture = Loader::getLoader()->loadTexture("image/dust_single.png");
    ParticleSystem particleSystem(30.0f, 3.0f, 0.2f, 0.5f, dust_texture);

    GLuint snow_texture = Loader::getLoader()->loadTexture("image/snow4.png");
    ParticleSystem snowSystem(150.0f, 0.2f, 0.02f, 10.0f, snow_texture);

    while (!glfwWindowShouldClose(new_window.get_window()))
    {
        GameTime::getGameTime()->update();
        camera->update(input);
        manager.render(entities, lights, terrain, water, skybox, shadowmap, camera, projection, new_window.get_width(), new_window.get_height());

        if(flag) {
            // close shot
            ParticleManager::getParticleManager()->update(terrain);

            for(auto it : entities) {
                if(!it->getIsPlayer())
                    it->update();
                else {
                    Player* temp = dynamic_cast<Player*>(it);
                    temp->update(entities);
                }
            }
            headlight->position = glm::vec4(player->getPosition() + glm::vec3(0.0f, 1.0f, 0.0f), 1.0f);
            headlight2->position = glm::vec4 (player->getPosition() + glm::vec3(0.0f,1.0f,0.0f),1.0f);
            headlight->coneDirection = player->calculateDirectionVector();
            headlight2->coneDirection = player->calculateDirectionVector();

            if (player->absVel > 5.0f || player->getThrottle() > 0.1f || (1 && player->getBrake() > 0.1f)) {
                particleSystem.generateParticles(player->getPosition() - player->calculateDirectionVector(), *terrain);
            }

            snowSystem.generateParticles(player->getPosition()+glm::vec3(0.0f,20.0f,0.0f), *terrain);

        } else {
            // near shot

        }
        
        glFlush();
        glfwSwapBuffers(new_window.get_window());
        glfwPollEvents();
    }
//    delete player;
//    delete water;
    for(auto it : entities) {
        delete it;
    }
    glfwDestroyWindow(new_window.get_window());
}

int main()
{
    mainLoop();

    glfwTerminate();
    return 0;
}