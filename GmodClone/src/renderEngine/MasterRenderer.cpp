#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../entities/light.hpp"
#include "../entities/camera.hpp"
#include "../shaders/shaderprogram.hpp"
#include "../entities/entity.hpp"
#include "../models/models.hpp"
#include "renderEngine.hpp"
#include "../toolbox/maths.hpp"
#include "../toolbox/matrix.hpp"
#include "../main/main.hpp"

#include <iostream>
#include <list>
#include <unordered_map>
#include <stdexcept>

ShaderProgram* shader = nullptr;
EntityRenderer* renderer = nullptr;
ShadowMapMasterRenderer* shadowMapRenderer = nullptr;
ShadowMapMasterRenderer2* shadowMapRenderer2 = nullptr;

std::unordered_map<TexturedModel*, std::list<Entity*>> entitiesMap;
std::unordered_map<TexturedModel*, std::list<Entity*>> entitiesMapPass2;
std::unordered_map<TexturedModel*, std::list<Entity*>> entitiesMapPass3;
std::unordered_map<TexturedModel*, std::list<Entity*>> entitiesMapNoDepth;
std::unordered_map<TexturedModel*, std::list<Entity*>> entitiesMapTransparent;

Matrix4f* projectionMatrix = nullptr;

float VFOV_BASE = 60; //Vertical fov
float VFOV_ADDITION = 0; //additional fov due to the vehicle going fast
const float NEAR_PLANE = 0.05f; //0.5
const float FAR_PLANE = 3000.0f; //15000

void prepare();
void prepareTransparentRender();
void prepareRenderDepthOnly();

GLuint randomMap = GL_NONE;


GLuint transparentFrameBuffer  = GL_NONE;
//GLuint transparentColorTexture = GL_NONE;
GLuint transparentDepthTexture = GL_NONE;

extern unsigned int SCR_WIDTH;
extern unsigned int SCR_HEIGHT;

void Master_init()
{
    //if (Global::renderShadowsFar)
    //{
    //    if (Global::renderShadowsClose)
    //    {
    //        if (Global::renderBloom)
    //        {
    //            switch (Global::shadowsFarQuality)
    //            {
    //            case 0:  shader = new ShaderProgram("res/Shaders/entity/vertexShaderShadowBoth.txt", "res/Shaders/entity/fragmentShaderShadow1BothBloom.txt");  break;
    //            case 1:  shader = new ShaderProgram("res/Shaders/entity/vertexShaderShadowBoth.txt", "res/Shaders/entity/fragmentShaderShadow9BothBloom.txt");  break;
    //            default: shader = new ShaderProgram("res/Shaders/entity/vertexShaderShadowBoth.txt", "res/Shaders/entity/fragmentShaderShadow25BothBloom.txt"); break;
    //            }
    //        }
    //        else
    //        {
    //            switch (Global::shadowsFarQuality)
    //            {
    //            case 0:  shader = new ShaderProgram("res/Shaders/entity/vertexShaderShadowBoth.txt", "res/Shaders/entity/fragmentShaderShadow1Both.txt");  break;
    //            case 1:  shader = new ShaderProgram("res/Shaders/entity/vertexShaderShadowBoth.txt", "res/Shaders/entity/fragmentShaderShadow9Both.txt");  break;
    //            default: shader = new ShaderProgram("res/Shaders/entity/vertexShaderShadowBoth.txt", "res/Shaders/entity/fragmentShaderShadow25Both.txt"); break;
    //            }
    //        }
    //    }
    //    else
    //    {
    //        if (Global::renderBloom)
    //        {
    //            switch (Global::shadowsFarQuality)
    //            {
    //            case 0:  shader = new ShaderProgram("res/Shaders/entity/vertexShaderShadowFar.txt", "res/Shaders/entity/fragmentShaderShadow1FarBloom.txt");  break;
    //            case 1:  shader = new ShaderProgram("res/Shaders/entity/vertexShaderShadowFar.txt", "res/Shaders/entity/fragmentShaderShadow9FarBloom.txt");  break;
    //            default: shader = new ShaderProgram("res/Shaders/entity/vertexShaderShadowFar.txt", "res/Shaders/entity/fragmentShaderShadow25FarBloom.txt"); break;
    //            }
    //        }
    //        else
    //        {
    //            switch (Global::shadowsFarQuality)
    //            {
    //            case 0:  shader = new ShaderProgram("res/Shaders/entity/vertexShaderShadowFar.txt", "res/Shaders/entity/fragmentShaderShadow1Far.txt");  break;
    //            case 1:  shader = new ShaderProgram("res/Shaders/entity/vertexShaderShadowFar.txt", "res/Shaders/entity/fragmentShaderShadow9Far.txt");  break;
    //            default: shader = new ShaderProgram("res/Shaders/entity/vertexShaderShadowFar.txt", "res/Shaders/entity/fragmentShaderShadow25Far.txt"); break;
    //            }
    //        }
    //    }
    //}
    //else
    //{
    //    if (Global::renderShadowsClose)
    //    {
    //        if (Global::renderBloom)
    //        {
    //            shader = new ShaderProgram("res/Shaders/entity/vertexShaderShadowClose.txt", "res/Shaders/entity/fragmentShaderShadowCloseBloom.txt");
    //        }
    //        else
    //        {
    //            shader = new ShaderProgram("res/Shaders/entity/vertexShaderShadowClose.txt", "res/Shaders/entity/fragmentShaderShadowClose.txt");
    //        }
    //    }
    //    else
    //    {
    //        if (Global::renderBloom)
    //        {
    //            shader = new ShaderProgram("res/Shaders/entity/vertexShader.txt", "res/Shaders/entity/fragmentShaderBloom.txt");
    //        }
    //        else
            {
                shader = new ShaderProgram("res/Shaders/entity/vertexShader.txt", "res/Shaders/entity/fragmentShader.txt");
            }
    //    }
    //}
    INCR_NEW("ShaderProgram");

    projectionMatrix = new Matrix4f; INCR_NEW("Matrix4f");

    renderer = new EntityRenderer(shader, projectionMatrix); INCR_NEW("EntityRenderer");
    Master_makeProjectionMatrix();


    //shadowMapRenderer = new ShadowMapMasterRenderer; INCR_NEW("ShadowMapMasterRenderer");
    //shadowMapRenderer2 = new ShadowMapMasterRenderer2; INCR_NEW("ShadowMapMasterRenderer2");

    randomMap = Loader::loadTextureNoInterpolation("res/Images/randomMap.png");


    //create frame buffer
    glGenFramebuffers(1, &transparentFrameBuffer); //generate name for frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, transparentFrameBuffer); //create the framebuffer
    glDrawBuffer(GL_COLOR_ATTACHMENT0); //indicate that we will always render to color attachment 0

    //create a color texture for no reason
    //glGenTextures(1, &transparentColorTexture);
    //glBindTexture(GL_TEXTURE_2D, transparentColorTexture);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, transparentColorTexture, 0);

    //create depth texture attachement
    glGenTextures(1, &transparentDepthTexture);
    glBindTexture(GL_TEXTURE_2D, transparentDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);  //here is the depth bits
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, transparentDepthTexture, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);


    Master_disableCulling();
}

void Master_render(Camera* camera, float clipX, float clipY, float clipZ, float clipW, float waterBlendAmount)
{
    ANALYSIS_START("Master Render");
    GLint currFB;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currFB);

    prepare();
    shader->start();
    shader->loadClipPlane(clipX, clipY, clipZ, clipW);

    //calc behind clipm plane based on camera
    Vector3f lookDir = camera->target - camera->eye;
    lookDir.normalize();
    lookDir.neg();
    Vector3f startPos(&camera->eye);
    //startPos = startPos + camDir.scaleCopy(-100);
    Vector4f plane = Maths::calcPlaneValues(&startPos, &lookDir);
    shader->loadClipPlaneBehind(plane.x, plane.y, plane.z, plane.w);

    shader->loadSkyColor(Global::skyColor.x, Global::skyColor.y, Global::skyColor.z);
    shader->loadSun(Global::gameLightSun);
    shader->loadFogGradient(0.0000000000000005f);
    shader->loadFogDensity(2.0f);
    shader->loadFogBottomPosition(-1000000.0f);
    shader->loadFogBottomThickness(1.0f);
    shader->loadViewMatrix(camera);
    shader->loadIsRenderingTransparent(false);
    shader->loadIsRenderingDepth(false);
    Vector3f waterColor(1,1,1);
    shader->loadWaterColor(&waterColor);
    shader->loadWaterBlendAmount(waterBlendAmount);
    shader->connectTextureUnits();

    //change this back
    glDepthMask(true);
    renderer->renderNEW(&entitiesMap,      nullptr, nullptr);
    renderer->renderNEW(&entitiesMapPass2, nullptr, nullptr);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D, 0);
    shader->stop();

    ANALYSIS_DONE("Master Render");
}

void Master_processEntity(Entity* e)
{
    std::vector<Entity*>* entitiesToRender = e->getEntitiesToRender();

    if (entitiesToRender == nullptr)
    {
        return;
    }

    for (Entity* entity : *entitiesToRender)
    {
        if (!entity->visible)
        {
            continue;
        }

        if (entity->renderOrderOverride <= 4)
        {
            std::unordered_map<TexturedModel*, std::list<Entity*>>* mapToUse = nullptr;

            switch (entity->renderOrderOverride)
            {
                case 0: mapToUse = &entitiesMap;            break;
                case 1: mapToUse = &entitiesMapPass2;       break;
                case 2: mapToUse = &entitiesMapPass3;       break;
                case 3: mapToUse = &entitiesMapTransparent; break;
                case 4: mapToUse = &entitiesMapNoDepth;     break;
                default: break;
            }

            std::list<TexturedModel*>* modellist = entity->getModels();

            for (TexturedModel* entityModel : (*modellist))
            {
                std::list<Entity*>* list = &(*mapToUse)[entityModel];
                list->push_back(entity);
            }
        }
        else
        {
            std::list<TexturedModel*>* modellist = entity->getModels();

            for (TexturedModel* entityModel : (*modellist))
            {
                std::unordered_map<TexturedModel*, std::list<Entity*>>* mapToUse = nullptr;

                switch (entityModel->renderOrder)
                {
                    case 0: mapToUse = &entitiesMap;            break;
                    case 1: mapToUse = &entitiesMapPass2;       break;
                    case 2: mapToUse = &entitiesMapPass3;       break;
                    case 3: mapToUse = &entitiesMapTransparent; break;
                    case 4: mapToUse = &entitiesMapNoDepth;     break;
                    default: break;
                }

                std::list<Entity*>* list = &(*mapToUse)[entityModel];
                list->push_back(entity);
            }
        }
    }
}

void Master_clearAllEntities()
{
    entitiesMap.clear();
    entitiesMapPass2.clear();
    entitiesMapPass3.clear();
    entitiesMapTransparent.clear();
    entitiesMapNoDepth.clear();
}

void prepare()
{
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(true);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(Global::skyColor.x, Global::skyColor.y, Global::skyColor.z, 1);

    //glActiveTexture(GL_TEXTURE5);
    //glBindTexture(GL_TEXTURE_2D, Master_getShadowMapTexture());

    //glActiveTexture(GL_TEXTURE6);
    //glBindTexture(GL_TEXTURE_2D, Master_getShadowMapTexture2());

    //glActiveTexture(GL_TEXTURE7);
    //glBindTexture(GL_TEXTURE_2D, randomMap);

    if (Global::renderWithCulling)
    {
        Master_enableCulling();
    }
    else
    {
        Master_disableCulling();
    }
}

void prepareTransparentRender()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //new
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(false);

    if (Global::renderWithCulling)
    {
        Master_enableCulling();
    }
    else
    {
        Master_disableCulling();
    }
}

void prepareRenderDepthOnly()
{
    //glBindTexture(GL_TEXTURE_2D, 0);//To make sure the texture isn't bound
    //glBindFramebuffer(GL_FRAMEBUFFER, transparentFrameBuffer);
    //glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    //glDepthMask(true);
    //glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glEnable(GL_DEPTH_TEST);

    if (Global::renderWithCulling)
    {
        Master_enableCulling();
    }
    else
    {
        Master_disableCulling();
    }
}

void Master_cleanUp()
{
    shader->cleanUp();
    delete shader; INCR_DEL("ShaderProgram");
    delete renderer; INCR_DEL("EntityRenderer");
    delete projectionMatrix; INCR_DEL("Matrix4f");

    //shadowMapRenderer->cleanUp();
    //delete shadowMapRenderer; INCR_DEL("ShadowMapMasterRenderer");

    //shadowMapRenderer2->cleanUp();
    //delete shadowMapRenderer2; INCR_DEL("ShadowMapMasterRenderer2");
}

void Master_enableCulling()
{
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void Master_disableCulling()
{
    glDisable(GL_CULL_FACE);
}

void Master_makeProjectionMatrix()
{
    int displayWidth;
    int displayHeight;
    glfwGetWindowSize(getWindow(), &displayWidth, &displayHeight);

    float aspectRatio = (float)displayWidth / (float)displayHeight;


    //FOV = 50;
    float y_scale = 1.0f / tanf(Maths::toRadians((VFOV_BASE+VFOV_ADDITION) / 2.0f));
    float x_scale = y_scale / aspectRatio;


    //FOV = 88.88888;
    //float x_scale = (float)((1.0f / tan(toRadians(HFOV / 2.0f))));
    //float y_scale = x_scale * aspectRatio;


    float frustum_length = FAR_PLANE - NEAR_PLANE;

    projectionMatrix->m00 = x_scale;
    projectionMatrix->m11 = y_scale;
    projectionMatrix->m22 = -((FAR_PLANE + NEAR_PLANE) / frustum_length);
    projectionMatrix->m23 = -1;
    projectionMatrix->m32 = -((2 * NEAR_PLANE * FAR_PLANE) / frustum_length);
    projectionMatrix->m33 = 0;

    renderer->updateProjectionMatrix(projectionMatrix);

    //if (Global::renderParticles)
    {
        //ParticleMaster::updateProjectionMatrix(projectionMatrix);
    }

    //if (Global::useHighQualityWater && Global::gameWaterRenderer != nullptr)
    {
        //Global::gameWaterRenderer->updateProjectionMatrix(projectionMatrix);
    }
}

Matrix4f* Master_getProjectionMatrix()
{
    return projectionMatrix;
}

float Master_getVFOV()
{
    return VFOV_BASE+VFOV_ADDITION;
}

float Master_getNearPlane()
{
    return NEAR_PLANE;
}

float Master_getFarPlane()
{
    return FAR_PLANE;
}

GLuint Master_getShadowMapTexture()
{
    return 0;//shadowMapRenderer->getShadowMap();
}

GLuint Master_getShadowMapTexture2()
{
    return 0;//shadowMapRenderer2->getShadowMap();
}

ShadowMapMasterRenderer* Master_getShadowRenderer()
{
    return shadowMapRenderer;
}

ShadowMapMasterRenderer2* Master_getShadowRenderer2()
{
    return shadowMapRenderer2;
}

void Master_renderShadowMaps(Light* /*sun*/)
{
    //if (Global::renderShadowsFar)
    {
        //shadowMapRenderer->render(&entitiesMap, sun);
    }
    //if (Global::renderShadowsClose)
    {
        //shadowMapRenderer2->render(&entitiesMap, sun);
    }
}
