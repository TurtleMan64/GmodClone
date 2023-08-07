#include <glad/glad.h>

#include <iostream>
#include <fstream>
#include <string>
#include <math.h>

#include "../toolbox/vector.hpp"
#include "../toolbox/matrix.hpp"
#include "../entities/camera.hpp"
#include "../toolbox/maths.hpp"
#include "../entities/light.hpp"
#include "../renderEngine/renderEngine.hpp"
#include "../main/main.hpp"
#include "stageshader.hpp"

StageShader::StageShader(const char* vertexFile, const char* fragmentFile)
{
    vertexShaderId = Loader::loadShader(vertexFile, GL_VERTEX_SHADER);
    fragmentShaderId = Loader::loadShader(fragmentFile, GL_FRAGMENT_SHADER);
    programId = glCreateProgram();
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);
    bindAttributes();
    glLinkProgram(programId);
    glValidateProgram(programId);
    getAllUniformLocations();
}

void StageShader::start()
{
    glUseProgram(programId);
}

void StageShader::stop()
{
    glUseProgram(0);
}

void StageShader::cleanUp()
{
    stop();
    glDetachShader(programId, vertexShaderId);
    glDetachShader(programId, fragmentShaderId);
    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);
    glDeleteProgram(programId);
}

void StageShader::loadTransformationMatrix(Matrix4f* matrix)
{
    loadMatrix(location_transformationMatrix, matrix);
}

void StageShader::loadProjectionMatrix(Matrix4f* projection)
{
    loadMatrix(location_projectionMatrix, projection);
}

void StageShader::loadViewMatrix(Camera* cam)
{
    Matrix4f viewMatrix;
    Maths::createViewMatrix(&viewMatrix, cam);
    loadMatrix(location_viewMatrix, &viewMatrix);
}

void StageShader::loadLights()
{
    //todo can probably not do this every frame. at least parts of it
    Matrix4f viewMatrix;
    Maths::createViewMatrix(&viewMatrix, Global::gameCamera);

    // The sun is extremely far away
    Vector3f position = Global::lights[0]->direction.scaleCopy(-1000000000.0f);
    Vector4f eyeSpacePos1(position.x,position.y, position.z, 1.0f);
    Vector4f eyeSpacePos2 = viewMatrix.transform(&eyeSpacePos1);
    Vector3f eyeSpacePos3(eyeSpacePos2.x, eyeSpacePos2.y, eyeSpacePos2.z);

    loadVector(location_lightPositionEyeSpace[0], &eyeSpacePos3);
    loadVector(location_lightColor[0], &Global::lights[0]->color);
    loadVector(location_attenuation[0], &Global::lights[0]->attenuation);

    // The other 3 lights are point lights
    for (int i = 1; i < 4; i++)
    {
        position = Global::lights[i]->position;
        eyeSpacePos1.set(position.x,position.y, position.z, 1.0f);
        eyeSpacePos2 = viewMatrix.transform(&eyeSpacePos1);
        eyeSpacePos3.set(eyeSpacePos2.x, eyeSpacePos2.y, eyeSpacePos2.z);

        loadVector(location_lightPositionEyeSpace[i], &eyeSpacePos3);
        loadVector(location_lightColor[i], &Global::lights[i]->color);
        loadVector(location_attenuation[i], &Global::lights[i]->attenuation);
    }
}

void StageShader::loadShineVariables(float damper, float reflectivity)
{
    loadFloat(location_shineDamper, damper);
    loadFloat(location_reflectivity, reflectivity);
}

void StageShader::loadFakeLighting(int fakeLighting)
{
    loadFloat(location_useFakeLighting, (float)fakeLighting);
}

void StageShader::loadTransparency(int transparency)
{
    loadInt(location_hasTransparency, transparency);
}

void StageShader::loadGlowAmount(float glowAmount)
{
    loadFloat(location_glowAmount, glowAmount);
}

void StageShader::loadBaseColor(Vector3f* baseColor)
{
    loadVector(location_baseColor, baseColor);
}

void StageShader::loadBaseAlpha(float baseAlpha)
{
    loadFloat(location_baseAlpha, baseAlpha);
}

void StageShader::loadTextureOffsets(float offX, float offY)
{
    loadFloat(location_texOffX, offX);
    loadFloat(location_texOffY, offY);
}

void StageShader::loadSkyColor(float r, float g, float b)
{
    Vector3f newColor(r, g, b);
    loadVector(location_skyColor, &newColor);
}

void StageShader::loadFogDensity(float density)
{
    loadFloat(location_fogDensity, density);
}

void StageShader::loadFogGradient(float gradient)
{
    loadFloat(location_fogGradient, gradient);
}

void StageShader::loadFogBottomPosition(float position)
{
    loadFloat(location_fogBottomPosition, position);
}

void StageShader::loadFogBottomThickness(float thickness)
{
    loadFloat(location_fogBottomThickness, 1/thickness);
}

void StageShader::bindAttributes()
{
    //if (Global::renderBloom)
    {
        //bindFragOutput(0, "out_Color");
        //bindFragOutput(1, "out_BrightColor");
    }

    bindAttribute(0, "position");
    bindAttribute(1, "textureCoords");
    bindAttribute(2, "normal");
    bindAttribute(3, "vertexColor");
    bindAttribute(4, "tangent");
    bindAttribute(5, "textureCoordsLightmap");
}

void StageShader::bindAttribute(int attribute, const char* variableName)
{
    glBindAttribLocation(programId, attribute, variableName);
}

void StageShader::bindFragOutput(int attatchment, const char* variableName)
{
    glBindFragDataLocation(programId, attatchment, variableName);
}

void StageShader::getAllUniformLocations()
{
    location_transformationMatrix     = getUniformLocation("transformationMatrix");
    location_projectionMatrix         = getUniformLocation("projectionMatrix");
    location_viewMatrix               = getUniformLocation("viewMatrix");
    location_shineDamper              = getUniformLocation("shineDamper");
    location_reflectivity             = getUniformLocation("reflectivity");
    location_useFakeLighting          = getUniformLocation("useFakeLighting");
    location_hasTransparency          = getUniformLocation("hasTransparency");
    location_glowAmount               = getUniformLocation("glowAmount");
    location_baseColor                = getUniformLocation("baseColor");
    location_baseAlpha                = getUniformLocation("baseAlpha");
    location_texOffX                  = getUniformLocation("texOffX");
    location_texOffY                  = getUniformLocation("texOffY");
    location_skyColor                 = getUniformLocation("skyColor");
    location_fogDensity               = getUniformLocation("fogDensity");
    location_fogGradient              = getUniformLocation("fogGradient");
    location_clipPlane                = getUniformLocation("clipPlane");
    location_clipPlaneBehind          = getUniformLocation("clipPlaneBehind");
    location_shadowMapFar             = getUniformLocation("shadowMapFar");
    location_toShadowMapSpaceFar      = getUniformLocation("toShadowMapSpaceFar");
    location_shadowMapClose           = getUniformLocation("shadowMapClose");
    location_toShadowMapSpaceClose    = getUniformLocation("toShadowMapSpaceClose");
    location_randomMap                = getUniformLocation("randomMap");
    location_mixFactor                = getUniformLocation("mixFactor");
    location_textureSampler2          = getUniformLocation("textureSampler2");
    location_fogScale                 = getUniformLocation("fogScale");
    location_fogBottomPosition        = getUniformLocation("fogBottomPosition");
    location_fogBottomThickness       = getUniformLocation("fogBottomThickness");
    location_depthBufferTransparent   = getUniformLocation("depthBufferTransparent");
    location_isRenderingTransparent   = getUniformLocation("isRenderingTransparent");
    location_waterColor               = getUniformLocation("waterColor");
    location_waterBlendAmount         = getUniformLocation("waterBlendAmount");
    location_waterMurkyAmount         = getUniformLocation("waterMurkyAmount");
    location_isRenderingDepth         = getUniformLocation("isRenderingDepth");
    location_normalMap                = getUniformLocation("normalMap");
    location_lightPositionEyeSpace[0] = getUniformLocation("lightPositionEyeSpace[0]");
    location_lightPositionEyeSpace[1] = getUniformLocation("lightPositionEyeSpace[1]");
    location_lightPositionEyeSpace[2] = getUniformLocation("lightPositionEyeSpace[2]");
    location_lightPositionEyeSpace[3] = getUniformLocation("lightPositionEyeSpace[3]");
    location_attenuation[0]           = getUniformLocation("attenuation[0]");
    location_attenuation[1]           = getUniformLocation("attenuation[1]");
    location_attenuation[2]           = getUniformLocation("attenuation[2]");
    location_attenuation[3]           = getUniformLocation("attenuation[3]");
    location_lightColor[0]            = getUniformLocation("lightColor[0]");
    location_lightColor[1]            = getUniformLocation("lightColor[1]");
    location_lightColor[2]            = getUniformLocation("lightColor[2]");
    location_lightColor[3]            = getUniformLocation("lightColor[3]");
    location_clock                    = getUniformLocation("clock");
    location_noise                    = getUniformLocation("noise");
    location_entityId                 = getUniformLocation("entityId");
    location_lightMap                 = getUniformLocation("lightMap");
}

int StageShader::getUniformLocation(const char* uniformName)
{
    return glGetUniformLocation(programId, uniformName);
}

void StageShader::loadFloat(int location, float value)
{
    glUniform1f(location, value);
}

void StageShader::loadInt(int location, int value)
{
    glUniform1i(location, value);
}

void StageShader::loadUnsignedInt(int location, unsigned int value)
{
    glUniform1ui(location, value);
}

void StageShader::loadVector(int location, Vector3f* vect)
{
    glUniform3f(location, vect->x, vect->y, vect->z);
}

void StageShader::loadBoolean(int location, bool value)
{
    glUniform1i(location, (int)(value));
}

void StageShader::loadMatrix(int location, Matrix4f* matrix)
{
    float matrixBuffer[16];
    matrix->store(matrixBuffer);
    glUniformMatrix4fv(location, 1, GL_FALSE, matrixBuffer);
}

void StageShader::loadClipPlane(float clipX, float clipY, float clipZ, float clipW)
{
    glUniform4f(location_clipPlane, clipX, clipY, clipZ, clipW);
}

void StageShader::loadClipPlaneBehind(float clipX, float clipY, float clipZ, float clipW)
{
    glUniform4f(location_clipPlaneBehind, clipX, clipY, clipZ, clipW);
}

void StageShader::connectTextureUnits()
{
    //if (Global::renderShadowsFar || Global::renderShadowsClose)
    {
        //if (Global::renderShadowsFar)
        {
            //loadInt(location_shadowMapFar, 5);
        }
        //if (Global::renderShadowsClose)
        {
            //loadInt(location_shadowMapClose, 6);
        }

        //loadInt(location_randomMap, 7);
    }

    loadInt(location_textureSampler2, 1);
    loadInt(location_normalMap, 2);
    loadInt(location_randomMap, 3);
    loadInt(location_lightMap,  7);
    loadInt(location_depthBufferTransparent, 8);
}

void StageShader::loadToShadowSpaceMatrixFar(Matrix4f* matrix)
{
    loadMatrix(location_toShadowMapSpaceFar, matrix);
}

void StageShader::loadToShadowSpaceMatrixClose(Matrix4f* matrix)
{
    loadMatrix(location_toShadowMapSpaceClose, matrix);
}

void StageShader::loadMixFactor(float factor)
{
    loadFloat(location_mixFactor, factor);
}

void StageShader::loadFogScale(float scale)
{
    loadFloat(location_fogScale, scale);
}

void StageShader::loadIsRenderingTransparent(bool value)
{
    loadInt(location_isRenderingTransparent, (int)value);
}

void StageShader::loadIsRenderingDepth(bool value)
{
    loadInt(location_isRenderingDepth, (int)value);
}

void StageShader::loadClock(unsigned int value)
{
    loadUnsignedInt(location_clock, value);
}

void StageShader::loadEntityId(unsigned int value)
{
    loadUnsignedInt(location_entityId, value);
}

void StageShader::loadNoise(float value)
{
    loadFloat(location_noise, value);
}

void StageShader::loadWaterColor(Vector3f* color)
{
    loadVector(location_waterColor, color);
}

void StageShader::loadWaterBlendAmount(float amount)
{
    loadFloat(location_waterBlendAmount, amount);
}
