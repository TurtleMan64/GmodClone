#include <list>
#include <set>

#include "../toolbox/vector.hpp"
#include "collisionblock.hpp"
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
#include "../entities/onlineplayer.hpp"

std::list<TexturedModel*> CollisionBlock::models;
CollisionModel* CollisionBlock::baseCM = nullptr;

extern float dt;

CollisionBlock::CollisionBlock(std::string name, Vector3f pos, int direction, float radius, float timePeriod, float distance, bool sinusoidal, float timeOffset)
{
    this->name = name;
    this->direction = direction;
    this->scale = radius;
    this->timePeriod = timePeriod;
    this->distance = distance;
    this->sinusoidal = sinusoidal;
    this->timeOffset = timeOffset;

    cm = CollisionBlock::baseCM->duplicateMe();

    position = pos;
    startPos = pos;
    visible = true;

    entitiesToRender.push_back(this);
}

CollisionBlock::~CollisionBlock()
{
    if (cm != nullptr)
    {
        cm->deleteMe();
        delete cm; INCR_DEL("CollisionModel");
        cm = nullptr;
    }
}

float prevPosX = 0.0f;

void CollisionBlock::step()
{
    float syncedTime = (((float)Global::syncedGlobalTime) - timeOffset)/timePeriod;

    const float magicNumber = 0.636612f; //no idea where this number comes from
    float off;
    float spd;
    if (sinusoidal)
    {
        off = distance*sinf(syncedTime);
        spd = (distance/timePeriod)*cosf(syncedTime);
    }
    else
    {
        float t = (syncedTime + Maths::PI/2)/(2*Maths::PI);
        float p = fmodf(t, 1.0f);
        if (p < 0.5f)
        {
            off = (-2*distance) + 4*distance*p + distance;
            spd = (distance/timePeriod)*magicNumber;
        }
        else
        {
            off =  (2*distance) - 4*distance*p + distance;
            spd = -(distance/timePeriod)*magicNumber;
        }
    }

    if (direction == 0)
    {
        position.x = startPos.x + off;
    }
    else if (direction == 1)
    {
        position.y = startPos.y + off;
    }
    else if (direction == 2)
    {
        position.z = startPos.z + off;
    }

    const float eps = scale + 0.01f;

    for (Entity* e : Global::gameEntities)
    {
        if (e->getEntityType() == ENTITY_ONLINE_PLAYER)
        {
            OnlinePlayer* onlinePlayer = (OnlinePlayer*)e;
            if (direction == 0)
            {
                if (onlinePlayer->collideEntityImTouching == this)
                {
                    if (onlinePlayer->position.y >= (position.y + scale) - eps)
                    {
                        onlinePlayer->externalVel.set(spd, 0, 0);
                    }
                    else if (onlinePlayer->position.x >= (position.x + scale) - eps && spd > 0)
                    {
                        onlinePlayer->externalVel.set(spd, 0, 0);
                    }
                    else if (onlinePlayer->position.x <= (position.x - scale) + eps && spd < 0)
                    {
                        onlinePlayer->externalVel.set(spd, 0, 0);
                    }
                }
            }
            else if (direction == 1)
            {
                if (onlinePlayer->collideEntityImTouching == this && 
                    onlinePlayer->position.y >= (position.y + scale) - scale)
                {
                    onlinePlayer->externalVel.set(0, spd, 0);
                }
            }
            else if (direction == 2)
            {
                if (onlinePlayer->collideEntityImTouching == this)
                {
                    if (onlinePlayer->position.y >= (position.y + scale) - eps)
                    {
                        onlinePlayer->externalVel.set(0, 0, spd);
                    }
                    else if (onlinePlayer->position.z >= (position.z + scale) - eps && spd > 0)
                    {
                        onlinePlayer->externalVel.set(0, 0, spd);
                    }
                    else if (onlinePlayer->position.z <= (position.z - scale) + eps && spd < 0)
                    {
                        onlinePlayer->externalVel.set(0, 0, spd);
                    }
                }
            }
        }
    }

    if (direction == 0)
    {
        if (Global::player->collideEntityImTouching == this)
        {
            if (Global::player->position.y >= (position.y + scale) - eps)
            {
                Global::player->externalVel.set(spd, 0, 0);
            }
            else if (Global::player->position.x >= (position.x + scale) - eps && spd > 0)
            {
                Global::player->externalVel.set(spd, 0, 0);
            }
            else if (Global::player->position.x <= (position.x - scale) + eps && spd < 0)
            {
                Global::player->externalVel.set(spd, 0, 0);
            }
        }
    }
    else if (direction == 1)
    {
        if (Global::player->collideEntityImTouching == this && 
            Global::player->position.y >= (position.y + scale) - scale)
        {
            Global::player->externalVel.set(0, spd, 0);
        }
    }
    else if (direction == 2)
    {
        if (Global::player->collideEntityImTouching == this)
        {
            if (Global::player->position.y >= (position.y + scale) - eps)
            {
                Global::player->externalVel.set(0, 0, spd);
            }
            else if (Global::player->position.z >= (position.z + scale) - eps && spd > 0)
            {
                Global::player->externalVel.set(0, 0, spd);
            }
            else if (Global::player->position.z <= (position.z - scale) + eps && spd < 0)
            {
                Global::player->externalVel.set(0, 0, spd);
            }
        }
    }

    baseCM->transformModelWithScale(cm, &position, 0, scale);

    updateTransformationMatrix();
}

std::vector<Entity*>* CollisionBlock::getEntitiesToRender()
{
    return &entitiesToRender;
}

std::list<TexturedModel*>* CollisionBlock::getModels()
{
    return &CollisionBlock::models;
}

int CollisionBlock::getEntityType()
{
    return ENTITY_COLLISION_BLOCK;
}

std::vector<Triangle3D*>* CollisionBlock::getCollisionTriangles()
{
    return &cm->triangles;
}

void CollisionBlock::loadModels()
{
    if (CollisionBlock::models.size() == 0)
    {
        ObjLoader::loadModel(&CollisionBlock::models, "res/Models/Cheese/", "Cheese");
        CollisionBlock::baseCM = ObjLoader::loadCollisionModel("res/Models/Cheese/", "Cheese");
    }
}
