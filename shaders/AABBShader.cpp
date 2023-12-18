//
// Created by sirui lai on 2023/12/18.
//
#include "AABBShader.h"

AABBShader::AABBShader() : ShaderProgram(AABB_VERTEX_SHADER, AABB_FRAGMENT_SHADER) {
    bindUniformLocations();
}

void AABBShader::bindUniformLocations() {

    location_projection = glGetUniformLocation(shaderID, "projection");
    location_model = glGetUniformLocation(shaderID, "model");
    location_view = glGetUniformLocation(shaderID, "view");

}

void AABBShader::loadView(const glm::mat4& view) {
    loadUniformValue(location_view, view);
}

void AABBShader::loadProjection(const glm::mat4& proj) {
    loadUniformValue(location_projection, proj);
}

void AABBShader::loadModel(const glm::mat4& model) {
    loadUniformValue(location_model, model);
}