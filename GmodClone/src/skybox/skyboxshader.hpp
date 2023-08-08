#ifndef STAGESHADER_H
#define STAGESHADER_H

class Matrix4f;
class Camera;
class Light;
class Vector3f;

#include <glad/glad.h>

class SkyboxShader
{
private:
    GLuint programId;
    GLuint vertexShaderId;
    GLuint fragmentShaderId;

    int location_projectionMatrix;
    int location_viewMatrix;
    int location_cubeMap;
    int location_clipPlane;
    int location_center;

public:
    SkyboxShader(const char*, const char*);

    void start();

    void stop();

    void cleanUp();

    void loadProjectionMatrix(Matrix4f*);

    void loadViewMatrix(Camera* camera);

    void loadClipPlane(float clipX, float clipY, float clipZ, float clipW);

    void loadCenter(Vector3f* center);

    void connectTextureUnits();

protected:
    void bindAttributes();

    void bindAttribute(int, const char*);

    void bindFragOutput(int attatchment, const char* variableName);

    void getAllUniformLocations();

    int getUniformLocation(const char*);

    void loadInt(int, int);

    void loadMatrix(int, Matrix4f*);
};

#endif
