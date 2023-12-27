#pragma once

#define _USE_MATH_DEFINES

#include "MY_Model.h"

#include <cassert>
#include <string>
#include <iostream>
#include <glad/glad.h>
#include <glm/glm.hpp>

class Entity {
  protected:
    MY_Model* m_model;

    glm::vec3 m_position;
    glm::vec3 m_scale;
    float m_x_rot;
    float m_y_rot;
    float m_z_rot;

    glm::vec4 vertices[8];
    bool is_player = false;
    bool is_base = false;
    float boundsize;

  public:
    Entity(MY_Model* model);
    Entity();

    virtual ~Entity() = default;

    virtual bool update();
    bool updateBound();

    [[nodiscard]] const MY_Model* getModel() const;
    [[nodiscard]] glm::mat4 calculateModelMatrix() const;
    [[nodiscard]] const glm::vec3& getPosition() const;
    [[nodiscard]] const glm::vec3& getScale() const;
    [[nodiscard]] float getRotationX() const;
    [[nodiscard]] float getRotationY() const;
    [[nodiscard]] float getRotationZ() const;
    inline bool getIsPlayer() const { return is_player; }
    inline glm::vec4 getVertices(int index) const { return vertices[index]; }
    inline float getBoundSize() const { return boundsize; }
    inline bool getIsbase() const { return this->is_base; }

    [[nodiscard]] glm::vec3 calculateDirectionVector() const;

    // Can be overridden in inheriting class if behaviour requires it.
    // Set the value absolutely of position, scale, or rotation.
    virtual void setPosition(const glm::vec3&);
    virtual void placeBottomEdge(float surfaceY);
    virtual void setScale(const glm::vec3&);
    virtual void setRotationX(float);
    virtual void setRotationY(float);
    virtual void setRotationZ(float);
    inline void setBoundSize(float sz) { boundsize = sz; }
    inline void setIsbase(bool is) { this->is_base = is; }

    // Set the value of rotation or position relatively (Takes into account current value)
    virtual void rotateX(float);
    virtual void rotateY(float);
    virtual void rotateZ(float);
    virtual void move(const glm::vec3&);

    // Generates the transformation to be applied to the mesh with the given parameters.
    static glm::mat4 calculateModelMatrix(const glm::vec3& position, const glm::mat4& rotation, const glm::vec3& scale);
    static glm::mat4 calculateRotationMatrix(float xRot, float yRot, float zRot);
};
