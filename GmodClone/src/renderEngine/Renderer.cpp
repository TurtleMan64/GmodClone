#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "renderEngine.hpp"
#include "../entities/entity.hpp"
#include "../shaders/shaderprogram.hpp"
#include "../toolbox/matrix.hpp"
#include "../toolbox/vector.hpp"
#include "../toolbox/maths.hpp"
#include "../models/models.hpp"
#include "../main/main.hpp"

#include <iostream>
#include <unordered_map>
#include <list>

GLuint flatNormalMapId = GL_NONE;

EntityRenderer::EntityRenderer(ShaderProgram* shader, Matrix4f* projectionMatrix)
{
    shader->start();
    shader->loadProjectionMatrix(projectionMatrix);
    shader->stop();
    this->shader = shader;

    if (flatNormalMapId == GL_NONE)
    {
        flatNormalMapId = Loader::loadTexture("res/Images/NormalMapFlat.png");
    }
}

void EntityRenderer::render(std::unordered_map<TexturedModel*, std::list<Entity*>>* entitiesMap, Matrix4f* /*toShadowSpaceFar*/, Matrix4f* /*toShadowSpaceClose*/)
{
    if (entitiesMap->size() == 0)
    {
        return;
    }

    //if (Global::renderShadowsFar)
    {
        //shader->loadToShadowSpaceMatrixFar(toShadowSpaceFar);
    }

    //if (Global::renderShadowsClose)
    {
        //shader->loadToShadowSpaceMatrixClose(toShadowSpaceClose);
    }

    clockTime = (float)glfwGetTime();

    unsigned int entityId = 0;

    for (auto entry : (*entitiesMap))
    {
        prepareTexturedModel(entry.first);
        std::list<Entity*>* entityList = &entry.second;

        for (Entity* entity : (*entityList))
        {
            prepareInstance(entity, entityId);
            entityId++;
            glDrawElements(GL_TRIANGLES, (entry.first)->getRawModel()->getVertexCount(), GL_UNSIGNED_INT, 0);
        }
        unbindTexturedModel();
    }
}

void EntityRenderer::prepareTexturedModel(TexturedModel* model)
{
    RawModel* rawModel = model->getRawModel();
    glBindVertexArray(rawModel->getVaoId());
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);

    ModelTexture* texture = model->getTexture();

    shader->loadFakeLighting(texture->useFakeLighting);
    shader->loadShineVariables(texture->shineDamper, texture->reflectivity);
    shader->loadTransparency(texture->hasTransparency);
    shader->loadGlowAmount(texture->glowAmount);
    shader->loadTextureOffsets(clockTime * (texture->scrollX), clockTime * (texture->scrollY));
    shader->loadMixFactor(texture->mixFactor());
    shader->loadFogScale(texture->fogScale);
    shader->loadNoise(texture->noise);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->getId());
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture->getId2());
    glActiveTexture(GL_TEXTURE2);
    if (texture->normalMapId == GL_NONE)
    {
        glBindTexture(GL_TEXTURE_2D, flatNormalMapId);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, texture->normalMapId);
    }
}

void EntityRenderer::unbindTexturedModel()
{
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
    glBindVertexArray(0);
}

void EntityRenderer::prepareInstance(Entity* entity, unsigned int entityId)
{
    shader->loadTransformationMatrix(&entity->transformationMatrix);
    shader->loadBaseColor(&entity->baseColor);
    shader->loadBaseAlpha(entity->baseAlpha);
    shader->loadEntityId(entityId);
}

void EntityRenderer::updateProjectionMatrix(Matrix4f* projectionMatrix)
{
    shader->start();
    shader->loadProjectionMatrix(projectionMatrix);
    shader->stop();
}
