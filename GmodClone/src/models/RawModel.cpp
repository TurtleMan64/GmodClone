#include <glad/glad.h>

#include <vector>

#include "models.hpp"
#include "../renderEngine/renderEngine.hpp"

RawModel::RawModel()
{

}

RawModel::RawModel(GLuint vaoId, int vertexCount, std::vector<GLuint>* vboIds)
{
    this->vaoId = vaoId;
    this->vertexCount = vertexCount;

    for (auto id : (*vboIds))
    {
        this->vboIds.push_back(id);
    }
}

GLuint RawModel::getVaoId()
{
    return vaoId;
}

void RawModel::setVaoId(GLuint newId)
{
    this->vaoId = newId;
}

int RawModel::getVertexCount()
{
    return vertexCount;
}

void RawModel::setVertexCount(int newCount)
{
    this->vertexCount = newCount;
}

std::vector<GLuint>* RawModel::getVboIds()
{
    return &vboIds;
}

void RawModel::deleteMe()
{
    Loader::deleteVAO(vaoId);
    for (auto vbo : vboIds)
    {
        Loader::deleteVBO(vbo);
    }
    vboIds.clear();
}
