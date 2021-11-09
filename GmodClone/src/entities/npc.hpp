#ifndef NPC_H
#define NPC_H

class TexturedModel;

#include <list>
#include "entity.hpp"
#include "../toolbox/vector.hpp"

class Npc : public Entity
{
private:
    static std::list<TexturedModel*> models;

public:
    Npc(std::string name, float x, float y, float z);

    void step();

    std::list<TexturedModel*>* getModels();

    int getEntityType();
};
#endif
