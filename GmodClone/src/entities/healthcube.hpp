#ifndef HEALTHCUBE_H
#define HEALTHCUBE_H

class TexturedModel;

#include <list>
#include "entity.hpp"
#include "../toolbox/vector.hpp"

class HealthCube : public Entity
{
private:
    static std::list<TexturedModel*> models;

    float floatTimer = 0.0f;

public:
    HealthCube(std::string name, Vector3f pos);

    void step();

    std::list<TexturedModel*>* getModels();

    static void loadModels();

    int getEntityType();
};
#endif
