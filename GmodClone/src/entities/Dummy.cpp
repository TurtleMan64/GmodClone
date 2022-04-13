#include <list>

#include "../toolbox/vector.hpp"
#include "dummy.hpp"

Dummy::Dummy(Model* models)
{
    myModels = models;
    scale = 1;
    visible = true;

    entitiesToRender.push_back(this);
}

void Dummy::step()
{

}

void Dummy::setRotation(float xr, float yr, float zr, float sr)
{
    rotX = xr;
    rotY = yr;
    rotZ = zr;
    rotRoll = sr;
}

std::vector<Entity*>* Dummy::getEntitiesToRender()
{
    return &entitiesToRender;
}

Model* Dummy::getModels()
{
    return myModels;
}

void Dummy::setModels(Model* newModels)
{
    myModels = newModels;
}
