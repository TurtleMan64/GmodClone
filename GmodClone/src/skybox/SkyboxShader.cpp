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
#include "skyboxshader.hpp"

SkyboxShader::SkyboxShader(const char* vertexFile, const char* fragmentFile)
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

void SkyboxShader::start()
{
    glUseProgram(programId);
}

void SkyboxShader::stop()
{
    glUseProgram(0);
}

void SkyboxShader::cleanUp()
{
    stop();
    glDetachShader(programId, vertexShaderId);
    glDetachShader(programId, fragmentShaderId);
    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);
    glDeleteProgram(programId);
}

void SkyboxShader::loadProjectionMatrix(Matrix4f* projection)
{
    loadMatrix(location_projectionMatrix, projection);
}

void SkyboxShader::loadViewMatrix(Camera* cam)
{
    Matrix4f viewMatrix;
    Maths::createViewMatrix(&viewMatrix, cam);
    loadMatrix(location_viewMatrix, &viewMatrix);
}

void SkyboxShader::loadClipPlane(float clipX, float clipY, float clipZ, float clipW)
{
    glUniform4f(location_clipPlane, clipX, clipY, clipZ, clipW);
}

void SkyboxShader::loadCenter(Vector3f* center)
{
    glUniform3f(location_center, center->x, center->y, center->z);
}

void SkyboxShader::bindAttributes()
{
    bindAttribute(0, "position");
}

void SkyboxShader::bindAttribute(int attribute, const char* variableName)
{
    glBindAttribLocation(programId, attribute, variableName);
}

void SkyboxShader::bindFragOutput(int attatchment, const char* variableName)
{
    glBindFragDataLocation(programId, attatchment, variableName);
}

void SkyboxShader::getAllUniformLocations()
{
    location_projectionMatrix = getUniformLocation("projectionMatrix");
    location_viewMatrix       = getUniformLocation("viewMatrix");
    location_clipPlane        = getUniformLocation("clipPlane");
    location_cubeMap          = getUniformLocation("cubeMap");
    location_center           = getUniformLocation("center");
}

int SkyboxShader::getUniformLocation(const char* uniformName)
{
    return glGetUniformLocation(programId, uniformName);
}

void SkyboxShader::loadInt(int location, int value)
{
    glUniform1i(location, value);
}

void SkyboxShader::loadMatrix(int location, Matrix4f* matrix)
{
    float matrixBuffer[16];
    matrix->store(matrixBuffer);
    glUniformMatrix4fv(location, 1, GL_FALSE, matrixBuffer);
}

void SkyboxShader::connectTextureUnits()
{
    loadInt(location_cubeMap, 0);
}
