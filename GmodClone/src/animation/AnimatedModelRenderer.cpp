#include <string>
#include <vector>

#include "texture.hpp"
#include "../toolbox/vector.hpp"
#include "../toolbox/maths.hpp"
#include "../toolbox/matrix.hpp"
#include "animatedmodelrenderer.hpp"
#include "animatedmodel.hpp"
#include "jointtransform.hpp"
#include "joint.hpp"
#include "../entities/camera.hpp"
#include "animatedmodelshader.hpp"
#include "../main/main.hpp"
#include "../openglObjects/vao.hpp"
#include "../entities/light.hpp"
#include "../entities/entity.hpp"

AnimatedModelRenderer::AnimatedModelRenderer()
{
    shader = new AnimatedModelShader(); INCR_NEW("AnimatedModelShader");
}

void AnimatedModelRenderer::render(std::unordered_map<AnimatedModel*, std::vector<Entity*>>* animatedEntitiesMap)
{
    if (animatedEntitiesMap->size() == 0)
    {
        return;
    }

    prepare();

    for (auto entry : (*animatedEntitiesMap))
    {
        AnimatedModel* animatedModel = entry.first;
        std::vector<Entity*>* entityList = &entry.second;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, animatedModel->texture);

        std::vector<GLuint> bindNums;
        bindNums.push_back(0);
        bindNums.push_back(1);
        bindNums.push_back(2);
        bindNums.push_back(3);
        bindNums.push_back(4);
        animatedModel->model->bind(&bindNums);

        for (Entity* entity : (*entityList))
        {
            if (!entity->visible)
            {
                continue;
            }

            std::vector<Matrix4f>* jointTransforms = &entity->jointTransforms;
            for (int i = 0; i < jointTransforms->size(); i++)
            {
                shader->loadMatrix(shader->location_jointTransforms[i], &jointTransforms->at(i));
            }

            glDrawElements(GL_TRIANGLES, animatedModel->model->indexCount, GL_UNSIGNED_INT, nullptr);
        }

        animatedModel->model->unbind(&bindNums);
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    finish();
}

void AnimatedModelRenderer::cleanUp()
{
    shader->cleanUp();
}

void AnimatedModelRenderer::prepare()
{
    shader->start();

    Matrix4f projectionViewMatrix = Global::gameCamera->calculateProjectionViewMatrix();
    shader->loadMatrix(shader->location_projectionViewMatrix, &projectionViewMatrix);
    shader->loadVector(shader->location_lightDirection, &Global::lights[0]->direction);

    glEnable(GL_MULTISAMPLE);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void AnimatedModelRenderer::finish()
{
    shader->stop();
}
