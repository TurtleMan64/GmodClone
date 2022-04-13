#ifndef MODELS_H
#define MODELS_H

#include <glad/glad.h>

#include <vector>

#include "../textures/modeltexture.hpp"

class RawModel
{
private:
    GLuint vaoId;
    int vertexCount;
    std::vector<GLuint> vboIds;

public:
    RawModel();

    RawModel(GLuint vaoId, int vertexCount, std::vector<GLuint>* vboIds);

    GLuint getVaoId();
    void setVaoId(GLuint newId);

    void setVertexCount(int newCount);
    int getVertexCount();

    void deleteMe();

    //for use in textured model constructor only
    std::vector<GLuint>* getVboIds();
};

class TexturedModel
{
private:
    RawModel rawModel;
    ModelTexture texture;

public:
    char renderOrder = 0; //0 = rendered first (default), 1 = second, 2 = third, 3 = fourth + transparent (only render top)

    TexturedModel(RawModel* rawModel, ModelTexture* texture);

    TexturedModel();

    RawModel* getRawModel();

    ModelTexture* getTexture();

    void deleteMe();
};

class Model
{
public:
    std::vector<TexturedModel*> texturedModels;

    Model();

    void deleteMe();

    bool isLoaded();

    void addTexturedModel(TexturedModel* tm);
};

#endif
