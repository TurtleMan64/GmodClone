#ifndef MODELS_H
#define MODELS_H

#include <glad/glad.h>

#include <list>

#include "../textures/modeltexture.hpp"

class RawModel
{
private:
    GLuint vaoID;
    int vertexCount;
    std::list<GLuint> vboIDs;

public:
    RawModel();

    RawModel(GLuint vaoID, int vertexCount, std::list<GLuint>* vboIDs);

    GLuint getVaoID();
    void setVaoID(GLuint newID);

    void setVertexCount(int newCount);
    int getVertexCount();

    void deleteMe();

    //for use in textured model constructor only
    std::list<GLuint>* getVboIDs();
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
#endif
