#ifndef WINZONE_H
#define WINZONE_H

class TexturedModel;

#include <list>

#include "entity.hpp"
#include "../toolbox/vector.hpp"

class WinZone : public Entity
{
private:
    static Model model;

public:
    // Radius of each dimension of rectangular prism
    Vector3f size;

    WinZone(std::string name, Vector3f pos, Vector3f size);
    ~WinZone();

    void step();

    int getEntityType();

    static void loadModels();

    Model* getModel();

    std::vector<Entity*>* getEntitiesToRender();
};
#endif
