#include <glad/glad.h>

#include <vector>

#include "../main/main.hpp"
#include "models.hpp"
#include "../textures/modeltexture.hpp"

Model::Model()
{

}

void Model::deleteMe()
{
    for (TexturedModel* model : texturedModels)
    {
        model->deleteMe();
        delete model; INCR_DEL("TexturedModel");
    }
    texturedModels.clear();
}

bool Model::isLoaded()
{
    return (int)texturedModels.size() > 0;
}

void Model::addTexturedModel(TexturedModel* tm)
{
    texturedModels.push_back(tm);
}


LightModel::LightModel()
{

}

void LightModel::deleteMe()
{
    for (TexturedModel* model : texturedModels)
    {
        model->deleteMe();
        delete model; INCR_DEL("TexturedModel");
    }
    texturedModels.clear();
}

bool LightModel::isLoaded()
{
    return (int)texturedModels.size() > 0;
}

void LightModel::addTexturedModel(TexturedModel* tm)
{
    texturedModels.push_back(tm);
}
