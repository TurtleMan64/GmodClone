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
#include "../entities/player.hpp"

std::list<TexturedModel*> RedBarrel::models;
CollisionModel* RedBarrel::baseCM = nullptr;

extern float dt;

RedBarrel::RedBarrel(std::string name, Vector3f pos)
{
    this->name = name;

    cm = RedBarrel::baseCM->duplicateMe();

    position = pos;
    visible = true;

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

std::list<TexturedModel*>* RedBarrel::getModels()
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
    if (weapon == 0)
    {
        health -= 0.1f;
    }
    else
    {
        health -= 0.6f;
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
