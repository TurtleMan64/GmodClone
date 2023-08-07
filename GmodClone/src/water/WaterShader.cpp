#include <glad/glad.h>

#include <iostream>
#include <fstream>
#include <string>
#include <math.h>

#include "../main/main.hpp"
#include "../toolbox/vector.hpp"
#include "../toolbox/matrix.hpp"
#include "../entities/camera.hpp"
#include "../toolbox/maths.hpp"
#include "../entities/light.hpp"
#include "watershader.hpp"
#include "../renderEngine/renderEngine.hpp"

WaterShader::WaterShader()
{
    vertexShaderId   = Loader::loadShader("res/Shaders/water/WaterVert.glsl", GL_VERTEX_SHADER);
    fragmentShaderId = Loader::loadShader("res/Shaders/water/WaterFrag.glsl", GL_FRAGMENT_SHADER);
    programId = glCreateProgram();
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);
    bindAttributes();
    glLinkProgram(programId);
    glValidateProgram(programId);
    getAllUniformLocations();
}

void WaterShader::start()
{
    glUseProgram(programId);
}

void WaterShader::stop()
{
    glUseProgram(0);
}

void WaterShader::cleanUp()
{
    stop();
    glDetachShader(programId, vertexShaderId);
    glDetachShader(programId, fragmentShaderId);
    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);
    glDeleteProgram(programId);
}

void WaterShader::connectTextureUnits()
{
    loadInt(location_reflectionTexture, 0);
    loadInt(location_refractionTexture, 1);
    loadInt(location_dudvMap, 2);
    loadInt(location_normalMap, 3);
    loadInt(location_depthMap, 4);
}

void WaterShader::loadSun(Light* sun)
{
    //loadVector(location_sunColor,     &sun->color);
    //loadVector(location_sunDirection, &sun->direction);
}

void WaterShader::loadWaterHeight(float waterHeight)
{
    loadFloat(location_waterHeight, waterHeight);
}

void WaterShader::loadWaterMurkyAmount(float murkyAmount)
{
    loadFloat(location_murkiness, murkyAmount);
}

void WaterShader::loadWaterColor(Vector3f* waterColor)
{
    loadVector(location_waterColor, waterColor);
}

void WaterShader::loadMoveFactor(float factor)
{
    loadFloat(location_moveFactor, factor);
}

void WaterShader::loadProjectionMatrix(Matrix4f* projection)
{
    loadMatrix(location_projectionMatrix, projection);
    //loadFloat(location_frustrumFar, 3000.0f);
}

void WaterShader::loadClipPlaneBehind(Vector4f* plane)
{
    loadVector4f(location_clipPlaneBehind, plane);
}

void WaterShader::loadViewMatrix(Camera* cam)
{
    Matrix4f viewMatrix;
    Maths::createViewMatrix(&viewMatrix, cam);
    loadMatrix(location_viewMatrix, &viewMatrix);
    loadVector(location_cameraPosition, &cam->eye);
}

void WaterShader::loadModelMatrix(Matrix4f* modelMatrix)
{
    loadMatrix(location_modelMatrix, modelMatrix);
}

void WaterShader::bindAttributes()
{
    bindFragOutput(0, "out_Color");
    bindFragOutput(1, "out_BrightColor");
    bindAttribute(0, "position");
}

void WaterShader::bindAttribute(int attribute, const char* variableName)
{
    glBindAttribLocation(programId, attribute, variableName);
}

void WaterShader::bindFragOutput(int attatchment, const char* variableName)
{
    glBindFragDataLocation(programId, attatchment, variableName);
}

void WaterShader::getAllUniformLocations()
{
    location_projectionMatrix  = getUniformLocation("projectionMatrix");
    //location_frustrumFar       = getUniformLocation("FAR");
    location_viewMatrix        = getUniformLocation("viewMatrix");
    location_modelMatrix       = getUniformLocation("modelMatrix");
    location_reflectionTexture = getUniformLocation("reflectionTexture");
    location_refractionTexture = getUniformLocation("refractionTexture");
    location_dudvMap           = getUniformLocation("dudvMap");
    location_moveFactor        = getUniformLocation("moveFactor");
    location_cameraPosition    = getUniformLocation("cameraPosition");
    location_normalMap         = getUniformLocation("normalMap");
    location_sunColor          = getUniformLocation("sunColor");
    location_sunDirection      = getUniformLocation("sunDirection");
    location_depthMap          = getUniformLocation("depthMap");
    location_waterHeight       = getUniformLocation("waterHeight");
    location_clipPlaneBehind   = getUniformLocation("clipPlaneBehind");
    location_murkiness         = getUniformLocation("murkiness");
    location_waterColor        = getUniformLocation("waterColor");
}

int WaterShader::getUniformLocation(const char* uniformName)
{
    return glGetUniformLocation(programId, uniformName);
}

void WaterShader::loadInt(int location, int value)
{
    glUniform1i(location, value);
}

void WaterShader::loadFloat(int location, float value)
{
    glUniform1f(location, value);
}

void WaterShader::loadVector(int location, Vector3f* vect)
{
    glUniform3f(location, vect->x, vect->y, vect->z);
}

void WaterShader::loadVector4f(int location, Vector4f* vect)
{
    glUniform4f(location, vect->x, vect->y, vect->z, vect->w);
}

void WaterShader::loadBoolean(int location, float value)
{
    glUniform1f(location, round(value));
}

void WaterShader::loadMatrix(int location, Matrix4f* matrix)
{
    float buf[16];
    matrix->store(buf);
    glUniformMatrix4fv(location, 1, GL_FALSE, buf);
}
