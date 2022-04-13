#include <glad/glad.h>

#include <vector>

#include "models.hpp"
#include "../textures/modeltexture.hpp"

Model::Model()
{

}

void Model::deleteMe()
{

}

int Model::size()
{
    return (int)models.size();
}

void Model::push_back(TexturedModel* tm)
{
    models.push_back(tm);
}
