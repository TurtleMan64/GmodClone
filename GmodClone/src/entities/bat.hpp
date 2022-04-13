#ifndef BAT_H
#define BAT_H

class TexturedModel;

#include <list>
#include "entity.hpp"
#include "../toolbox/vector.hpp"

class Bat : public Entity
{
private:
    static Model models;

    float floatTimer = 0.0f;

public:
    Bat(std::string name, Vector3f pos);

    void step();

    std::vector<Entity*>* getEntitiesToRender();

    Model* getModels();

    static void loadModels();

    int getEntityType();
};
#endif
