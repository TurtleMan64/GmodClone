#ifndef RENDERENGINE_H
#define RENDERENGINE_H

class ShaderProgram;
class Entity;
class Matrix4f;
class Light;
class Camera;
class ShadowMapMasterRenderer;
class ShadowMapMasterRenderer2;

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <list>
#include <vector>
#include <unordered_map>
#include <string>

#include "../models/models.hpp"

//DisplayManager
int createDisplay();
void updateDisplay();
void closeDisplay();
int displayWantsToClose();
GLFWwindow* getWindow();

struct TextureEntry
{
    GLuint id;
    int count;
};

//Loader
class Loader
{
private:
    static std::list<GLuint> vaos;
    static std::list<GLuint> vbos;
    //static std::list<GLuint> textures;

    static std::unordered_map<std::string, TextureEntry> textures;
    static std::unordered_map<GLuint, std::string> texIdToFilename;

    static int vaoNumber;
    static int vboNumber;
    //static int texNumber;

    static GLuint createVAO();

    static GLuint storeDataInAttributeList(int, int, std::vector<float>*);

    static void unbindVAO();

    static GLuint bindIndiciesBuffer(std::vector<int>*);

public:
    //For 3D Models
    static RawModel loadToVAO(std::vector<float>* positions, 
                              std::vector<float>* textureCoords, 
                              std::vector<float>* normals, 
                              std::vector<float>* vertexColors, 
                              std::vector<int>* indices);

    //For 3D models with normal maps
    static RawModel loadToVAO(std::vector<float>* positions, 
                              std::vector<float>* textureCoords, 
                              std::vector<float>* normals, 
                              std::vector<float>* vertexColors,
                              std::vector<float>* tangents,
                              std::vector<int>* indices);

    //for text
    //returns a std::vector<int> where the first entry is the vao and the rest are vbos
    static std::vector<int> loadToVAO(std::vector<float>* positions, std::vector<float>* textureCoords);

    //for water
    static RawModel loadToVAO(std::vector<float>* positions, int dimensions);

    //for instanced rendering particles
    static GLuint createEmptyVBO(int floatCount);

    static void addInstancedAttribute(GLuint vao, GLuint vbo, int attribute, int dataSize, int instancedDataLength, int offset);

    static void updateVBO(GLuint vbo, int bufferNumFloats, std::vector<float>* buffer);

    //Loads a texture into GPU memory, returns the GLuint id
    static GLuint loadTexture(const char* filename);

    //Loads a texture without any interpolation
    static GLuint loadTextureNoInterpolation(const char* fileName);

    static void cleanUp();

    static void deleteVAO(GLuint vaoId);

    static void deleteVBO(GLuint vboId);

    static void deleteTexture(GLuint texId);

    static void deleteTexturedModels(std::list<TexturedModel*>* tm);

    static void printInfo();

    static GLuint loadShader(const char* file, int shaderType);
};

//Master Renderer
void Master_render(Camera* camera, float clipX, float clipY, float clipZ, float clipW, float waterBlendAmount);

void Master_cleanUp();

void Master_processEntity(Entity* entity);
//void Master_processEntityPass2(Entity* entity);
//void Master_processEntityPass3(Entity* entity);
//void Master_processTransparentEntity(Entity* entity);

void Master_clearAllEntities();
//void Master_clearEntitiesPass2();
//void Master_clearEntitiesPass3();
//void Master_clearTransparentEntities();

void Master_init();

void Master_enableCulling();

void Master_disableCulling();

void Master_makeProjectionMatrix();

Matrix4f* Master_getProjectionMatrix();

float Master_getVFOV();

float Master_getNearPlane();

float Master_getFarPlane();

GLuint Master_getShadowMapTexture();

ShadowMapMasterRenderer* Master_getShadowRenderer();

GLuint Master_getShadowMapTexture2();

ShadowMapMasterRenderer2* Master_getShadowRenderer2();

void Master_renderShadowMaps(Light* sun);

//Renderer
class EntityRenderer
{
private:
    float clockTime;

    ShaderProgram* shader;

    void prepareTexturedModel(TexturedModel* model);

    void unbindTexturedModel();

    void prepareInstance(Entity* entity);

public:
    EntityRenderer(ShaderProgram* shader, Matrix4f* projectionMatrix);

    void render(Entity*);

    void renderNEW(std::unordered_map<TexturedModel*, std::list<Entity*>>* entities, Matrix4f* toShadowSpaceFar, Matrix4f* toShadowSpaceClose);

    void updateProjectionMatrix(Matrix4f* projectionMatrix);

};
#endif
