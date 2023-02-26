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

Model Ball::model;

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
    // idea: have a  list of collisions from previous frame. that way it can handle multiple at a time.
    //if (resultPrev.hit)
    //{
    //    Vector3f previousCollidePosition = resultPrev.collidePosition;
    //    Vector3f newCollidePosition = Maths::calcPointFromBarycentricCoords(&baryCoordsPrev, resultPrev.tri);
    //    Vector3f diff = newCollidePosition - previousCollidePosition;
    //
    //    float meters = diff.length();
    //    float triMoveSpeed = meters/dt;
    //
    //    Vector3f triMoveDir = diff;
    //    triMoveDir.normalize();
    //
    //    float dot = triMoveDir.dot(&resultPrev.tri->normal);
    //
    //    if (dot >= 0.001f) // There is some sort of contact/friction being applied from the moving triangle to the ball
    //    {
    //        //idea: calculate the amount done based off the "friction" between the ball and the triangle, calculated by remembering the previous frame's velocity right after gravity is applied. and also using the 
    //    
    //    
    //        
    //    }
    //
    //    //printf("diff = %f %f %f\n", diff.x, diff.y, diff.z);
    //    //diff.setLength(speed);
    //    //diff.scale(2.0f);
    //
    //    //idea: do things differently here depending on the normal of the triangle. 
    //    // if the normal and the diff are going in the same direction then it is a very
    //    // forceful push. if the normal and the diff are 90 degrees, then the ball
    //    // is only being scraped by it .
    //
    //    // either that or do this and the other movingBlocksAreDone function thing. 
    //    vel = vel + diff;
    //}

    Vector3f yAxis(0, 1, 0);
    vel = vel - yAxis.scaleCopy(gravityForce*dt);
    velPrev = vel;

    position = position + vel.scaleCopy(dt);

    resultPrev.hit = false;

    int numChecks = 5;
    while (numChecks > 0)
    {
        numChecks--;

        CollisionResult result = CollisionChecker::checkCollision(&position, scale);
        if (result.hit)
        {
            // First, resolve the collision
            float distanceToMoveAway = (scale - result.distanceToPosition);

            //if (distanceToMoveAway < 0.00001f)
            {
                //numChecks = 0;
            }

            distanceToMoveAway += 0.0001f;

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

            //if (resultPrev.hit)
            //{
            //    Vector3f previousCollidePosition = Maths::calcPointFromBarycentricCoords(&baryCoordsPrev, resultPrev.tri);
            //    //printf("previousCollidePosition %f %f %f\n", previousCollidePosition.x, previousCollidePosition.y, previousCollidePosition.z);
            //    Vector3f newCp = result.collidePosition;
            //    //printf("newCp %f %f %f\n\n", newCp.x, newCp.y, newCp.z);
            //    Vector3f diff = newCp - previousCollidePosition;
            //    float meters = diff.length();
            //    float speed = meters/dt;
            //    printf("diff = %f %f %f\n", diff.x, diff.y, diff.z);
            //    //diff.setLength(speed);
            //    diff.scale(2.0f);
            //    vel = vel - diff;
            //
            //    //printf("diff = %f %f %f\n", diff.x, diff.y, diff.z);
            //}

            if (result.entity != nullptr)
            {
                baryCoordsPrev = Maths::calcBarycentricCoordsFromPoint(&result.collidePosition, result.tri);
                resultPrev = result;
            }
        }
        else
        {
            numChecks = 0;
        }
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

Model* Ball::getModel()
{
    return &Ball::model;
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

void Ball::movingBlocksAreDone()
{
    //int numChecks = 20;
    // First check for displacement.
    CollisionResult result = CollisionChecker::checkCollision(&position, scale);
    if (result.hit)
    {
        // First, resolve the collision
        float distanceToMoveAway = scale - result.distanceToPosition;
    
        if (distanceToMoveAway > 0.0001f)
        {
            Vector3f directionToMove = result.directionToPosition.scaleCopy(-1);
            position = position + directionToMove.scaleCopy(distanceToMoveAway);
    
            vel = Maths::projectOntoPlane(&vel, &result.tri->normal);
    
            float unitsPerSecond = distanceToMoveAway/dt;
    
            vel = vel + directionToMove.scaleCopy(unitsPerSecond);
        }
    }

    //Next check for friction due to the previous collision point moving
    if (resultPrev.hit && resultPrev.entity != nullptr)
    {
        Vector3f previousCollidePosition = resultPrev.collidePosition;
        Vector3f newCollidePosition = Maths::calcPointFromBarycentricCoords(&baryCoordsPrev, resultPrev.tri);
        Vector3f diff = newCollidePosition - previousCollidePosition;
    
        float meters = diff.length();
        float triMoveSpeed = meters/dt;
    
        Vector3f triMoveDir = diff;
        triMoveDir.normalize();
    
        float dot = velPrev.dot(&resultPrev.tri->normal);

        // < 0 means the vel was going into the triangle, not away from it. so we need to apply some friction
        if (dot < 0)
        {
            Vector3f frictionVec = Maths::projectAlongLine(&velPrev, &resultPrev.tri->normal);
            float frictionAmount = frictionVec.length()/dt;

            //Vector3f restoreMe = Maths::projectOntoPlane(&vel, &resultPrev.tri->normal);
            Vector3f modifyMe  = Maths::projectAlongLine(&vel, &triMoveDir);
            Vector3f restoreMe = vel - modifyMe;

            //printf("MM = %f %f %f\n", modifyMe.x, modifyMe.y, modifyMe.z);
            //printf("RM = %f %f %f\n", restoreMe.x, restoreMe.y, restoreMe.z);

            // the relative speed between the ball and the moving triangle
            Vector3f diffBetweenTriMoveAndMyVel = triMoveDir.scaleCopy(triMoveSpeed) - modifyMe;
            //printf("D1 = %f %f %f\n", diffBetweenTriMoveAndMyVel.x, diffBetweenTriMoveAndMyVel.y, diffBetweenTriMoveAndMyVel.z);

            diffBetweenTriMoveAndMyVel = Maths::applyDrag(&diffBetweenTriMoveAndMyVel, -frictionAmount/7.5f, dt); //Slow vel down due to friction
            //printf("D2 = %f %f %f\n", diffBetweenTriMoveAndMyVel.x, diffBetweenTriMoveAndMyVel.y, diffBetweenTriMoveAndMyVel.z);

            Vector3f newVel = (triMoveDir.scaleCopy(triMoveSpeed) - diffBetweenTriMoveAndMyVel);
            //printf("NV = %f %f %f\n\n", newVel.x, newVel.y, newVel.z);

            vel = restoreMe + newVel;


            //vel = restoreMe + (triMoveDir.scaleCopy(triMoveSpeed*0.01f) + modifyMe.scaleCopy(0.99f));

            //printf("frictionAmount = %f\n\n", frictionAmount);
            //printf("frictionDir = %f %f %f\n\n", frictionVec.x, frictionVec.y, frictionVec.z);
        }
    }
}

void Ball::loadModels()
{
    if (!Ball::model.isLoaded())
    {
        ObjLoader::loadModel(&Ball::model, "res/Models/Ball/", "Soccer");
    }
}
