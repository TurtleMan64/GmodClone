#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

class Matrix4f;
class Camera;
class Light;
class Vector3f;

#include <glad/glad.h>


class ShaderProgram
{
private:
    GLuint programId;
    GLuint vertexShaderId;
    GLuint fragmentShaderId;

    int location_transformationMatrix;
    int location_projectionMatrix;
    int location_viewMatrix;
    int location_shineDamper;
    int location_reflectivity;
    int location_useFakeLighting;
    int location_hasTransparency;
    int location_baseColor;
    int location_baseAlpha;
    int location_skyColor;
    int location_texOffX;
    int location_texOffY;
    int location_glowAmount;
    int location_fogDensity;
    int location_fogGradient;
    int location_clipPlane;
    int location_clipPlaneBehind;
    int location_shadowMapFar;
    int location_toShadowMapSpaceFar;
    int location_shadowMapClose;
    int location_toShadowMapSpaceClose;
    int location_randomMap;
    int location_mixFactor;
    int location_textureSampler2;
    int location_fogScale;
    int location_fogBottomPosition;
    int location_fogBottomThickness;
    int location_depthBufferTransparent;
    int location_isRenderingTransparent;
    int location_waterColor;
    int location_waterBlendAmount;
    int location_waterMurkyAmount;
    int location_isRenderingDepth;
    int location_normalMap;
    int location_lightPositionEyeSpace[4];
    int location_attenuation[4];
    int location_lightColor[4];
    int location_clock;
    int location_noise;
    int location_entityId;
    int location_lightMap;
    int location_lightMapOriginX;
    int location_lightMapOriginY;
    int location_lightMapOriginZ;
    int location_lightMapSizeX;
    int location_lightMapSizeY;
    int location_lightMapSizeZ;

public:
    ShaderProgram(const char*, const char*);

    void start();

    void stop();

    void cleanUp();

    void loadTransformationMatrix(Matrix4f*);

    void loadProjectionMatrix(Matrix4f*);

    void loadViewMatrix(Camera* camera);

    void loadLights();

    void loadShineVariables(float damper, float reflectivity);

    void loadFakeLighting(int fakeLighting);

    void loadTransparency(int transparency);

    void loadGlowAmount(float glowAmount);

    void loadBaseColor(Vector3f* baseColor);

    void loadBaseAlpha(float baseAlpha);

    void loadTextureOffsets(float offX, float offY);

    void loadSkyColor(float r, float g, float b);

    void loadFogDensity(float density);

    void loadFogGradient(float gradient);

    void loadFogBottomPosition(float position);

    void loadFogBottomThickness(float thickness);

    void loadClipPlane(float clipX, float clipY, float clipZ, float clipW);

    void loadClipPlaneBehind(float clipX, float clipY, float clipZ, float clipW);

    void loadToShadowSpaceMatrixFar(Matrix4f* matrix);

    void loadToShadowSpaceMatrixClose(Matrix4f* matrix);

    void loadMixFactor(float factor);

    void loadFogScale(float scale);

    void loadIsRenderingTransparent(bool value);

    void loadWaterColor(Vector3f* color);

    void loadWaterBlendAmount(float);

    void loadIsRenderingDepth(bool value);

    void loadClock(unsigned int value);

    void loadEntityId(unsigned int value);

    void loadNoise(float value);

    void loadLightMapData(float x, float y, float z, float width, float height, float depth);

    void connectTextureUnits();

protected:
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
