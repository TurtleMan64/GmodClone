#ifndef WATERRENDERER_H
#define WATERRENDERER_H

class WaterShader;
class WaterFrameBuffers;
class Matrix4f;
class RawModel;
class Camera;
class WaterTile;
class Camera;
class Light;

#include <list>
#include <vector>
#include <glad/glad.h>

class WaterRenderer
{
private:
    float moveFactor = 1.0f;
    GLuint dudvTexture = GL_NONE;
    GLuint normalMap = GL_NONE;

    RawModel* quad = nullptr;
    WaterShader* shader = nullptr;
    WaterFrameBuffers* fbos = nullptr;

    void prepareRender(Camera* camera, Light* sun);

    void unbind();

    void setUpVAO();

public:
    static constexpr float WAVE_SPEED = 0.036f;

    WaterRenderer(WaterShader* shader, Matrix4f* projectionMatrix, WaterFrameBuffers* fbos);

    void render(std::vector<WaterTile*>* water, Camera* camera, Light* sun);

    void updateProjectionMatrix(Matrix4f* projectionMatrix);
};
#endif
