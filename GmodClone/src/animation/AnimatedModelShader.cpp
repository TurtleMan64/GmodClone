#include <glad/glad.h>

#include <string>

#include "../renderEngine/renderEngine.hpp"
#include "animatedmodelshader.hpp"
#include "../toolbox/vector.hpp"
#include "../toolbox/maths.hpp"
#include "../toolbox/matrix.hpp"

AnimatedModelShader::AnimatedModelShader()
{
    vertexShaderId   = Loader::loadShader("res/Shaders/animation/animatedEntityVert.glsl", GL_VERTEX_SHADER);
    fragmentShaderId = Loader::loadShader("res/Shaders/animation/animatedEntityFrag.glsl", GL_FRAGMENT_SHADER);
    programId = glCreateProgram();
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);
    bindAttributes();
    glLinkProgram(programId);
    glValidateProgram(programId);
    getAllUniformLocations();

    connectTextureUnits();
}

void AnimatedModelShader::connectTextureUnits()
{
    start();
    loadInt(location_diffuseMap, 0);
    loadInt(location_lightMap, 7);
    stop();
}

void AnimatedModelShader::start()
{
    glUseProgram(programId);
}

void AnimatedModelShader::stop()
{
    glUseProgram(0);
}

void AnimatedModelShader::cleanUp()
{
    stop();
    glDetachShader(programId, vertexShaderId);
    glDetachShader(programId, fragmentShaderId);
    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);
    glDeleteProgram(programId);
}

void AnimatedModelShader::bindAttributes()
{
    bindAttribute(0, "in_position");
    bindAttribute(1, "in_textureCoords");
    bindAttribute(2, "in_normal");
    //bindAttribute(3, "in_vertexColor");
    bindAttribute(3, "in_jointIndices");
    bindAttribute(4, "in_weights");
}

void AnimatedModelShader::bindAttribute(int attribute, const char* variableName)
{
    glBindAttribLocation(programId, attribute, variableName);
}

void AnimatedModelShader::bindFragOutput(int attatchment, const char* variableName)
{
    glBindFragDataLocation(programId, attatchment, variableName);
}

void AnimatedModelShader::getAllUniformLocations()
{
    location_projectionViewMatrix = getUniformLocation("projectionViewMatrix");
    location_lightDirection = getUniformLocation("lightDirection");
    for (int i = 0; i < MAX_JOINTS; i++)
    {
        location_jointTransforms[i] = getUniformLocation(("jointTransforms[" + std::to_string(i) + "]").c_str());
    }
    location_diffuseMap      = getUniformLocation("diffuseMap");
    location_lightMap        = getUniformLocation("lightMap");
    location_lightMapOriginX = getUniformLocation("lightMapOriginX");
    location_lightMapOriginY = getUniformLocation("lightMapOriginY");
    location_lightMapOriginZ = getUniformLocation("lightMapOriginZ");
    location_lightMapSizeX   = getUniformLocation("lightMapSizeX");
    location_lightMapSizeY   = getUniformLocation("lightMapSizeY");
    location_lightMapSizeZ   = getUniformLocation("lightMapSizeZ");
    location_clipPlane       = getUniformLocation("clipPlane");
}

int AnimatedModelShader::getUniformLocation(const char* uniformName)
{
    return glGetUniformLocation(programId, uniformName);
}

void AnimatedModelShader::loadFloat(int location, float value)
{
    glUniform1f(location, value);
}

void AnimatedModelShader::loadInt(int location, int value)
{
    glUniform1i(location, value);
}

void AnimatedModelShader::loadUnsignedInt(int location, unsigned int value)
{
    glUniform1ui(location, value);
}

void AnimatedModelShader::loadVector(int location, Vector3f* vect)
{
    glUniform3f(location, vect->x, vect->y, vect->z);
}

void AnimatedModelShader::loadVector4(int location, Vector4f* vect)
{
    glUniform4f(location, vect->x, vect->y, vect->z, vect->w);
}

void AnimatedModelShader::loadBoolean(int location, bool value)
{
    glUniform1i(location, (int)(value));
}

void AnimatedModelShader::loadMatrix(int location, Matrix4f* matrix)
{
    float matrixBuffer[16];
    matrix->store(matrixBuffer);
    glUniformMatrix4fv(location, 1, GL_FALSE, matrixBuffer);
}
