#include <list>
#include <set>

#include "../toolbox/vector.hpp"
#include "ball.hpp"
#include "../collision/collisionchecker.hpp"
#include "../collision/triangle3d.hpp"
#include "../main/main.hpp"
#include "../toolbox/maths.hpp"
#include "../loader/objloader.hpp"
#include "../audio/audioplayer.hpp"
#include "../audio/source.hpp"

Model Ball::models;

extern float dt;

Ball::Ball(std::string name, Vector3f pos, Vector3f vel)
{
    this->name = name;

    scale = 0.225f; //Size 5 soccer ball is ~22.5 cm
    position = pos;
    this->vel = vel;
    visible = true;

    entitiesToRender.push_back(this);

    //src = AudioPlayer::play(12, &position, 1.0f, true);
}

void Ball::step()
{
    Vector3f yAxis(0, 1, 0);

    vel = vel - yAxis.scaleCopy(gravityForce*dt);

    position = position + vel.scaleCopy(dt);

    CollisionResult result = CollisionChecker::checkCollision(&position, scale);
    if (result.hit)
    {
        // First, resolve the collision
        float distanceToMoveAway = scale - result.distanceToPosition;
        Vector3f directionToMove = result.directionToPosition.scaleCopy(-1);
        position = position + directionToMove.scaleCopy(distanceToMoveAway);

        // Then, bounce
        Vector3f velBounce = Maths::bounceVector(&vel, &directionToMove, 1.0f);

        Vector3f bounceUp = Maths::projectAlongLine(&velBounce, &directionToMove);

        if (bounceUp.lengthSquared() > 1*1)
        {
            AudioPlayer::play(12, &position);
        }

        bounceUp.scale(1 - bounceAmount);
        vel = velBounce - bounceUp;
    }

    vel = Maths::applyDrag(&vel, -DRAG_AIR, dt); //Slow vel down due to air drag

    Vector3f rotP = vel;
    rotP.y = 0;
    rotZ -= 150*rotP.length()*dt;
    rotY = Maths::toDegrees(atan2f(-vel.z, vel.x));

    //if (src != nullptr)
    //{
        //src->setPosition(position.x, position.y, position.z);
        //src->setVelocity(vel.x, vel.y, vel.z);
    //}

    updateTransformationMatrix();
}

std::vector<Entity*>* Ball::getEntitiesToRender()
{
    return &entitiesToRender;
}

Model* Ball::getModels()
{
    return &Ball::models;
}

int Ball::getEntityType()
{
    return ENTITY_BALL;
}

void Ball::getHit(Vector3f* /*hitPos*/, Vector3f* hitDir, int weapon)
{
    vel = *hitDir;

    switch (weapon)
    {
        case WEAPON_FIST: vel.setLength(10.0f); break;
        case WEAPON_BAT:  vel.setLength(30.0f); break;
        case WEAPON_GUN:  vel.setLength(50.0f); break;
        default: break;
    }
}

void Ball::loadModels()
{
    if (Ball::models.size() == 0)
    {
        ObjLoader::loadModel(&Ball::models, "res/Models/Ball/", "Soccer");
    }
}
