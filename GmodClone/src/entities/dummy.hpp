#ifndef DUMMY_H
#define DUMMY_H

class TexturedModel;

#include <list>
#include "entity.hpp"

class Dummy : public Entity
{
private:
    std::list<TexturedModel*>* myModels;

public:
    Dummy(std::list<TexturedModel*>* models);

    void step();

    void setRotation(float xr, float yr, float zr, float sr);

    std::list<TexturedModel*>* getModels();

    void setModels(std::list<TexturedModel*>* newModels);
};
#endif
