#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>

#include "vao.hpp"
#include "vbo.hpp"
#include "../toolbox/vector.hpp"
#include "../toolbox/maths.hpp"
#include "../toolbox/matrix.hpp"
#include "../main/main.hpp"

Vao::Vao(GLuint id)
{
    this->id = id;
}

void Vao::bindPrivate()
{
    glBindVertexArray(id);
}

void Vao::unbindPrivate()
{
    glBindVertexArray(0);
}

Vao* Vao::create()
{
    GLuint id;
    glGenVertexArrays(1, &id);

    INCR_NEW("Vao");
    return new Vao(id);
}

int Vao::getIndexCount()
{
    return indexCount;
}

void Vao::bind(std::vector<GLuint>* attributes)
{
    bindPrivate();
    for (GLuint i : *attributes)
    {
        glEnableVertexAttribArray(i);
    }
}

void Vao::unbind(std::vector<GLuint>* attributes)
{
    for (GLuint i : *attributes)
    {
        glDisableVertexAttribArray(i);
    }
    unbindPrivate();
}

void Vao::createIndexBuffer(std::vector<int>* indices)
{
    indexVbo = Vbo::create(GL_ELEMENT_ARRAY_BUFFER);
    indexVbo->bind();
    indexVbo->storeData(indices);
    indexCount = (int)indices->size();
}

void Vao::createAttribute(GLuint attribute, std::vector<float>* data, GLint attrSize)
{
    Vbo* dataVbo = Vbo::create(GL_ARRAY_BUFFER);
    dataVbo->bind();
    dataVbo->storeData(data);
    glVertexAttribPointer(attribute, attrSize, GL_FLOAT, false, attrSize*BYTES_PER_FLOAT, nullptr);
    dataVbo->unbind();
    dataVbos.push_back(dataVbo);
}

void Vao::createIntAttribute(GLuint attribute, std::vector<int>* data, GLint attrSize)
{
    Vbo* dataVbo = Vbo::create(GL_ARRAY_BUFFER);
    dataVbo->bind();
    dataVbo->storeData(data);
    glVertexAttribIPointer(attribute, attrSize, GL_INT, attrSize*BYTES_PER_INT, nullptr);
    dataVbo->unbind();
    dataVbos.push_back(dataVbo);
}

void Vao::deleteMe()
{
    glDeleteVertexArrays(1, &id);

    for (Vbo* vbo : dataVbos)
    {
        vbo->deleteMe();
        delete vbo; INCR_DEL("Vbo");
    }
    dataVbos.clear();

    indexVbo->deleteMe();
    delete indexVbo; INCR_DEL("Vbo");
    indexVbo = nullptr;
}
