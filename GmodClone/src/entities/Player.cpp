#include <list>
#include <set>
#include <GLFW/glfw3.h>

#include "../toolbox/vector.h"
#include "player.h"
#include "../collision/collisionchecker.h"
#include "../collision/triangle3d.h"
#include "../toolbox/input.h"
#include "../main/main.h"
#include "../toolbox/maths.h"
#include "camera.h"
#include "ball.h"

extern float dt;

Player::Player(std::list<TexturedModel*>* models)
{
    myModels = models;
    scale = 0.5f;
    position.set(0, 10, 0);
    vel.set(0.000001f, 0, 0);
    groundNormal.set(0, 1, 0);
    wallNormal.set(1, 0, 0);
    camDir.set(0, 0, -1);
    visible = false;
}

void Player::step()
{
    Vector3f yAxis(0, -1, 0);

    updateCamera();

    // Crouching
    if (Input::inputs.INPUT_ACTION2 && slideTimer < 0.0f)
    {
        if (!isCrouching)
        {
            //CollisionResult result = CollisionChecker::checkCollision(position.x, position.y + COLLISION_RADIUS*3, position.z, COLLISION_RADIUS);
            //if (!result.hit)
            {
                isCrouching = true;
                //position.y += COLLISION_RADIUS*2;
            }
        }
    }
    else
    {
        if (isCrouching)
        {
            CollisionResult result = CollisionChecker::checkCollision(position.x, position.y + COLLISION_RADIUS*3, position.z, COLLISION_RADIUS);
            if (!result.hit)
            {
                isCrouching = false;

                //determine if we move the player back down ( so they cant get infinite height by spamming crouch
                //result = CollisionChecker::checkCollision(position.x, (position.y + COLLISION_RADIUS)-0.1f, position.z, COLLISION_RADIUS);
                //if (!result.hit)
                {
                    //position.y -= COLLISION_RADIUS*2;
                }
            }
        }
    }

    // Sliding
    slideTimer-=dt;
    if (Input::inputs.INPUT_ACTION4 && !Input::inputs.INPUT_PREVIOUS_ACTION4)
    {
        if (onGround && !isCrouching && vel.lengthSquared() > SLIDE_SPEED_REQUIRED*SLIDE_SPEED_REQUIRED && slideTimer < -SLIDE_TIMER_COOLDOWN)
        {
            slideTimer = SLIDE_TIMER_DURATION;
            storedSlideSpeed = vel.length() + SLIDE_SPEED_ADDITION;
        }
        //else if (onGround && slideTimer >= 0.0f)
        //{
        //    slideTimer = 0.0f;
        //}
    }

    if (!onGround)
    {
        slideTimer = -SLIDE_TIMER_COOLDOWN;
    }

    if (slideTimer > 0.0f)
    {
        vel.setLength(storedSlideSpeed);
    }

    //printf("slideTimer = %f\n", slideTimer);

    // Player direction input
    {
        float stickAngle = atan2f(Input::inputs.INPUT_Y, Input::inputs.INPUT_X) - Maths::PI/2; //angle you are holding on the stick, with 0 being up
        float stickRadius = sqrtf(Input::inputs.INPUT_X*Input::inputs.INPUT_X + Input::inputs.INPUT_Y*Input::inputs.INPUT_Y);

        if (slideTimer > 0.0f)
        {
            stickRadius = 0.0f;
        }

        Vector3f dirForward = Maths::projectOntoPlane(&camDir, &yAxis);
        dirForward.setLength(stickRadius);

        Vector3f velToAdd = Maths::rotatePoint(&dirForward, &yAxis, stickAngle + Maths::PI);

        if (onGround)
        {
            vel = vel + velToAdd.scaleCopy(getPushValueGround(dt)*dt);
        }
        else 
        {
            vel = vel + velToAdd.scaleCopy(getPushValueAir(dt)*dt);
        }
    }

    // Gravity
    vel = vel + yAxis.scaleCopy(FORCE_GRAVITY*dt);

    // Normal Jump
    if (Input::inputs.INPUT_ACTION1 && !Input::inputs.INPUT_PREVIOUS_ACTION1 && onGround)
    {
        vel = vel + groundNormal.scaleCopy(JUMP_SPEED);
    }

    // Wall Jumping
    wallJumpTimer-=dt;
    if (!onGround && isTouchingWall)
    {
        wallJumpTimer = WALL_JUMP_TIMER_MAX;
        storedWallNormal = wallNormal;
    }

    if (!onGround && wallJumpTimer >= 0.0f && Input::inputs.INPUT_ACTION1 && !Input::inputs.INPUT_PREVIOUS_ACTION1)
    {
        vel = vel + storedWallNormal.scaleCopy(WALL_JUMP_SPEED_HORIZONTAL);
        vel.y += WALL_JUMP_SPEED_VERTICAL;
        wallJumpTimer = -1.0f;
    }

    position = position + vel.scaleCopy(dt);

    bool hitAny = false;
    std::unordered_set<Triangle3D*> collisionResults;

    //bottom sphere collision
    for (int c = 0; c < 20; c++)
    {
        Vector3f spotToTest = position;
        spotToTest.y += COLLISION_RADIUS;
        CollisionResult result = CollisionChecker::checkCollision(&spotToTest, COLLISION_RADIUS);
        if (result.hit)
        {
            //resolve this collision
            float distanceToMoveAway = COLLISION_RADIUS - result.distanceToPosition;
            Vector3f directionToMove = result.directionToPosition.scaleCopy(-1);
            spotToTest = spotToTest + directionToMove.scaleCopy(distanceToMoveAway);
            position = spotToTest;
            position.y -= COLLISION_RADIUS;

            //move along the new plane
            vel = Maths::projectOntoPlane(&vel, &directionToMove);
            hitAny = true;
            collisionResults.insert(result.tri);
        }
        else
        {
            break;
        }
    }

    //top sphere collision
    if (!isCrouching)
    {
        for (int c = 0; c < 20; c++)
        {
            Vector3f spotToTest = position;
            spotToTest.y += COLLISION_RADIUS*3;
            CollisionResult result = CollisionChecker::checkCollision(&spotToTest, COLLISION_RADIUS);
            if (result.hit)
            {
                //resolve this collision
                float distanceToMoveAway = COLLISION_RADIUS - result.distanceToPosition;
                Vector3f directionToMove = result.directionToPosition.scaleCopy(-1);
                spotToTest = spotToTest + directionToMove.scaleCopy(distanceToMoveAway);
                position = spotToTest;
                position.y -= COLLISION_RADIUS*3;

                //move along the new plane
                vel = Maths::projectOntoPlane(&vel, &directionToMove);
                //hitAny = true;
                //collisionResults.insert(result.tri);
            }
            else
            {
                break;
            }
        }
    }

    onGround = false;
    isTouchingWall = false;
    //bool bounced = false;
    if (hitAny)
    {
        isTouchingWall = true;

        Vector3f normalGroundSum;
        Vector3f normalWallSum;

        for (auto tri : collisionResults)
        {
            if (tri->normal.y > 0.5f) //dont add walls into this calculation, since we use it to determine jump direction.
            {
                normalGroundSum = normalGroundSum + tri->normal;
                onGround = true;
                isTouchingWall = false;
            }
            else
            {
                normalWallSum = normalWallSum + tri->normal;

                slideTimer = 0.0f;
            }
        }

        normalGroundSum.normalize();
        normalWallSum.normalize();

        groundNormal = normalGroundSum;
        wallNormal = normalWallSum;
    }

    //printf("isTouchingWall = %d\n", isTouchingWall);

    if (onGround)
    {
        //Vector3f flat = Maths::projectOntoPlane(&vel, &relativeUp);
        //Vector3f up = Maths::projectAlongLine(&vel, &relativeUp);
        //flat = Maths::applyDrag(&flat, -friction, dt); //Slow vel down due to friction
        //vel = flat + up;
        //vel = Maths::applyDrag(&vel, -friction, dt); //Slow vel down due to friction
        Vector3f gravityDir(0, -1, 0);
        float sameness = fabsf(groundNormal.dot(&gravityDir));
        //sameness = 1.0f;
        //printf("%f\n", sameness);
        //printf("%f\n", vel.length());
        if (vel.length() > 10.0f/36.7816091954f)
        {
            vel = Maths::applyDrag(&vel, -(sameness*DRAG_GROUND), dt); //Slow vel down due to friction on ground
        }
        else
        {
            float down = 60*dt*(10.0f/36.7816091954f);
            if (vel.length() < down)
            {
                vel.scale(0.0000001f);
            }
            else
            {
                vel.setLength(vel.length() - down);
            }
        }
    }
    else
    {
        float ySpd = vel.y;
        vel.y = 0;
        vel = Maths::applyDrag(&vel, -(DRAG_AIR), dt); //Slow vel down due air drag, but just horizontally
        vel.y = ySpd;
    }

    //printf("%f\t%f\n", (float)glfwGetTime(), 36.7816091954f*vel.length());

    if (Input::inputs.INPUT_LB && !Input::inputs.INPUT_PREVIOUS_LB)
    {
        Ball* b = new Ball(position, camDir.scaleCopy(10));
        b->position.y += HUMAN_HEIGHT;
        Global::addEntity(b);
    }
    
    updateTransformationMatrix();
}

float Player::getPushValueGround(float deltaTime)
{
    double fps = 1/deltaTime;

    if (Input::inputs.INPUT_ACTION3)
    {
        return (float)(66.56795 + (17926930.0 - 66.56795)/(1.0 + pow(fps/0.00004730095, 1.080826)));
    }

    double val = (44.35799 + (17431400.0 - 44.35799)/(1.0 + pow(fps/0.00002812362, 1.065912)));
    if (isCrouching)
    {
        val = val/2;
    }
    return (float)val;
}

float Player::getPushValueAir(float deltaTime)
{
    double fps = 1/deltaTime;
    return (float)(9.917682 + (14.17576 - 9.917682)/(1.0 + pow(fps/1.386981, 1.042293)));
}

void Player::updateCamera()
{
    Vector3f yAxis(0, 1, 0);

    camDir = Maths::rotatePoint(&camDir, &yAxis, -Input::inputs.INPUT_X2*dt);
    camDir.normalize();

    Vector3f perpen = camDir.cross(&yAxis);
    perpen.normalize();

    const float maxViewAng = Maths::toRadians(0.1f);

    if (Input::inputs.INPUT_Y2 > 0) //mouse is going down
    {
        Vector3f down(0, -1, 0);
        float angBetweenCamAndDown = Maths::angleBetweenVectors(&camDir, &down);
        float angToRotate = Input::inputs.INPUT_Y2*dt;
        if (angBetweenCamAndDown - angToRotate > maxViewAng)
        {
            camDir = Maths::rotatePoint(&camDir, &perpen, -Input::inputs.INPUT_Y2*dt);
        }
        else
        {
            camDir = Maths::rotatePoint(&camDir, &perpen, -(angBetweenCamAndDown - maxViewAng));
        }
    }
    else
    {
        Vector3f up(0, 1, 0);
        float angBetweenCamAndUp = Maths::angleBetweenVectors(&camDir, &up);
        float angToRotate = -Input::inputs.INPUT_Y2*dt;
        if (angBetweenCamAndUp - angToRotate > maxViewAng)
        {
            camDir = Maths::rotatePoint(&camDir, &perpen, -Input::inputs.INPUT_Y2*dt);
        }
        else
        {
            camDir = Maths::rotatePoint(&camDir, &perpen, (angBetweenCamAndUp - maxViewAng));
        }
    }
    camDir.normalize();

    Vector3f eye(&position);
    if (!isCrouching && slideTimer <= 0.0f)
    {
        eye.y += HUMAN_HEIGHT - 0.01f; //put the eye at the top of the sphere (1.74m from feet)
    }
    else
    {
        eye.y += HUMAN_HEIGHT/2 - 0.01f;
    }

    Vector3f target(&eye);
    target = target + camDir;

    Vector3f up = Maths::rotatePoint(&camDir, &perpen, Maths::PI/2);
    up.normalize();

    Global::gameCamera->setViewMatrixValues(&eye, &target, &up);
}

void Player::setRotation(float xr, float yr, float zr, float sr)
{
    rotX = xr;
    rotY = yr;
    rotZ = zr;
    rotRoll = sr;
}

std::list<TexturedModel*>* Player::getModels()
{
    return myModels;
}

void Player::setModels(std::list<TexturedModel*>* newModels)
{
    myModels = newModels;
}
