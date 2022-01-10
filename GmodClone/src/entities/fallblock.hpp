#ifndef FALLBLOCK_H
#define FALLBLOCK_H

class TexturedModel;
class Triangle3D;
class CollisionModel;
class Dummy;

#include <list>
#include <vector>
#include "entity.hpp"
#include "../toolbox/vector.hpp"

class FallBlock : public Entity
{
private:
    static std::list<TexturedModel*> models;
    static std::list<TexturedModel*> modelsShadow;
    static CollisionModel* baseCM;

    CollisionModel* cm = nullptr;
    Dummy* shadow = nullptr;

    static constexpr float TIME_APPEAR = 7.0f;
    static constexpr float TIME_FALL = 4.5f;

    static constexpr float HEIGHT_TOP = 26.0f;

    float totalTimeAccumulated = 0.0f;

    bool oneLastAdjust = true;

    bool isStopped = false;

public:
    float phaseTimer       = 100000000.0f;
    float phaseTimerOffset = 100000000.0f;

    FallBlock(std::string name, Vector3f pos, float phaseTimer);
    ~FallBlock();

    void step();

    std::vector<Entity*>* getEntitiesToRender();

    std::list<TexturedModel*>* getModels();

    static void loadModels();

    int getEntityType();

    std::vector<Triangle3D*>* getCollisionTriangles();
};
#endif
