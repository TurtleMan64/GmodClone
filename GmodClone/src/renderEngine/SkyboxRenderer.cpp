#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "renderEngine.hpp"
#include "../entities/entity.hpp"
#include "../skybox/skyboxshader.hpp"
#include "../toolbox/matrix.hpp"
#include "../toolbox/vector.hpp"
#include "../toolbox/maths.hpp"
#include "../models/models.hpp"
#include "../main/main.hpp"
#include "../entities/camera.hpp"

#include <iostream>
#include <unordered_map>
#include <list>

float SkyboxRenderer::VERTICES[] = {
    -SIZE,  SIZE, -SIZE,
    -SIZE, -SIZE, -SIZE,
     SIZE, -SIZE, -SIZE,
     SIZE, -SIZE, -SIZE,
     SIZE,  SIZE, -SIZE,
    -SIZE,  SIZE, -SIZE,

    -SIZE, -SIZE,  SIZE,
    -SIZE, -SIZE, -SIZE,
    -SIZE,  SIZE, -SIZE,
    -SIZE,  SIZE, -SIZE,
    -SIZE,  SIZE,  SIZE,
    -SIZE, -SIZE,  SIZE,

     SIZE, -SIZE, -SIZE,
     SIZE, -SIZE,  SIZE,
     SIZE,  SIZE,  SIZE,
     SIZE,  SIZE,  SIZE,
     SIZE,  SIZE, -SIZE,
     SIZE, -SIZE, -SIZE,

    -SIZE, -SIZE,  SIZE,
    -SIZE,  SIZE,  SIZE,
     SIZE,  SIZE,  SIZE,
     SIZE,  SIZE,  SIZE,
     SIZE, -SIZE,  SIZE,
    -SIZE, -SIZE,  SIZE,

    -SIZE,  SIZE, -SIZE,
     SIZE,  SIZE, -SIZE,
     SIZE,  SIZE,  SIZE,
     SIZE,  SIZE,  SIZE,
    -SIZE,  SIZE,  SIZE,
    -SIZE,  SIZE, -SIZE,

    -SIZE, -SIZE, -SIZE,
    -SIZE, -SIZE,  SIZE,
     SIZE, -SIZE, -SIZE,
     SIZE, -SIZE, -SIZE,
    -SIZE, -SIZE,  SIZE,
     SIZE, -SIZE,  SIZE
};

SkyboxRenderer::SkyboxRenderer()
{
    std::vector<float> verts;
    for (int i = 0; i < 108; i++)
    {
        verts.push_back(VERTICES[i]);
    }
    cube = Loader::loadToVAO(&verts, 3);

    std::vector<std::string> fnames;
    fnames.push_back("res/Images/Sky/2/px.png");
    fnames.push_back("res/Images/Sky/2/nx.png");
    fnames.push_back("res/Images/Sky/2/py.png");
    fnames.push_back("res/Images/Sky/2/ny.png");
    fnames.push_back("res/Images/Sky/2/pz.png");
    fnames.push_back("res/Images/Sky/2/nz.png");

    texture = Loader::loadCubeMap(fnames);
    shader = new SkyboxShader("res/Shaders/skybox/SkyVertex.glsl", "res/Shaders/skybox/SkyFrag.glsl");
    shader->start();
    shader->loadProjectionMatrix(Master_getProjectionMatrix());
    shader->connectTextureUnits();
    shader->stop();
}

void SkyboxRenderer::render(float clipX, float clipY, float clipZ, float clipW)
{
    //glEnable(GL_MULTISAMPLE);
    //glEnable(GL_BLEND);
    //glEnable(GL_DEPTH_TEST);
    glEnable(GL_CLIP_DISTANCE0);

    shader->start();
    shader->loadViewMatrix(Global::gameCamera);
    shader->loadClipPlane(clipX, clipY, clipZ, clipW);
    shader->loadCenter(&Global::gameCamera->eye);
    glBindVertexArray(cube.getVaoId());
    glEnableVertexAttribArray(0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    glDrawArrays(GL_TRIANGLES, 0, cube.getVertexCount());
    glDisableVertexAttribArray(0);
    glBindVertexArray(0);
    shader->stop();
}

void SkyboxRenderer::updateProjectionMatrix(Matrix4f* projectionMatrix)
{
    shader->start();
    shader->loadProjectionMatrix(projectionMatrix);
    shader->stop();
}
