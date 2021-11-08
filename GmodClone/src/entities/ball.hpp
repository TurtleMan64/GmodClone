#ifndef BALL_H
#define BALL_H

class TexturedModel;
class Source;

#include <list>
#include "entity.hpp"
#include "../toolbox/vector.hpp"

class Ball : public Entity
{
private:
    static std::list<TexturedModel*> models;

    Vector3f vel;

    Source* src = nullptr;

    const float gravityForce = 0.588f*60;

    const float DRAG_AIR = 0.4f;

    const float bounceAmount = 0.85f; //85% of relative vertical speed retain after bounce

public:
    Ball(Vector3f pos, Vector3f vel);

    void step();

    std::list<TexturedModel*>* getModels();
};
#endif
