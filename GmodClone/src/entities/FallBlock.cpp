#include <list>
#include <set>

#include "../toolbox/vector.hpp"
#include "fallblock.hpp"
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
#include "../audio/audioplayer.hpp"
#include "../network/tcpclient.hpp"
#include "dummy.hpp"

std::list<TexturedModel*> FallBlock::models;
std::list<TexturedModel*> FallBlock::modelsShadow;
CollisionModel* FallBlock::baseCM = nullptr;

FallBlock::FallBlock(std::string name, Vector3f pos, float phaseTimer)
{
    this->name = name;

    cm = FallBlock::baseCM->duplicateMe();

    this->phaseTimer = phaseTimer;
    phaseTimerOffset = phaseTimer;

    position = pos;
    position.y = HEIGHT_TOP;
    visible = true;

    shadow = new Dummy(&FallBlock::modelsShadow); INCR_NEW("Dummy");
    shadow->baseAlpha = 0.5f;

    entitiesToRender.push_back(this);
    entitiesToRender.push_back(shadow);

    baseCM->transformModel(cm, &position);

    updateTransformationMatrix();
    shadow->updateTransformationMatrix();
}

FallBlock::~FallBlock()
{
    if (cm != nullptr)
    {
        cm->deleteMe();
        delete cm; INCR_DEL("CollisionModel");
    }

    if (shadow != nullptr)
    {
        delete shadow; INCR_DEL("Dummy");
        shadow = nullptr;
    }
}

extern float dt;

void FallBlock::step()
{
    if (Global::timeUntilRoundStarts <= 0.0f && phaseTimer > 0.0f)
    {
        totalTimeAccumulated += dt;

        phaseTimer = phaseTimerOffset - totalTimeAccumulated - 0.01f*(totalTimeAccumulated*totalTimeAccumulated);
    }

    if (Global::timeUntilRoundEnds > 0.0f)
    {
        if (phaseTimer > TIME_APPEAR) // We are invisible
        {
            visible = false;
            shadow->visible = false;
        }
        else if (phaseTimer > TIME_FALL) // We are visible and our shadow is visible
        {
            visible = true;
            shadow->visible = true;

            float progress = 1.0f - ((phaseTimer - TIME_FALL)/(TIME_APPEAR - TIME_FALL));

            CollisionResult result = CollisionChecker::checkCollision(position.x, position.y, position.z, position.x, position.y - 1000.0f, position.z);
            if (result.hit)
            {
                shadow->position = result.collidePosition;
                shadow->position.y += 0.005f;
                shadow->scale = progress;
            }
            shadow->updateTransformationMatrix();
        }
        else if (phaseTimer > 0.0f) // We are falling down
        {
            visible = true;
            shadow->visible = true;

            float progress = 1.0f - (phaseTimer/TIME_FALL);

            float nextY = HEIGHT_TOP - HEIGHT_TOP*progress;

            position.y += 0.001f;

            CollisionResult result = CollisionChecker::checkCollision(position.x, position.y, position.z, position.x, nextY, position.z);
            if (result.hit)
            {
                position = result.collidePosition;
                phaseTimer = 0.0f;
                oneLastAdjust = false;
            }
            else
            {
                position.y = nextY;
            }

            baseCM->transformModel(cm, &position);
            updateTransformationMatrix();

            result = CollisionChecker::checkCollision(position.x, position.y, position.z, position.x, position.y - 1000.0f, position.z);
            if (result.hit)
            {
                shadow->position = result.collidePosition;
                shadow->position.y += 0.005f;
            }

            shadow->scale = 1.0f;
            shadow->updateTransformationMatrix();

            // Check if the player got crushed
            Vector3f diff = position - Global::player->position;
            if (fabsf(diff.x) < 4 + Player::COLLISION_RADIUS &&
                fabsf(diff.z) < 4 + Player::COLLISION_RADIUS &&
                Global::player->position.y + Player::HUMAN_HEIGHT > position.y &&
                Global::player->position.y < position.y + 1.5f)
            {
                bool collided = false;

                Vector3f center = Global::player->position;
                center.y += Player::COLLISION_RADIUS;
                float out1;
                Vector3f out2;
                for (Triangle3D* tri : cm->triangles)
                {
                    if (Maths::sphereIntersectsTriangle(&center, Player::COLLISION_RADIUS, tri, &out1, &out2))
                    {
                        collided = true;
                        break;
                    }
                }

                if (!collided && !Global::player->isCrouching && Global::player->slideTimer < 0.0f)
                {
                    center.y += Player::COLLISION_RADIUS*2;
                    for (Triangle3D* tri : cm->triangles)
                    {
                        if (Maths::sphereIntersectsTriangle(&center, Player::COLLISION_RADIUS, tri, &out1, &out2))
                        {
                            collided = true;
                            break;
                        }
                    }
                }

                if (collided && Global::player->health > 0)
                {
                    Global::player->health = 0;
                    AudioPlayer::play(66, nullptr);
                    Global::sendAudioMessageToServer(66, &Global::player->position);
                }
            }
        }
        else // We have come to a stop 
        {
            visible = true;
            shadow->visible = false;
        
            if (oneLastAdjust)
            {
                oneLastAdjust = false;

                position.y += 0.001f;

                CollisionResult result = CollisionChecker::checkCollision(position.x, position.y, position.z, position.x, position.y - 10, position.z);
                if (result.hit)
                {
                    position = result.collidePosition;
                }

                updateTransformationMatrix();
                baseCM->transformModel(cm, &position);
            }

            if (!isStopped)
            {
                isStopped = true;
                AudioPlayer::play(76, &position);
            }
        }
    }
}

std::vector<Entity*>* FallBlock::getEntitiesToRender()
{
    return &entitiesToRender;
}

std::list<TexturedModel*>* FallBlock::getModels()
{
    return &FallBlock::models;
}

int FallBlock::getEntityType()
{
    return ENTITY_FALL_BLOCK;
}

std::vector<Triangle3D*>* FallBlock::getCollisionTriangles()
{
    if (phaseTimer > TIME_FALL)
    {
        return nullptr;
    }
    else
    {
        return &cm->triangles;
    }
}

void FallBlock::loadModels()
{
    if (FallBlock::models.size() == 0)
    {
        ObjLoader::loadModel(&FallBlock::models,          "res/Models/FallBlock/", "FallBlock2");
        ObjLoader::loadModel(&FallBlock::modelsShadow,    "res/Models/FallBlock/", "Shadow");
        FallBlock::baseCM = ObjLoader::loadCollisionModel("res/Models/FallBlock/", "FallBlock2");
    }
}
