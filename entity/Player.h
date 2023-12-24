#pragma once

#include "Entity.h"
#include "Terrain.h"
#include "MY_Model.h"
#include "../tools/shader.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Player : public Entity {
  private:
    const float MOVE_SPEED = 10.0f;
    float ROTATION_SPEED;

    glm::vec2 velocity;
    glm::vec2 velocity_c;
    glm::vec2 accel;
    glm::vec2 accel_c;

    float yawRate;

    float steerAngle;
    float steerChange;
    float throttle_input;
    float brake_input;
    float ebrake_input;

    Terrain* terrain;

    bool isCollide = false;

    // True to use basic controls, false to use physics model
    bool basic_controls;

    float smoothSteering(float);

  public:
    Player(MY_Model* model, Terrain* terrain, bool basic_controls);
    bool update(std::vector<Entity*> entities);
    float getThrottle() const;
    float getBrake() const;
    float getSteer() const;
    inline void setCollide(bool c) { isCollide = c; }
    bool VertexInRange(glm::vec4 entityRange[], glm::vec4 player_vertice) {
        if((player_vertice.x > fmin(entityRange[0].x,entityRange[1].x) && player_vertice.x < entityRange[0].x
            && player_vertice.z > entityRange[1].z && player_vertice.z < fmax(entityRange[0].z,entityRange[1].z))) {
            return true;
        } else {
            return false;
        }
    }
    float absVel;

    void handleKeyboardEvents(GLFWwindow* window, int key, int scancode, int action, int mods);
    bool ImpactChecker1(glm::vec3 coord, GLfloat r, glm::vec4 vertices[8]);
};
