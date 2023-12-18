#include "EntityRenderer.h"

void EntityRenderer::render(const std::vector<Entity*>& entities, const std::vector<Light*>& lights,
    const glm::mat4& view, const glm::mat4& proj, GLuint reflectionTexture, const glm::vec4& clipPlane) {
    m_shader.enable();
    m_shader.loadProjection(proj);
    m_shader.loadLights(lights);
    m_shader.loadView(view);
    m_shader.loadClipPlane(clipPlane);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, reflectionTexture);

    for (const auto& entity : entities) {
        m_shader.loadEntity(entity);
        if (entity->getModel() != nullptr) {
            renderModel(entity->getModel());
        }
    }

    m_shader.disable();

    for (const auto& entity : entities) {
        if (entity->getModel() != nullptr) {
            renderBound(*entity, view, proj);
        }
    }
}

void EntityRenderer::render(const std::vector<Entity*>& entities, const std::vector<Light*>& lights,
    const glm::mat4& view, const glm::mat4& proj, GLuint reflectionTexture, const glm::mat4& depthView,
    const glm::mat4& depthProj, GLuint shadowMap, const glm::vec4& clipPlane) {
    m_shader.enable();
    m_shader.loadProjection(proj);
    m_shader.loadLights(lights);
    m_shader.loadView(view);
    m_shader.loadClipPlane(clipPlane);

    glm::mat4 biasMatrix(0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.5, 0.5, 0.5, 1.0);

    glm::mat4 depthBiasPV = biasMatrix * depthProj * depthView;
    m_shader.loadDepth(depthBiasPV);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, reflectionTexture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, shadowMap);

    for (const auto& entity : entities) {
        m_shader.loadEntity(entity);
        if (entity->getModel() != nullptr) {
            renderModel(entity->getModel());
        }
    }

    m_shader.disable();


    AABBShader.enable();
    AABBShader.loadProjection(proj);
    AABBShader.loadView(view);

    for (const auto& entity : entities) {
        if (entity->getModel() != nullptr) {
            renderBound(*entity, view, proj);
        }
    }
    AABBShader.disable();
}

void EntityRenderer::renderModel(const MY_Model* model) {
    for (const auto& component : model->getModelComponents()) {
        m_shader.loadModelComponent(component);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, component.getTextureID());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glBindVertexArray(component.getVaoID());

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);

        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(component.getIndexCount()), GL_UNSIGNED_INT, (void*)0);

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
        glBindVertexArray(0);
    }
}

void EntityRenderer::renderBound(Entity& entity, const glm::mat4& view, const glm::mat4& proj) {
    unsigned int cubeVAO, cubeVBO, cubeEBO;

    GLfloat cubeVertices[] = {
            -0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f,  0.5f, -0.5f,
            -0.5f,  0.5f, -0.5f,
            -0.5f, -0.5f,  0.5f,
            0.5f, -0.5f,  0.5f,
            0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,
    };

    GLuint cubeIndices[] = {
            0, 1, 1, 2, 2, 3, 3, 0,
            4, 5, 5, 6, 6, 7, 7, 4,
            0, 4, 1, 5, 2, 6, 3, 7
    };

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glGenBuffers(1, &cubeEBO);

    glBindVertexArray(cubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // 解绑VAO
    glBindVertexArray(0);


    glm::mat4 modelAABB = glm::mat4(1.0f);

    glm::vec3 aabbSize = glm::vec3 (entity.getModel()->getRangeInDim(0).second - entity.getModel()->getRangeInDim(0).first,
    + entity.getModel()->getRangeInDim(1).second - entity.getModel()->getRangeInDim(1).first,
            + entity.getModel()->getRangeInDim(2).second - entity.getModel()->getRangeInDim(2).first);
    glm::vec3 aabbCenter = glm::vec3 (+ entity.getModel()->getRangeInDim(0).second + entity.getModel()->getRangeInDim(0).first,
                                      + entity.getModel()->getRangeInDim(1).second + entity.getModel()->getRangeInDim(1).first,
                                      + entity.getModel()->getRangeInDim(2).second + entity.getModel()->getRangeInDim(2).first)*0.5f;
    aabbSize *= 1.0f;
    aabbCenter *= 1.0f;

    modelAABB = glm::translate(modelAABB, entity.getPosition());
    modelAABB = glm::translate(modelAABB, aabbCenter);
    modelAABB = glm::scale(modelAABB, aabbSize);

    AABBShader.loadModel(modelAABB);

//        vertices[0] = entity.getModel()AABB * glm::vec4(+ entity.getModel()->getRangeInDim(0).first, + entity.getModel()->getRangeInDim(1).first, + entity.getModel()->getRangeInDim(2).first, 1.0f);
//        vertices[1] = entity.getModel()AABB * glm::vec4(+ entity.getModel()->getRangeInDim(0).second, + entity.getModel()->getRangeInDim(1).first, + entity.getModel()->getRangeInDim(2).first, 1.0f);
//        vertices[2] = entity.getModel()AABB * glm::vec4(+ entity.getModel()->getRangeInDim(0).second, + entity.getModel()->getRangeInDim(1).second, + entity.getModel()->getRangeInDim(2).first, 1.0f);
//        vertices[3] = entity.getModel()AABB * glm::vec4(+ entity.getModel()->getRangeInDim(0).first, + entity.getModel()->getRangeInDim(1).second, + entity.getModel()->getRangeInDim(2).first, 1.0f);
//        vertices[4] = entity.getModel()AABB * glm::vec4(+ entity.getModel()->getRangeInDim(0).first, + entity.getModel()->getRangeInDim(1).first, + entity.getModel()->getRangeInDim(2).second, 1.0f);
//        vertices[5] = entity.getModel()AABB * glm::vec4(+ entity.getModel()->getRangeInDim(0).second, + entity.getModel()->getRangeInDim(1).first, + entity.getModel()->getRangeInDim(2).second, 1.0f);
//        vertices[6] = entity.getModel()AABB * glm::vec4(+ entity.getModel()->getRangeInDim(0).second, + entity.getModel()->getRangeInDim(1).second, + entity.getModel()->getRangeInDim(2).second, 1.0f);
//        vertices[7] = entity.getModel()AABB * glm::vec4(+ entity.getModel()->getRangeInDim(0).first, + entity.getModel()->getRangeInDim(1).second, + entity.getModel()->getRangeInDim(2).second, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(cubeVAO);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}