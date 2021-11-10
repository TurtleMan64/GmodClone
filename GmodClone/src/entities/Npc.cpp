#include <list>

#include "npc.hpp"
#include "../main/main.hpp"
#include "../loader/objloader.hpp"

std::list<TexturedModel*> Npc::models;

Npc::Npc(std::string name, float x, float y, float z)
{
    this->name = name;
    visible = true;
    position.set(x, y, z);
    updateTransformationMatrix();

    if (Npc::models.size() == 0)
    {
        ObjLoader::loadModel(&Npc::models, "res/Models/Human/", "Human");
    }
}

void Npc::step()
{
    
}

std::list<TexturedModel*>* Npc::getModels()
{
    return &Npc::models;
}

int Npc::getEntityType()
{
    return ENTITY_NPC;
}
