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
    updateBound();
    return true;
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
    this->m_position = inputPosition;
}

void Entity::placeBottomEdge(float surfaceY) {
    if (m_model != nullptr) {
        m_position.y = surfaceY - m_model->getRangeInDim(1).first * m_scale.y;
    }
}

void Entity::setScale(const glm::vec3& scale) {
    this->m_scale = scale;
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

bool Entity::updateBound() {
    glm::mat4 modelAABB = glm::mat4(1.0f);
    const MY_Model* model = this->getModel();

    glm::vec3 aabbSize = glm::vec3 (this->getModel()->getRangeInDim(0).second - this->getModel()->getRangeInDim(0).first,
                                    this->getModel()->getRangeInDim(1).second - this->getModel()->getRangeInDim(1).first,
                                    this->getModel()->getRangeInDim(2).second - this->getModel()->getRangeInDim(2).first);
    glm::vec3 aabbCenter = glm::vec3 (this->getModel()->getRangeInDim(0).second + this->getModel()->getRangeInDim(0).first,
                                      this->getModel()->getRangeInDim(1).second + this->getModel()->getRangeInDim(1).first,
                                      this->getModel()->getRangeInDim(2).second + this->getModel()->getRangeInDim(2).first)*0.5f;
    aabbSize *= this->getScale();
    aabbCenter *= 1.0f;

    modelAABB = glm::translate(modelAABB, this->getPosition());
    modelAABB = glm::translate(modelAABB, aabbCenter);
    modelAABB = glm::rotate(modelAABB, this->getRotationX(), glm::vec3(1.0f,0.0f,0.0f));
    modelAABB = glm::rotate(modelAABB, this->getRotationY(), glm::vec3(0.0f,1.0f,0.0f));
    modelAABB = glm::rotate(modelAABB, this->getRotationZ(), glm::vec3(0.0f,0.0f,1.0f));
    modelAABB = glm::scale(modelAABB, aabbSize);

    vertices[0] = modelAABB * glm::vec4(model->getRangeInDim(0).first, model->getRangeInDim(1).first, model->getRangeInDim(2).first, 1.0f);
    vertices[1] = modelAABB * glm::vec4(model->getRangeInDim(0).second, model->getRangeInDim(1).first, model->getRangeInDim(2).first, 1.0f);
    vertices[2] = modelAABB * glm::vec4(model->getRangeInDim(0).second, model->getRangeInDim(1).second, model->getRangeInDim(2).first, 1.0f);
    vertices[3] = modelAABB * glm::vec4(model->getRangeInDim(0).first, model->getRangeInDim(1).second, model->getRangeInDim(2).first, 1.0f);
    vertices[4] = modelAABB * glm::vec4(model->getRangeInDim(0).first, model->getRangeInDim(1).first, model->getRangeInDim(2).second, 1.0f);
    vertices[5] = modelAABB * glm::vec4(model->getRangeInDim(0).second, model->getRangeInDim(1).first, model->getRangeInDim(2).second, 1.0f);
    vertices[6] = modelAABB * glm::vec4(model->getRangeInDim(0).second, model->getRangeInDim(1).second, model->getRangeInDim(2).second, 1.0f);
    vertices[7] = modelAABB * glm::vec4(model->getRangeInDim(0).first, model->getRangeInDim(1).second, model->getRangeInDim(2).second, 1.0f);
}