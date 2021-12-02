#ifndef CHANDELIER_H
#define CHANDELIER_H

class TexturedModel;

#include <list>
#include <vector>
#include "entity.hpp"
#include "../toolbox/vector.hpp"

class Chandelier : public Entity
{
private:
    static std::list<TexturedModel*> modelsChandelier;
    static std::list<TexturedModel*> modelsWallLamp;

    int lightIdx = 1;

    int type = 0;

    float flickerTimer = 0.0f;

public:
    Chandelier(std::string name, Vector3f pos, int type, float rotY, int lightIdx);
    ~Chandelier();

    void step();

    std::vector<Entity*>* getEntitiesToRender();

    std::list<TexturedModel*>* getModels();

    static void loadModels();

    int getEntityType();
};
#endif
