#include <string>

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

AnimatedModelRenderer::AnimatedModelRenderer()
{
    shader = new AnimatedModelShader(); INCR_NEW("AnimatedModelShader");
}

void AnimatedModelRenderer::render(AnimatedModel* entity)
{
    prepare();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, entity->texture);

    std::vector<GLuint> bindNums;
    bindNums.push_back(0);
    bindNums.push_back(1);
    bindNums.push_back(2);
    bindNums.push_back(3);
    bindNums.push_back(4);
    entity->model->bind(&bindNums);

    std::vector<Matrix4f> jointTransforms = entity->calculateJointTransforms();
    for (int i = 0; i < jointTransforms.size(); i++)
    {
        shader->loadMatrix(shader->location_jointTransforms[i], &jointTransforms[i]);
    }

    glDrawElements(GL_TRIANGLES, entity->model->indexCount, GL_UNSIGNED_INT, nullptr);

    entity->model->unbind(&bindNums);
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
