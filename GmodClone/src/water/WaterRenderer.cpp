#include "waterrenderer.hpp"
#include "../renderEngine/renderEngine.hpp"
#include "watershader.hpp"
#include "waterframebuffers.hpp"
#include "watertile.hpp"
#include <list>
#include <vector>
#include "../main/main.hpp"
#include "../toolbox/maths.hpp"
#include "../toolbox/vector.hpp"
#include "../toolbox/matrix.hpp"
#include "../entities/camera.hpp"

WaterRenderer::WaterRenderer(WaterShader* shader, Matrix4f* projectionMatrix, WaterFrameBuffers* fbos)
{
    this->shader = shader;
    this->fbos = fbos;
    dudvTexture = Loader::loadTexture("res/Images/waterDUDV.png");
    normalMap   = Loader::loadTexture("res/Images/normalMap.png");
    shader->start();
    shader->connectTextureUnits();
    shader->loadProjectionMatrix(projectionMatrix);
    shader->stop();
    setUpVAO();
}

void WaterRenderer::prepareRender(Camera* camera, Light* sun)
{
    shader->start();
    shader->loadViewMatrix(camera);

    //calc behind clipm plane based on camera
    Vector3f camDir = camera->target - camera->eye;
    camDir.normalize();
    camDir.inv();
    Vector3f startPos(&camera->eye);
    //startPos = startPos + camDir.scaleCopy(-100);
    Vector4f plane = Maths::calcPlaneValues(&startPos, &camDir);
    shader->loadClipPlaneBehind(&plane);

    //extern float dt;
    //moveFactor += WaterRenderer::WAVE_SPEED*dt;
    moveFactor = WaterRenderer::WAVE_SPEED*(float)Global::syncedGlobalTime;
    moveFactor = fmodf(moveFactor, 1);
    shader->loadMoveFactor(moveFactor);
    shader->loadSun(sun);
    shader->loadWaterHeight(Global::waterHeight);
    shader->loadWaterMurkyAmount(0.15f);
    shader->loadWaterColor(&Global::waterColor);
    glBindVertexArray(quad->getVaoId());
    glEnableVertexAttribArray(0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture  (GL_TEXTURE_2D, fbos->getReflectionTexture());
    glActiveTexture(GL_TEXTURE1);
    glBindTexture  (GL_TEXTURE_2D, fbos->getRefractionTexture());
    glActiveTexture(GL_TEXTURE2);
    glBindTexture  (GL_TEXTURE_2D, dudvTexture);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture  (GL_TEXTURE_2D, normalMap);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture  (GL_TEXTURE_2D, fbos->getRefractionDepthTexture());
    glDepthMask(true);
    glEnable(GL_CLIP_DISTANCE1);
    glDisable(GL_CULL_FACE);
}

void WaterRenderer::unbind()
{
    glDisableVertexAttribArray(0);
    glBindVertexArray(0);
    glDisable(GL_CLIP_DISTANCE1);
    shader->stop();
}

void WaterRenderer::setUpVAO()
{
    // Just x and z vectex positions here, y is set to 0 in v.shader
    std::vector<float> vertices;
    vertices.push_back(-1);
    vertices.push_back(-1);
    vertices.push_back(-1);
    vertices.push_back( 1);
    vertices.push_back( 1);
    vertices.push_back(-1);
    vertices.push_back( 1);
    vertices.push_back(-1);
    vertices.push_back(-1);
    vertices.push_back( 1);
    vertices.push_back( 1);
    vertices.push_back( 1);

    quad = new RawModel(Loader::loadToVAO(&vertices, 2)); INCR_NEW("RawModel");
}

void WaterRenderer::render(std::vector<WaterTile*>* water, Camera* camera, Light* sun)
{
    prepareRender(camera, sun);
    float xOff = camera->eye.x;
    float zOff = camera->eye.z;
    const int waterTileCount = (int)water->size();
    for (int i = 0; i < waterTileCount; i++)
    {
        WaterTile* tile = water->at(i);
        Matrix4f modelMatrix;
        Vector3f tilePosition(tile->centerX + xOff, Global::waterHeight, tile->centerZ + zOff);
        Maths::createTransformationMatrix(&modelMatrix, &tilePosition, 0, 0, 0, 0, WaterTile::TILE_SIZE);
        shader->loadModelMatrix(&modelMatrix);
        glDrawArrays(GL_TRIANGLES, 0, quad->getVertexCount());
    }
    unbind();
}

void WaterRenderer::updateProjectionMatrix(Matrix4f* projectionMatrix)
{
    shader->start();
    shader->loadProjectionMatrix(projectionMatrix);
    shader->stop();
}
