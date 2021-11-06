#ifndef HUMAN_H
#define HUMAN_H

class TexturedModel;

#include <list>
#include "entity.hpp"
#include "../toolbox/vector.hpp"

class Human : public Entity
{
private:
    static std::list<TexturedModel*> models;

public:
    Human(float x, float y, float z);

    void step();

    std::list<TexturedModel*>* getModels();
};
#endif
