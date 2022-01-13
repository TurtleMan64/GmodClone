#include <string>

#include "vbo.hpp"
#include "../toolbox/vector.hpp"
#include "../toolbox/maths.hpp"
#include "../toolbox/matrix.hpp"
#include "../main/main.hpp"

Vbo::Vbo(GLuint vboId, GLuint type)
{
    this->vboId = vboId;
    this->type = type;
}

Vbo* Vbo::create(GLuint type)
{
    GLuint id;
    glGenBuffers(1, &id);

    INCR_NEW("Vbo");
    return new Vbo(id, type);
}

void Vbo::bind()
{
    glBindBuffer(type, vboId);
}

void Vbo::unbind()
{
    glBindBuffer(type, 0);
}

void Vbo::storeData(std::vector<float>* data)
{
    glBufferData(type, data->size()*sizeof(float), (GLvoid*)(&((*data)[0])), GL_STATIC_DRAW);
}

void Vbo::storeData(std::vector<int>* data)
{
    glBufferData(type, data->size()*sizeof(int), (GLvoid*)(&((*data)[0])), GL_STATIC_DRAW);

}

void Vbo::deleteMe()
{
    glDeleteBuffers(1, &vboId);
    vboId = GL_NONE;
}
