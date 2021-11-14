#include <list>

#include "onlineplayer.hpp"
#include "../main/main.hpp"
#include "../loader/objloader.hpp"
#include "../toolbox/maths.hpp"

std::list<TexturedModel*> OnlinePlayer::modelsFall;
std::list<TexturedModel*> OnlinePlayer::modelsJump;
std::list<TexturedModel*> OnlinePlayer::modelsSlide;
std::list<TexturedModel*> OnlinePlayer::modelsSquat;
std::list<TexturedModel*> OnlinePlayer::modelsStand;

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
    rotZ = 0;

    updateTransformationMatrix();
}

std::list<TexturedModel*>* OnlinePlayer::getModels()
{
    if (isCrouching)
    {
        return &OnlinePlayer::modelsSquat;
    }
    else if (isSliding)
    {
        return &OnlinePlayer::modelsSlide;
    }
    else if (vel.y > 1.0f)
    {
        return &OnlinePlayer::modelsJump;
    }
    else if (vel.y < -1.0f)
    {
        return &OnlinePlayer::modelsFall;
    }

    return &OnlinePlayer::modelsStand;
}

int OnlinePlayer::getEntityType()
{
    return ENTITY_ONLINE_PLAYER;
}

void OnlinePlayer::loadModels()
{
    if (OnlinePlayer::modelsStand.size() == 0)
    {
        ObjLoader::loadModel(&OnlinePlayer::modelsFall , "res/Models/Human/", "ShrekFall");
        ObjLoader::loadModel(&OnlinePlayer::modelsJump , "res/Models/Human/", "ShrekJump");
        ObjLoader::loadModel(&OnlinePlayer::modelsSlide, "res/Models/Human/", "ShrekSlide");
        ObjLoader::loadModel(&OnlinePlayer::modelsSquat, "res/Models/Human/", "ShrekSquat");
        ObjLoader::loadModel(&OnlinePlayer::modelsStand, "res/Models/Human/", "ShrekStand");
    }
}
