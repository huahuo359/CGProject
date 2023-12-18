#include "Entity.h"

#include <glm/ext.hpp>

using namespace std;

// Constructor accepts a model defining vertex, colour and index data for this entity.
Entity::Entity(MY_Model* model) {
    this->m_model = model;

    m_position = glm::vec3(0.0f, 0.0f, 0.0f);
    m_scale = glm::vec3(1.0f, 1.0f, 1.0f);
    m_x_rot = 0.0f;
    m_y_rot = 0.0f;
    m_z_rot = 0.0f;
}

Entity::Entity() {
    this->m_model = nullptr;

    m_position = glm::vec3(0.0f, 0.0f, 0.0f);
    m_scale = glm::vec3(1.0f, 1.0f, 1.0f);
    m_x_rot = 0.0f;
    m_y_rot = 0.0f;
    m_z_rot = 0.0f;
}

bool Entity::update() {
    return false;
}

const MY_Model* Entity::getModel() const {
    return m_model;
}

glm::mat4 Entity::calculateModelMatrix() const {
    glm::mat4 rotation = calculateRotationMatrix(m_x_rot, m_y_rot, m_z_rot);
    return calculateModelMatrix(m_position, rotation, m_scale);
}

// Getters and setters for entity state values.
const glm::vec3& Entity::getPosition() const {
    return m_position;
}

const glm::vec3& Entity::getScale() const {
    return m_scale;
}

float Entity::getRotationX() const {
    return m_x_rot;
}

float Entity::getRotationY() const {
    return m_y_rot;
}

float Entity::getRotationZ() const {
    return m_z_rot;
}

glm::vec3 Entity::calculateDirectionVector() const {
    return glm::normalize(glm::vec3(glm::sin(m_y_rot), glm::sin(m_x_rot), glm::cos(m_y_rot)));
}

void Entity::setPosition(const glm::vec3& inputPosition) {
//    glm::vec3 dp;
//    dp = inputPosition - this->m_position;

    this->m_position = inputPosition;

//    if(this->m_model) {
//        std::vector<float> new_range;
//
//        new_range.push_back(this->m_model->maxRanges[0] + dp.x);
//        new_range.push_back(this->m_model->maxRanges[1] + dp.x);
//
//        new_range.push_back(this->m_model->maxRanges[2] + dp.y);
//        new_range.push_back(this->m_model->maxRanges[3] + dp.y);
//
//        new_range.push_back(this->m_model->maxRanges[4] + dp.z);
//        new_range.push_back(this->m_model->maxRanges[5] + dp.z);
//
//        this->m_model->maxRanges = new_range;
//    }
}

void Entity::placeBottomEdge(float surfaceY) {
    if (m_model != nullptr) {
        m_position.y = surfaceY - m_model->getRangeInDim(1).first * m_scale.y;
    }
}

void Entity::setScale(const glm::vec3& scale) {
    this->m_scale = scale;

    if(this->m_model) {
        std::vector<float> new_range;

        new_range.push_back(this->m_model->maxRanges[0] * scale.x);
        new_range.push_back(this->m_model->maxRanges[1] * scale.x);

        new_range.push_back(this->m_model->maxRanges[2] * scale.y);
        new_range.push_back(this->m_model->maxRanges[3] * scale.y);

        new_range.push_back(this->m_model->maxRanges[4] * scale.z);
        new_range.push_back(this->m_model->maxRanges[5] * scale.z);

        this->m_model->maxRanges = new_range;
    }

}

void Entity::setRotationX(float rot) {
    m_x_rot = rot;
}

void Entity::setRotationY(float rot) {
    m_y_rot = rot;
}

void Entity::setRotationZ(float rot) {
    m_z_rot = rot;
}
// Set the value of rotation or position relatively (Takes into account current value)
void Entity::rotateX(float rot) {
    m_x_rot += rot;
}

void Entity::rotateY(float rot) {
    m_y_rot += rot;
}

void Entity::rotateZ(float rot) {
    m_z_rot += rot;
}

void Entity::move(const glm::vec3& movement) {
    m_position = m_position + movement;
//    if(this->m_model) {
//        std::vector<float> new_range;
//
//        new_range.push_back(this->m_model->maxRanges[0] + movement.x);
//        new_range.push_back(this->m_model->maxRanges[1] + movement.x);
//
//        new_range.push_back(this->m_model->maxRanges[2] + movement.y);
//        new_range.push_back(this->m_model->maxRanges[3] + movement.y);
//
//        new_range.push_back(this->m_model->maxRanges[4] + movement.z);
//        new_range.push_back(this->m_model->maxRanges[5] + movement.z);
//
//        this->m_model->maxRanges = new_range;
//    }
}

glm::mat4 Entity::calculateModelMatrix(
    const glm::vec3& position, const glm::mat4& rotationMat, const glm::vec3& scale) {
    glm::mat4 modelMatrix(1.0f);

    // scale, rotate and translate
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = modelMatrix * rotationMat;
    modelMatrix = glm::scale(modelMatrix, scale);

    return modelMatrix;
}

glm::mat4 Entity::calculateRotationMatrix(float xRot, float yRot, float zRot) {
    glm::mat4 rotation(1.0f);

    rotation = glm::rotate(rotation, yRot, glm::vec3(0.0f, 1.0f, 0.0f));
    rotation = glm::rotate(rotation, xRot, glm::vec3(1.0f, 0.0f, 0.0f));
    rotation = glm::rotate(rotation, zRot, glm::vec3(0.0f, 0.0f, 1.0f));

    return rotation;
}
