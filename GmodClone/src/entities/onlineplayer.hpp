#ifndef ONLINEPLAYER_H
#define ONLINEPLAYER_H

class TexturedModel;

#include <list>
#include "entity.hpp"
#include "../toolbox/vector.hpp"

class OnlinePlayer : public Entity
{
private:
    static std::list<TexturedModel*> modelsFall;
    static std::list<TexturedModel*> modelsJump;
    static std::list<TexturedModel*> modelsSlide;
    static std::list<TexturedModel*> modelsSquat;
    static std::list<TexturedModel*> modelsStand;

public:
    char health = 0;
    char weapon = 0;
    bool isCrouching = false;
    bool isSliding = false;
    Vector3f lookDir;

    OnlinePlayer(std::string name, float x, float y, float z);

    void step();

    std::list<TexturedModel*>* getModels();

    static void loadModels();

    int getEntityType();
};
#endif
