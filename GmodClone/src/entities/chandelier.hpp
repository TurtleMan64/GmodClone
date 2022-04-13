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
    static Model modelsChandelier;
    static Model modelsWallLamp;

    int lightIdx = 1;

    int type = 0;

    float flickerTimer = 0.0f;

    float attenuation2;
    float attenuation3;

public:
    Chandelier(std::string name, Vector3f pos, int type, float rotY, float attenuation2, float attenuation3, int lightIdx);
    ~Chandelier();

    void step();

    std::vector<Entity*>* getEntitiesToRender();

    Model* getModels();

    static void loadModels();

    int getEntityType();
};
#endif
