#ifndef BALL_H
#define BALL_H

class TexturedModel;

#include <list>
#include "entity.h"
#include "../toolbox/vector.h"

class Ball : public Entity
{
private:
    static std::list<TexturedModel*> models;

    Vector3f vel;

    bool onGround = false;

    const float gravityForce = 0.588f*60;

    Vector3f groundNormal;

    //const float friction = 0.6f;

    const float DRAG_AIR = 0.4f;

    const float bounceAmount = 0.85f; //85% of relative vertical speed retain after bounce
    const float bounceThreshold = 0.5f; 

public:
    Ball(Vector3f pos, Vector3f vel);

    void step();

    std::list<TexturedModel*>* getModels();
};
#endif
