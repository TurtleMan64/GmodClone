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
#include "shaderprogram.hpp"

ShaderProgram::ShaderProgram(const char* vertexFile, const char* fragmentFile)
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

void ShaderProgram::start()
{
    glUseProgram(programId);
}

void ShaderProgram::stop()
{
    glUseProgram(0);
}

void ShaderProgram::cleanUp()
{
    stop();
    glDetachShader(programId, vertexShaderId);
    glDetachShader(programId, fragmentShaderId);
    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);
    glDeleteProgram(programId);
}

void ShaderProgram::loadTransformationMatrix(Matrix4f* matrix)
{
    loadMatrix(location_transformationMatrix, matrix);
}

void ShaderProgram::loadProjectionMatrix(Matrix4f* projection)
{
    loadMatrix(location_projectionMatrix, projection);
}

void ShaderProgram::loadViewMatrix(Camera* cam)
{
    Matrix4f viewMatrix;
    Maths::createViewMatrix(&viewMatrix, cam);
    loadMatrix(location_viewMatrix, &viewMatrix);
}

void ShaderProgram::loadLights()
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

void ShaderProgram::loadShineVariables(float damper, float reflectivity)
{
    loadFloat(location_shineDamper, damper);
    loadFloat(location_reflectivity, reflectivity);
}

void ShaderProgram::loadFakeLighting(int fakeLighting)
{
    loadFloat(location_useFakeLighting, (float)fakeLighting);
}

void ShaderProgram::loadTransparency(int transparency)
{
    loadInt(location_hasTransparency, transparency);
}

void ShaderProgram::loadGlowAmount(float glowAmount)
{
    loadFloat(location_glowAmount, glowAmount);
}

void ShaderProgram::loadBaseColor(Vector3f* baseColor)
{
    loadVector(location_baseColor, baseColor);
}

void ShaderProgram::loadBaseAlpha(float baseAlpha)
{
    loadFloat(location_baseAlpha, baseAlpha);
}

void ShaderProgram::loadTextureOffsets(float offX, float offY)
{
    loadFloat(location_texOffX, offX);
    loadFloat(location_texOffY, offY);
}

void ShaderProgram::loadSkyColor(float r, float g, float b)
{
    Vector3f newColor(r, g, b);
    loadVector(location_skyColor, &newColor);
}

void ShaderProgram::loadFogDensity(float density)
{
    loadFloat(location_fogDensity, density);
}

void ShaderProgram::loadFogGradient(float gradient)
{
    loadFloat(location_fogGradient, gradient);
}

void ShaderProgram::loadFogBottomPosition(float position)
{
    loadFloat(location_fogBottomPosition, position);
}

void ShaderProgram::loadFogBottomThickness(float thickness)
{
    loadFloat(location_fogBottomThickness, 1/thickness);
}

void ShaderProgram::bindAttributes()
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
}

void ShaderProgram::bindAttribute(int attribute, const char* variableName)
{
    glBindAttribLocation(programId, attribute, variableName);
}

void ShaderProgram::bindFragOutput(int attatchment, const char* variableName)
{
    glBindFragDataLocation(programId, attatchment, variableName);
}

void ShaderProgram::getAllUniformLocations()
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
}

int ShaderProgram::getUniformLocation(const char* uniformName)
{
    return glGetUniformLocation(programId, uniformName);
}

void ShaderProgram::loadFloat(int location, float value)
{
    glUniform1f(location, value);
}

void ShaderProgram::loadInt(int location, int value)
{
    glUniform1i(location, value);
}

void ShaderProgram::loadUnsignedInt(int location, unsigned int value)
{
    glUniform1ui(location, value);
}

void ShaderProgram::loadVector(int location, Vector3f* vect)
{
    glUniform3f(location, vect->x, vect->y, vect->z);
}

void ShaderProgram::loadBoolean(int location, bool value)
{
    glUniform1i(location, (int)(value));
}

void ShaderProgram::loadMatrix(int location, Matrix4f* matrix)
{
    float matrixBuffer[16];
    matrix->store(matrixBuffer);
    glUniformMatrix4fv(location, 1, GL_FALSE, matrixBuffer);
}

void ShaderProgram::loadClipPlane(float clipX, float clipY, float clipZ, float clipW)
{
    glUniform4f(location_clipPlane, clipX, clipY, clipZ, clipW);
}

void ShaderProgram::loadClipPlaneBehind(float clipX, float clipY, float clipZ, float clipW)
{
    glUniform4f(location_clipPlaneBehind, clipX, clipY, clipZ, clipW);
}

void ShaderProgram::connectTextureUnits()
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
    loadInt(location_depthBufferTransparent, 8);
}

void ShaderProgram::loadToShadowSpaceMatrixFar(Matrix4f* matrix)
{
    loadMatrix(location_toShadowMapSpaceFar, matrix);
}

void ShaderProgram::loadToShadowSpaceMatrixClose(Matrix4f* matrix)
{
    loadMatrix(location_toShadowMapSpaceClose, matrix);
}

void ShaderProgram::loadMixFactor(float factor)
{
    loadFloat(location_mixFactor, factor);
}

void ShaderProgram::loadFogScale(float scale)
{
    loadFloat(location_fogScale, scale);
}

void ShaderProgram::loadIsRenderingTransparent(bool value)
{
    loadInt(location_isRenderingTransparent, (int)value);
}

void ShaderProgram::loadIsRenderingDepth(bool value)
{
    loadInt(location_isRenderingDepth, (int)value);
}

void ShaderProgram::loadClock(unsigned int value)
{
    loadUnsignedInt(location_clock, value);
}

void ShaderProgram::loadEntityId(unsigned int value)
{
    loadUnsignedInt(location_entityId, value);
}

void ShaderProgram::loadNoise(float value)
{
    loadFloat(location_noise, value);
}

void ShaderProgram::loadWaterColor(Vector3f* color)
{
    loadVector(location_waterColor, color);
}

void ShaderProgram::loadWaterBlendAmount(float amount)
{
    loadFloat(location_waterBlendAmount, amount);
}
