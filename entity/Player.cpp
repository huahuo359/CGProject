#include "Player.h"

#include "constants.h"
#include "GameTime.h"
#include "Terrain.h"
#include "../tools/camera.h"

#include <algorithm>
#include <cmath>

#include <glm/glm.hpp>

using namespace std;
using namespace glm;

// Static variables used for car physics
constexpr float gravity = 9.81f;        // m/s^2
constexpr float mass = 2000.0f;         // kg
constexpr float inertiaScale = 1.0f;    // Multiply by mass for inertia

constexpr float cgToFrontAxle = 1.25f;  // Centre gravity to front axle
constexpr float cgToRearAxle = 1.25f;   // Centre gravity to rear axle
constexpr float cgHeight = 0.45f;       // Centre gravity height

constexpr float tireGrip = 3.0f;        // How much grip tires have
constexpr float lockGrip = 0.8f;        // % of grip available when wheel is locked
constexpr float engineForce = 8000.0f;
constexpr float brakeForce = 12000.0f;
constexpr float eBrakeForce = brakeForce / 2.5f;
constexpr float weightTransfer = 0.2f;  // How much weight is transferred during acceleration/braking

constexpr float cornerStiffnessFront = 5.0f;
constexpr float cornerStiffnessRear = 5.2f;
constexpr float airResist = 3.0f;  // air resistance (* vel)
constexpr float rollResist = 5.0f;
constexpr float inertia = mass * inertiaScale;                    // will be = mass
constexpr float wheelBase = cgToFrontAxle + cgToRearAxle;         // set from axle to CG lengths
constexpr float axleWeightRatioFront = cgToRearAxle / wheelBase;  // % car weight on the front axle
constexpr float axleWeightRatioRear = cgToFrontAxle / wheelBase;  // % car weight on the rear axle

extern Camera camera;
extern bool is_fired;
const unsigned int SCR_WIDTH=1600;
const unsigned int SCR_HEIGHT=1200;

Player::Player(MY_Model* model, Terrain* terrain, bool basic_controls) : Entity(model) {
    this->terrain = terrain;
    this->absVel = 0.0f;
    this->yawRate = 0.0f;
    this->steerAngle = 0.0f;
    this->steerChange = 0.0f;
    this->throttle_input = 0.0f;
    this->brake_input = 0.0f;
    this->ebrake_input = 0.0f;
    this->basic_controls = basic_controls;
    this->velocity = {0.f, 0.f};
    this->velocity_c = {0.f, 0.f};
    this->accel = {0.f, 0.f};
    this->accel_c = {0.f, 0.f};

    this->is_player = true;
    if (basic_controls) {
        ROTATION_SPEED = constants::PI;
    } else {
        ROTATION_SPEED = 0.3f;
    }
}

template <typename T>
int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

float Player::getThrottle() const {
    return throttle_input;
}

float Player::getBrake() const {
    return brake_input;
}

float Player::getSteer() const {
    return steerAngle;
}

bool Player::update(std::vector<Entity*> entities) {
    steerAngle = smoothSteering(steerChange);
    float dt = GameTime::getGameTime()->getDt();
    float dx = 0.0f;
    float dz = 0.0f;

    if (basic_controls) {
        rotateY(steerAngle * dt);

        if (m_y_rot > (float)constants::PI * 2) {
            m_y_rot -= constants::PI * 2;
        } else if (m_y_rot < (float)-constants::PI * 2) {
            m_y_rot += constants::PI * 2;
        }

        float distance = (throttle_input - brake_input) * MOVE_SPEED * dt;

        dx = distance * glm::sin(m_y_rot);
        dz = distance * glm::cos(m_y_rot);
    } else {
        /* Following is code from #1 and adapted to this program */
        // Pre-calc heading vector
        float sn = sin(m_y_rot);
        float cs = cos(m_y_rot);

        // Get velocity in local car coordinates
        velocity_c.y = cs * velocity.y + sn * velocity.x;
        velocity_c.x = cs * velocity.x - sn * velocity.y;

        // Weight on axles based on centre of gravity and weight shift due to forward/reverse acceleration
        float axleWeightFront =
            mass * (axleWeightRatioFront * gravity - weightTransfer * accel_c.y * cgHeight / wheelBase);
        float axleWeightRear =
            mass * (axleWeightRatioRear * gravity + weightTransfer * accel_c.y * cgHeight / wheelBase);

        // Resulting velocity of the wheels as result of the yaw rate of the car body.
        // v = yawrate * r where r is distance from axle to CG and yawRate (angular velocity) in rad/s.
        float yawSpeedFront = cgToFrontAxle * yawRate;
        float yawSpeedRear = -cgToRearAxle * yawRate;

        // Calculate slip angles for front and rear wheels (a.k.a. alpha)
        float slipAngleFront = atan2(velocity_c.x + yawSpeedFront, abs(velocity_c.y)) - sgn(velocity_c.y) * steerAngle;
        float slipAngleRear = atan2(velocity_c.x + yawSpeedRear, abs(velocity_c.y));

        float tireGripFront = tireGrip;
        float tireGripRear = tireGrip * (1.f - ebrake_input * (1.f - lockGrip));  // reduce rear grip when ebrake is on

        float frictionForceFront_cy =
            std::clamp(-cornerStiffnessFront * slipAngleFront, -tireGripFront, tireGripFront) * axleWeightFront;
        float frictionForceRear_cy =
            std::clamp(-cornerStiffnessRear * slipAngleRear, -tireGripRear, tireGripRear) * axleWeightRear;

        //  Get amount of brake/throttle from our inputs
        float brake = std::min(brake_input * brakeForce + ebrake_input * eBrakeForce, brakeForce);
        float throttle = throttle_input * engineForce;

        //  Resulting force in local car coordinates.
        //  is implemented as a RWD car only.
        float tractionForce_cx = throttle - brake * sgn(velocity_c.y);
        float tractionForce_cy = 0;

        float dragForce_cx = -rollResist * velocity_c.y - airResist * velocity_c.y * abs(velocity_c.y);
        float dragForce_cy = -rollResist * velocity_c.x - airResist * velocity_c.x * abs(velocity_c.x);

        // total force in car coordinates
        float totalForce_cx = dragForce_cx + tractionForce_cx;
        float totalForce_cy =
            dragForce_cy + tractionForce_cy + cos(steerAngle) * frictionForceFront_cy + frictionForceRear_cy;

        // acceleration along car axes
        accel_c.y = totalForce_cx / mass;  // forward/reverse accel
        accel_c.x = totalForce_cy / mass;  // sideways accel

        // acceleration in world coordinates
        accel.y = cs * accel_c.y - sn * accel_c.x;
        accel.x = sn * accel_c.y + cs * accel_c.x;

        // update velocity
        velocity.y += accel.y * dt;
        velocity.x += accel.x * dt;

        absVel = length(velocity);

        // Slow the car down when no throttle, the overall equation doesn't seem to do this very well.
        if (throttle < 0.5f) {
            velocity = velocity - (velocity * 0.5f * dt);
        }

        // calculate rotational forces
        float angularTorque =
            (frictionForceFront_cy + tractionForce_cy) * cgToFrontAxle - frictionForceRear_cy * cgToRearAxle;

        float angularAccel = angularTorque / inertia;

        //  Sim gets unstable at very slow speeds, so just stop the car
        if (abs(absVel) < 2.0f && throttle < 0.5f) {
            velocity.y = 0.0f;
            velocity.x = 0.0f;
            absVel = 0.0f;
            yawRate = 0.0f;
        } else {
            yawRate += angularAccel * dt;
            m_y_rot += yawRate * dt;

            dx = velocity.x * dt;
            dz = velocity.y * dt;
        }
        /* End code from #1 */
    }

    // Wrap rotation around once it reaches 2*pi
    if (m_y_rot > (float)constants::PI * 2) {
        m_y_rot -= constants::PI * 2;
    } else if (m_y_rot < (float)-constants::PI * 2) {
        m_y_rot += constants::PI * 2;
    }

    // Assumes constant scale
    float player_length = (abs(m_model->getRangeInDim(2).second - m_model->getRangeInDim(2).first)) / 2.0f * m_scale.x;
    float player_length_x = player_length * glm::sin(m_y_rot);
    float player_length_z = player_length * glm::cos(m_y_rot);


    glm::vec4 player_ver[8];
    for(int i=0; i<8; ++i) {
        player_ver[i] = this->getVertices(i);
    }

    bool _isCollide=false;
    for(auto it : entities) {
        if(!it->getIsPlayer()) {
            glm::vec4 temp_vertice[8];
            for(int i = 0; i<8; ++i) {
                temp_vertice[i] = it->getVertices(i);
            }
            _isCollide = ImpactChecker1(this->getPosition()+glm::vec3(player_length_x+dx,0.0f,player_length_z+dz), this->getScale().x/10, temp_vertice);
//            temp_vertice[0] = glm::vec4 (it->getPosition()+it->getScale()*glm::vec3 (it->getBoundSize(),it->getBoundSize(),it->getBoundSize()), 1.0f);
//            temp_vertice[1] = glm::vec4 (it->getPosition()+it->getScale()*glm::vec3 (-it->getBoundSize(),it->getBoundSize(),-it->getBoundSize()), 1.0f);
//            temp_vertice[2] = glm::vec4 (it->getPosition()+it->getScale()*glm::vec3 (-it->getBoundSize(),it->getBoundSize(),it->getBoundSize()), 1.0f);
//            temp_vertice[3] = glm::vec4 (it->getPosition()+it->getScale()*glm::vec3 (it->getBoundSize(),it->getBoundSize(),-it->getBoundSize()), 1.0f);

//            for(int i=0; i<8; ++i) {
//                if(VertexInRange(temp_vertice, player_ver[i]+glm::vec4(player_length_x+dx,0.0f,player_length_z+dz,1.0f))) {
//                    _isCollide = true;
//                    break;
//                }
//            }
        }

        if(_isCollide) {
            this->setCollide(true);
            break;
        }
    }
    if(!_isCollide)
        this->setCollide(false);

    if (terrain->isOnTerrain(m_position.x + dx + player_length_x, m_position.z + dz + player_length_z) && !isCollide) {
        move(glm::vec3(dx, 0, dz));
        placeBottomEdge(terrain->getHeight(getPosition().x, getPosition().z));
        setRotationX(terrain->getAngleX(getPosition().x, getPosition().z, getRotationY()));
        setRotationZ(terrain->getAngleZ(getPosition().x, getPosition().z, getRotationY()));
        updateBound();
        return true;
    }
    return false;
}

float Player::smoothSteering(float inputAngle) {
    float smoothedAngle = 0;
    float dt = GameTime::getGameTime()->getDt();
    float CHANGE_MODIFIER = 12.0f;

    if (abs(inputAngle) > 0.001) {
        smoothedAngle =
            std::clamp((float)(steerAngle + inputAngle * dt * CHANGE_MODIFIER), -ROTATION_SPEED, ROTATION_SPEED);
    } else {
        //  No steer input - move toward centre (0)
        if (steerAngle > 0) {
            smoothedAngle = std::max(steerAngle - dt * CHANGE_MODIFIER, 0.0f);
        } else if (steerAngle < 0) {
            smoothedAngle = std::min(steerAngle + dt * CHANGE_MODIFIER, 0.0f);
        }
    }

    return smoothedAngle;
}

void Player::handleKeyboardEvents(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mods*/) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_W || key == GLFW_KEY_UP) {
            throttle_input = 1.0f;
        }
        if (key == GLFW_KEY_S || key == GLFW_KEY_DOWN) {
            brake_input = 1.0f;
        }
        if (key == GLFW_KEY_SPACE) {
            ebrake_input = 1.0f;
        }
        if (key == GLFW_KEY_A || key == GLFW_KEY_LEFT) {
            steerChange = ROTATION_SPEED;
        } else if (key == GLFW_KEY_D || key == GLFW_KEY_RIGHT) {
            steerChange = -ROTATION_SPEED;
        }
    }

    if (action == GLFW_RELEASE) {
        if (key == GLFW_KEY_W || key == GLFW_KEY_UP) {
            throttle_input = 0.0f;
        }
        if (key == GLFW_KEY_S || key == GLFW_KEY_DOWN) {
            brake_input = 0.0f;
        }
        if (key == GLFW_KEY_SPACE) {
            ebrake_input = 0.0f;
        }

        if ((key == GLFW_KEY_A || key == GLFW_KEY_LEFT) && steerAngle > 0.0f) {
            steerChange = 0.0f;
        }

        if ((key == GLFW_KEY_D || key == GLFW_KEY_RIGHT) && steerAngle < 0.0f) {
            steerChange = 0.0f;
        }

        if (key == GLFW_KEY_F) {
            is_fired = true;
        }
    }
}

bool Player::ImpactChecker1(glm::vec3 coord, GLfloat r, glm::vec4 vertices[8]) {


    glm::vec3 minCoord = glm::vec3(vertices[0]);
    glm::vec3 maxCoord = glm::vec3(vertices[0]);

    for (int i = 1; i < 8; ++i) {
        for (int j = 0; j < 3; ++j) {
            minCoord[j] = std::min(minCoord[j], vertices[i][j]);
            maxCoord[j] = std::max(maxCoord[j], vertices[i][j]);
        }
    }

    // 找到 AABB 中与球心最近的一个点
    GLfloat x_near = coord.x;
    x_near = x_near > maxCoord.x ? maxCoord.x : x_near;
    x_near = x_near < minCoord.x ? minCoord.x : x_near;

    GLfloat y_near = coord.y;
    y_near = y_near > maxCoord.y ? maxCoord.y : y_near;
    y_near = y_near < minCoord.y ? minCoord.y : y_near;

    GLfloat z_near = coord.z;
    z_near = z_near > maxCoord.z ? maxCoord.z : z_near;
    z_near = z_near < minCoord.z ? minCoord.z : z_near;

    GLfloat distance = (coord.x-x_near) * (coord.x-x_near) +
                       (coord.y-y_near) * (coord.y-y_near) +
                       (coord.z-z_near) * (coord.z-z_near);

    if(distance <= r*r ) {
        //std::cout << "bump dis: " << distance << std::endl;
        return true;
    } else {
        //std::cout << "normal dis: " << distance << std::endl;
        return false;
    }

}
