#include <list>
#include <set>

#include "../toolbox/vector.h"
#include "ball.h"
#include "../collision/collisionchecker.h"
#include "../collision/triangle3d.h"
#include "../main/main.h"
#include "../toolbox/maths.h"
#include "../loader/objloader.h"

std::list<TexturedModel*> Ball::models;

extern float dt;

Ball::Ball(Vector3f pos, Vector3f vel)
{
    if (Ball::models.size() == 0)
    {
        ObjLoader::loadModel(&Ball::models, "res/Models/Ball/", "Soccer");
    }

    scale = 0.225f;
    position = pos;
    this->vel = vel;
    visible = true;
    onGround = false;
    groundNormal.set(0, 1, 0);
}

void Ball::step()
{
    Vector3f yAxis(0, 1, 0);

    if (!onGround)
    {
        vel = vel - yAxis.scaleCopy(gravityForce*dt);
    }
    else
    {
        float sc = 1.0f - yAxis.dot(&groundNormal);
        if (sc < 0.001f)
        {
            sc = 0.001f;
        }
        vel = vel - yAxis.scaleCopy(sc*gravityForce*dt);
        //printf("sc = %f\n", sc);
    }

    position = position + vel.scaleCopy(dt);

    bool hitAny = false;
    std::unordered_set<Triangle3D*> collisionResults;
    for (int c = 0; c < 20; c++)
    {
        CollisionResult result = CollisionChecker::checkCollision(&position, scale);
        if (result.hit)
        {
            //resolve this collision
            float distanceToMoveAway = scale - result.distanceToPosition;
            Vector3f directionToMove = result.directionToPosition.scaleCopy(-1);
            position = position + directionToMove.scaleCopy(distanceToMoveAway);

            //do we bounce
            Vector3f velBounce = Maths::bounceVector(&vel, &directionToMove, 1.0f);
            //printf("velBounce.length() = %f\n", velBounce.length());
            //Vector3f bounceFlat = Maths::projectOntoPlane(&velBounce, &directionToMove);
            Vector3f bounceUp = Maths::projectAlongLine(&velBounce, &directionToMove);
            //printf("velBounce.length = %f   bounceUp.length() = %f\n", velBounce.length(), bounceUp.length());
            //bounceUp.scale(bounceAmount);
            //float angBetween = Maths::angleBetweenVectors(&velBounce, &directionToMove);
            //printf("angBetween = %f\n", Maths::toDegrees(angBetween));

            //forget all this nonsense... just bounce 100% of the time
            //if (bounceUp.lengthSquared() >= bounceThreshold*bounceThreshold) 
            {
                //vel = bounceFlat + bounceUp;
                //vel = velBounce.scaleCopy(bounceAmount);
                bounceUp.scale(1 - bounceAmount);
                vel = velBounce - bounceUp;
                //printf("bounceUp.lengthSquared() = %f\n", bounceUp.length());
                
                //bounced = true;
                //hitAny = false;
                onGround = false;
                break;
                //TODO: could keep going to next collision check. basically continue; here instead of break;. it might be fine
            }

            //dont do this anymore, just bounce 100% of the time.
            //move along the new plane
            Vector3f velBefore = vel;
            vel = Maths::projectOntoPlane(&vel, &directionToMove);
            if (onGround)
            {
                Vector3f vel1 = vel;
                Vector3f vel2 = velBefore;
                vel1.normalize();
                vel2.normalize();
                vel.setLength(velBefore.length()*vel1.dot(&vel2));
            }
            else
            {
                //printf("transition from air to ground. velBefore.length = %f    vel.length = %f\n", velBefore.length(), vel.length());
            }
            hitAny = true;
            collisionResults.insert(result.tri);

            onGround = true;
        }
        else
        {
            break;
        }
    }

    //bool onGround = false;
    groundNormal.set(0,0,0);
    if (hitAny)
    {
        for (auto tri : collisionResults)
        {
            if (tri->normal.y > 0.5f) //dont add walls into this calculation, since we use it to determine jump direction.
            {
                groundNormal = groundNormal + tri->normal;
            }
        }
    
        groundNormal.normalize();
    }

    //if (hitAny)
    {
        //Vector3f flat = Maths::projectOntoPlane(&vel, &relativeUp);
        //Vector3f up = Maths::projectAlongLine(&vel, &relativeUp);
        //flat = Maths::applyDrag(&flat, -friction, dt); //Slow vel down due to friction
        //vel = flat + up;
        //vel = Maths::applyDrag(&vel, -friction, dt); //Slow vel down due to friction

        //float sameness = fabsf(groundNormal.dot(&yAxis));

        //vel = Maths::applyDrag(&vel, -(sameness*friction), dt); //Slow vel down due to friction
    }

    vel = Maths::applyDrag(&vel, -DRAG_AIR, dt); //Slow vel down due to air drag

    //printf("vel.length = %f\n", vel.length());

    Vector3f rotP = vel;
    rotP.y = 0;
    rotZ -= 150*rotP.length()*dt;
    rotY = Maths::toDegrees(atan2f(-vel.z, vel.x));

    updateTransformationMatrix();
}

std::list<TexturedModel*>* Ball::getModels()
{
    return &Ball::models;
}
