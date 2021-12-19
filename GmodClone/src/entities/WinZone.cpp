#include <vector>
#include <list>

#include "../toolbox/vector.hpp"
#include "winzone.hpp"
#include "../main/main.hpp"
#include "../entities/entity.hpp"
#include "../loader/objloader.hpp"
#include "../entities/player.hpp"

std::list<TexturedModel*> WinZone::models;

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

std::list<TexturedModel*>* WinZone::getModels()
{
    return &WinZone::models;
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
    if (WinZone::models.size() == 0)
    {
        ObjLoader::loadModel(&WinZone::models, "res/Models/WinZone/", "WinZone");
    }
}
