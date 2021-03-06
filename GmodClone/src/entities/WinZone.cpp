#include <vector>
#include <list>

#include "../toolbox/vector.hpp"
#include "winzone.hpp"
#include "../main/main.hpp"
#include "../entities/entity.hpp"
#include "../loader/objloader.hpp"
#include "../entities/player.hpp"

Model WinZone::model;

extern float dt;

WinZone::WinZone(std::string name, Vector3f pos, Vector3f size)
{
    this->name = name;

    position = pos;
    this->size = size;
    visible = true;

    entitiesToRender.push_back(this);

    updateTransformationMatrix(size.x, size.y, size.z);
}

WinZone::~WinZone()
{

}

void WinZone::step()
{
    Vector3f diff = Global::player->position - position;
    if (fabsf(diff.x) < size.x &&
        fabsf(diff.y) < size.y &&
        fabsf(diff.z) < size.z)
    {
        Global::player->inZoneTime += dt;
    }
}

Model* WinZone::getModel()
{
    return &WinZone::model;
}

std::vector<Entity*>* WinZone::getEntitiesToRender()
{
    return &entitiesToRender;
}

int WinZone::getEntityType()
{
    return ENTITY_WIN_ZONE;
}

void WinZone::loadModels()
{
    if (!WinZone::model.isLoaded())
    {
        ObjLoader::loadModel(&WinZone::model, "res/Models/WinZone/", "WinZone");
    }
}
