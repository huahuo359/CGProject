//
// Created by sirui lai on 2023/12/18.
//

#ifndef MAIN_AABBSHADER_H
#define MAIN_AABBSHADER_H
#pragma once

#define _USE_MATH_DEFINES

#include "../entity/Entity.h"
#include "../entity/Light.h"
#include "../entity/MY_Model.h"
#include "ShaderProgram.h"

#include <cstdio>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glad/glad.h>

const std::string AABB_VERTEX_SHADER = "shaders/AABB.vs";
const std::string AABB_FRAGMENT_SHADER = "shaders/AABB.fs";

class AABBShader : public ShaderProgram {
private:
    GLuint location_projection;
    GLuint location_model;
    GLuint location_view;


public:
    AABBShader();

    void bindUniformLocations();
    void loadView(const glm::mat4& view);
    void loadProjection(const glm::mat4& proj);
    void loadModel(const glm::mat4& model);

};


#endif //MAIN_AABBSHADER_H
