#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "renderEngine.hpp"
#include "../entities/entity.hpp"
#include "../shaders/stageshader.hpp"
#include "../toolbox/matrix.hpp"
#include "../toolbox/vector.hpp"
#include "../toolbox/maths.hpp"
#include "../models/models.hpp"
#include "../main/main.hpp"

#include <iostream>
#include <unordered_map>
#include <list>

GLuint flatNormal2MapId = GL_NONE;
GLuint whiteLightMapId = GL_NONE;

StageRenderer::StageRenderer()
{
    shader = new StageShader("res/Shaders/stage/StageVert.glsl", "res/Shaders/stage/StageFrag.glsl");
    shader->start();
    shader->loadProjectionMatrix(Master_getProjectionMatrix());
    shader->connectTextureUnits();
    shader->stop();

    if (flatNormal2MapId == GL_NONE)
    {
        flatNormal2MapId = Loader::loadTexture("res/Images/NormalMapFlat.png");
    }

    if (whiteLightMapId == GL_NONE)
    {
        whiteLightMapId = Loader::loadTexture("res/Images/white.png");
    }
}

void StageRenderer::render(LightModel* lightModel, float clipX, float clipY, float clipZ, float clipW)
{
    if (lightModel == nullptr)
    {
        return;
    }

    //glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CLIP_DISTANCE0);

    shader->start();
    shader->loadViewMatrix(Global::gameCamera);
    shader->loadClipPlane(clipX, clipY, clipZ, clipW);

    clockTime = (float)glfwGetTime();

    GLuint currentLightmapId = GL_NONE;

    for (int i = 0; i < 5; i++)
    {
        for (TexturedModel* model : lightModel->texturedModels)
        {
            if (model->getTexture()->renderOrder == i)
            {
                prepareTexturedModel(model, &currentLightmapId);
                glDrawElements(GL_TRIANGLES, (model)->getRawModel()->getVertexCount(), GL_UNSIGNED_INT, 0);
                unbindTexturedModel();
            }
        }
    }

    glDisable(GL_CLIP_DISTANCE0);

    shader->stop();
}

void StageRenderer::prepareTexturedModel(TexturedModel* model, GLuint* currentLightmapId)
{
    RawModel* rawModel = model->getRawModel();
    glBindVertexArray(rawModel->getVaoId());
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);

    ModelTexture* texture = model->getTexture();

    shader->loadFakeLighting(texture->useFakeLighting);
    shader->loadShineVariables(texture->shineDamper, texture->reflectivity);
    shader->loadTransparency(texture->hasTransparency);
    shader->loadGlowAmount(texture->glowAmount);
    shader->loadTextureOffsets(clockTime * (texture->scrollX), clockTime * (texture->scrollY));
    shader->loadMixFactor(texture->mixFactor());
    shader->loadFogScale(texture->fogScale);
    shader->loadNoise(texture->noise);
    shader->loadLights();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->getId());

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture->getId2());

    glActiveTexture(GL_TEXTURE2);
    if (texture->normalMapId == GL_NONE)
    {
        glBindTexture(GL_TEXTURE_2D, flatNormal2MapId);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, texture->normalMapId);
    }

    GLuint newLightmapId = whiteLightMapId;
    if (texture->lightMapId != GL_NONE)
    {
        newLightmapId = texture->lightMapId;
    }

    // Some maps share a single lightmap. This might save some time if we don't keep binding it over and over.
    if (*currentLightmapId != newLightmapId)
    {
        *currentLightmapId = newLightmapId;

        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, newLightmapId);
    }
}

void StageRenderer::unbindTexturedModel()
{
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
    glDisableVertexAttribArray(5);
    glBindVertexArray(0);
}

void StageRenderer::prepareInstance(Entity* entity, unsigned int entityId)
{
    shader->loadTransformationMatrix(&entity->transformationMatrix);
    shader->loadBaseColor(&entity->baseColor);
    shader->loadBaseAlpha(entity->baseAlpha);
    shader->loadEntityId(entityId);
}

void StageRenderer::updateProjectionMatrix(Matrix4f* projectionMatrix)
{
    shader->start();
    shader->loadProjectionMatrix(projectionMatrix);
    shader->stop();
}
