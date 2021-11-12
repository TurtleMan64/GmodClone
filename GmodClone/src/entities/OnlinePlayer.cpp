#include <list>

#include "onlineplayer.hpp"
#include "../main/main.hpp"
#include "../loader/objloader.hpp"
#include "../toolbox/maths.hpp"

std::list<TexturedModel*> OnlinePlayer::models;

OnlinePlayer::OnlinePlayer(std::string name, float x, float y, float z)
{
    this->name = name;
    visible = true;
    position.set(x, y, z);
    vel.set(0,0,0);
    lookDir.set(1, 0, 0);
    updateTransformationMatrix();
}

void OnlinePlayer::step()
{
    extern float dt;

    position = position + vel.scaleCopy(dt);

    Maths::sphereAnglesFromPosition(&lookDir, &rotY, &rotZ);

    updateTransformationMatrix();
}

std::list<TexturedModel*>* OnlinePlayer::getModels()
{
    return &OnlinePlayer::models;
}

int OnlinePlayer::getEntityType()
{
    return ENTITY_ONLINE_PLAYER;
}

void OnlinePlayer::loadModels()
{
    if (OnlinePlayer::models.size() == 0)
    {
        ObjLoader::loadModel(&OnlinePlayer::models, "res/Models/Human/", "Human");
    }
}
