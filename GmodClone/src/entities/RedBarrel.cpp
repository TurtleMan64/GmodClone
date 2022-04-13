#include <list>
#include <set>

#include "../toolbox/vector.hpp"
#include "redbarrel.hpp"
#include "../collision/collisionchecker.hpp"
#include "../collision/collisionmodel.hpp"
#include "../collision/triangle3d.hpp"
#include "../main/main.hpp"
#include "../toolbox/maths.hpp"
#include "../loader/objloader.hpp"
#include "../audio/audioplayer.hpp"
#include "../audio/source.hpp"
#include "../entities/entity.hpp"

Model RedBarrel::models;
CollisionModel* RedBarrel::baseCM = nullptr;

extern float dt;

RedBarrel::RedBarrel(std::string name, Vector3f pos)
{
    this->name = name;

    cm = RedBarrel::baseCM->duplicateMe();

    position = pos;
    visible = true;

    entitiesToRender.push_back(this);

    baseCM->transformModelWithScale(cm, &position, 0, scale);

    updateTransformationMatrix();
}

void RedBarrel::step()
{
    if (health <= 0.0f)
    {
        AudioPlayer::play(53, nullptr);
        health = 100.0f;
        visible = false;
    }
}

std::vector<Entity*>* RedBarrel::getEntitiesToRender()
{
    return &entitiesToRender;
}

Model* RedBarrel::getModels()
{
    return &RedBarrel::models;
}

int RedBarrel::getEntityType()
{
    return ENTITY_RED_BARREL;
}

std::vector<Triangle3D*>* RedBarrel::getCollisionTriangles()
{
    if (visible)
    {
        return &cm->triangles;
    }

    return nullptr;
}

void RedBarrel::getHit(Vector3f* /*hitPos*/, Vector3f* /*hitDir*/, int weapon)
{
    switch (weapon)
    {
        case WEAPON_FIST: health -= 0.01f; break;
        case WEAPON_BAT:  health -= 0.2f;  break;
        case WEAPON_GUN:  health -= 0.5f;  break;
        default: break;
    }

    int sound = (int)(Maths::random()*2) + 54;

    AudioPlayer::play(sound, nullptr);
}

void RedBarrel::loadModels()
{
    if (RedBarrel::models.size() == 0)
    {
        ObjLoader::loadModel(&RedBarrel::models, "res/Models/RedBarrel/", "RedBarrel");
        RedBarrel::baseCM = ObjLoader::loadCollisionModel("res/Models/RedBarrel/", "RedBarrel");
    }
}
