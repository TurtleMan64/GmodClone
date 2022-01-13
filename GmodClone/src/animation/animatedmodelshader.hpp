#ifndef ANIMATED_MODEL_SHADER_H
#define ANIMATED_MODEL_SHADER_H

#include <glad/glad.h>

class Vector3f;
class Matrix4f;

class AnimatedModelShader
{
public:
    static constexpr int MAX_JOINTS = 20;
    static constexpr int DIFFUSE_TEX_UNIT = 0;

    GLuint programId;
    GLuint vertexShaderId;
    GLuint fragmentShaderId;

    int location_projectionViewMatrix;
    int location_lightDirection;
    int location_jointTransforms[MAX_JOINTS];
    int location_diffuseMap;

    AnimatedModelShader();

    void connectTextureUnits();

    void start();

    void stop();

    void cleanUp();

    void bindAttributes();

    void bindAttribute(int, const char*);

    void bindFragOutput(int attatchment, const char* variableName);

    void getAllUniformLocations();

    int getUniformLocation(const char*);

    void loadFloat(int, float);

    void loadInt(int, int);

    void loadUnsignedInt(int, unsigned int);

    void loadVector(int, Vector3f*);

    void loadBoolean(int, bool);

    void loadMatrix(int, Matrix4f*);

};
#endif
