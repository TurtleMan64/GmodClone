#ifndef DUMMY_H
#define DUMMY_H

class TexturedModel;

#include <list>
#include "entity.hpp"

class Dummy : public Entity
{
private:
    Model* myModels;

public:
    Dummy(Model* models);

    void step();

    void setRotation(float xr, float yr, float zr, float sr);

    std::vector<Entity*>* getEntitiesToRender();

    Model* getModels();

    void setModels(Model* newModels);
};
#endif
