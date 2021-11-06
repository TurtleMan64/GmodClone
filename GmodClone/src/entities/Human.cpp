#include <list>

#include "human.h"
#include "../main/main.h"
#include "../loader/objloader.h"

std::list<TexturedModel*> Human::models;

Human::Human(float x, float y, float z)
{
    visible = true;
    position.set(x, y, z);
    updateTransformationMatrix();

    if (Human::models.size() == 0)
    {
        ObjLoader::loadModel(&Human::models, "res/Models/Human/", "Human");
    }
}

void Human::step()
{
    
}

std::list<TexturedModel*>* Human::getModels()
{
    return &Human::models;
}
