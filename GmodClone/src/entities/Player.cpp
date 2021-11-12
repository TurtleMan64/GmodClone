#include <list>
#include <set>
#include <GLFW/glfw3.h>

#include "../toolbox/vector.hpp"
#include "player.hpp"
#include "../collision/collisionchecker.hpp"
#include "../collision/triangle3d.hpp"
#include "../toolbox/input.hpp"
#include "../main/main.hpp"
#include "../toolbox/maths.hpp"
#include "camera.hpp"
#include "ball.hpp"
#include "../audio/audioplayer.hpp"

extern float dt;

Player::Player(std::list<TexturedModel*>* models)
{
    myModels = models;
    scale = 0.5f;
    position.set(77.327835f, 4.313488f, -14.730497f);
    vel.set(0.000001f, 0, 0);
    groundNormal.set(0, 1, 0);
    lastGroundNormal.set(0, 1, 0);
    wallNormal.set(1, 0, 0);
    lookDir.set(0, 0, -1);
    visible = false;
}

void Player::step()
{
    Vector3f yAxis(0, -1, 0);

    updateCamera();

    if (Input::inputs.INPUT_SCROLL != 0)
    {
        AudioPlayer::play(56, nullptr);
        weapon += Input::inputs.INPUT_SCROLL;
        weapon = weapon % 2;
        if (weapon < 0)
        {
            weapon = 1;
        }
    }

    if (Input::inputs.INPUT_LEFT_CLICK && !Input::inputs.INPUT_PREVIOUS_LEFT_CLICK)
    {
        swingYourArm();
    }

    // Sliding
    float slideTimerBefore = slideTimer;
    slideTimer-=dt;
    if (Input::inputs.INPUT_ACTION4 && !Input::inputs.INPUT_PREVIOUS_ACTION4)
    {
        if (timeSinceOnGround <= AIR_JUMP_TOLERANCE && !isCrouching && vel.lengthSquared() > SLIDE_SPEED_REQUIRED*SLIDE_SPEED_REQUIRED && slideTimer <= -SLIDE_TIMER_COOLDOWN)
        {
            AudioPlayer::play(1, nullptr);
            slideTimer = SLIDE_TIMER_DURATION;
            storedSlideSpeed = vel.length() + SLIDE_SPEED_ADDITION;
            vel = Maths::projectOntoPlane(&vel, &groundNormal);
        }
    }

    // If holding crouch after slide is done, start crouching
    if (slideTimer <= 0.0f && slideTimerBefore > 0.0f && Input::inputs.INPUT_ACTION4)
    {
        isCrouching = true;
    }

    // If you are in the air, cancel slide
    if (timeSinceOnGround > AIR_JUMP_TOLERANCE)
    {
        if (slideTimer > 0.0f && Input::inputs.INPUT_ACTION4)
        {
            isCrouching = true;
        }

        slideTimer = -SLIDE_TIMER_COOLDOWN;
    }

    // When sliding, maintain speed
    if (slideTimer > 0.0f)
    {
        vel.setLength(storedSlideSpeed);
    }

    // Crouching
    if (Input::inputs.INPUT_ACTION4)
    {
        if ((slideTimer < 0.0f && vel.lengthSquared() < SLIDE_SPEED_REQUIRED*SLIDE_SPEED_REQUIRED) || (!onGround && slideTimer < 0.0f))
        {
            if (!isCrouching)
            {
                isCrouching = true;
                position.y += COLLISION_RADIUS*2;
                eyeHeightSmooth -= COLLISION_RADIUS*2;
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

                if (!onGround)
                {
                    //determine if we move the player back down ( so they cant get infinite height by spamming crouch
                    float distanceToFloor = -1.0f;
                    const int NUM_ITERATIONS = 200; //can probably reduce this, but makes things possibly choppier
                    for (int i = 0; i < NUM_ITERATIONS; i++)
                    {
                        float yOff = i*COLLISION_RADIUS*(2.0f/NUM_ITERATIONS);
                        result = CollisionChecker::checkCollision(position.x, (position.y + COLLISION_RADIUS) - yOff, position.z, COLLISION_RADIUS);
                        if (result.hit)
                        {
                            distanceToFloor = yOff;
                            break;
                        }
                    }

                    if (distanceToFloor < 0.0f)
                    {
                        position.y -= COLLISION_RADIUS*2;
                        eyeHeightSmooth += COLLISION_RADIUS*2;
                    }
                    else
                    {
                        position.y -= distanceToFloor;
                        eyeHeightSmooth += distanceToFloor;
                    }
                }
            }
        }
    }

    // Player direction input
    if (slideTimer < 0.0f)
    {
        float stickAngle = atan2f(Input::inputs.INPUT_Y, Input::inputs.INPUT_X) - Maths::PI/2; //angle you are holding on the stick, with 0 being up
        float stickRadius = sqrtf(Input::inputs.INPUT_X*Input::inputs.INPUT_X + Input::inputs.INPUT_Y*Input::inputs.INPUT_Y);

        Vector3f dirForward = Maths::projectOntoPlane(&lookDir, &yAxis);
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
    if (Input::inputs.INPUT_ACTION1 && !Input::inputs.INPUT_PREVIOUS_ACTION1 && timeSinceOnGround <= AIR_JUMP_TOLERANCE)
    {
        AudioPlayer::play(36, nullptr);
        vel = vel + lastGroundNormal.scaleCopy(getJumpValue(dt));
        timeSinceOnGround = AIR_JUMP_TOLERANCE + 0.0001f;
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
        AudioPlayer::play(50, nullptr);
        vel = vel + storedWallNormal.scaleCopy(WALL_JUMP_SPEED_HORIZONTAL);
        vel.y += getJumpValue(dt) - 1.0f;
        wallJumpTimer = -1.0f;
    }

    // Go through entities and get pushed by NPC's
    for (Entity* e : Global::gameEntities)
    {
        switch (e->getEntityType())
        {
            case ENTITY_ONLINE_PLAYER:
            {
                Vector3f diff = e->position - position;
                if (fabsf(diff.y) < HUMAN_HEIGHT)
                {
                    diff.y = 0;
                    if (diff.lengthSquared() < COLLISION_RADIUS*COLLISION_RADIUS)
                    {
                        float percent = (COLLISION_RADIUS - diff.length())/COLLISION_RADIUS;
                        float power = 200*dt*percent;
                        diff.setLength(power);
                        diff.scale(-1);
                        vel = vel + diff;
                    }
                }
                break;
            }

            case ENTITY_BALL:
            {
                Vector3f diff = e->position - position;
                if (fabsf(diff.y) < HUMAN_HEIGHT)
                {
                    diff.y = 0;
                    if (diff.lengthSquared() < COLLISION_RADIUS*COLLISION_RADIUS)
                    {
                        e->vel = e->vel + vel.scaleCopy(1.1f);
                    }
                }
                break;
            }

            default:
                break;
        }
    }

    position = position + vel.scaleCopy(dt) + externalVel.scaleCopy(dt);

    bool hitAny = false;
    std::vector<Triangle3D*> collisionResults;

    Vector3f velBefore = vel;

    Entity* previousCollideEntity = collideEntityImTouching;
    collideEntityImTouching = nullptr;

    const float MIN_BOUNCE_SPD = 6.0f;

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

            if (result.tri->type == 1)
            {
                vel = Maths::bounceVector(&vel, &result.tri->normal, 1.0f);
                if (vel.lengthSquared() < MIN_BOUNCE_SPD*MIN_BOUNCE_SPD)
                {
                    vel = result.tri->normal;
                    vel.setLength(MIN_BOUNCE_SPD);
                }
                AudioPlayer::play((int)(2*Maths::random()) + 57, nullptr);
                break;
            }
            else if (result.tri->type == 2)
            {
                die();
                break;
            }

            //move along the new plane
            vel = Maths::projectOntoPlane(&vel, &directionToMove);
            hitAny = true;
            collisionResults.push_back(result.tri);

            if (result.entity != nullptr)
            {
                collideEntityImTouching = result.entity;
            }
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

                if (result.tri->type == 1)
                {
                    vel = Maths::bounceVector(&vel, &result.tri->normal, 1.0f);
                    if (vel.lengthSquared() < MIN_BOUNCE_SPD*MIN_BOUNCE_SPD)
                    {
                        vel = result.tri->normal;
                        vel.setLength(MIN_BOUNCE_SPD);
                    }
                    AudioPlayer::play((int)(2*Maths::random()) + 57, nullptr);
                    break;
                }
                else if (result.tri->type == 2)
                {
                    die();
                    break;
                }

                //move along the new plane
                vel = Maths::projectOntoPlane(&vel, &directionToMove);

                if (result.entity != nullptr)
                {
                    collideEntityImTouching = result.entity;
                }
            }
            else
            {
                break;
            }
        }
    }

    if (collideEntityImTouching == nullptr && previousCollideEntity != nullptr)
    {
        //printf("adding vel %f\n", externalVel.length());
        vel = vel + externalVel;
    }
    //if (externalVelPrevious.lengthSquared() > 0.0f && externalVel.lengthSquared() == 0.0f)
    //{
    //    //printf("adding vel %f\n", externalVelPrevious.length());
    //    vel = vel + externalVelPrevious;
    //}
    //externalVelPrevious = externalVel;
    externalVel.set(0, 0, 0);

    bool onGroundBefore = onGround;
    onGround = false;
    isTouchingWall = false;

    bool touchedAWall = false;
    if (hitAny)
    {
        isTouchingWall = true;

        Vector3f normalGroundSum;
        Vector3f normalWallSum;

        for (Triangle3D* tri : collisionResults)
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
                touchedAWall = true;
            }
        }

        normalGroundSum.normalize();
        normalWallSum.normalize();

        groundNormal = normalGroundSum;
        wallNormal = normalWallSum;
    }

    // End the slide if contact with wall is too direct
    if (touchedAWall && slideTimer > 0.0f)
    {
        Vector3f velNorm = velBefore;
        velNorm.normalize();

        if (velNorm.dot(&wallNormal) < -0.6f)
        {
            slideTimer = 0.0f;

            if (Input::inputs.INPUT_ACTION4)
            {
                isCrouching = true;
            }
        }
    }

    //Calculate landing on the ground
    landSoundTimer+=dt;
    if (!onGroundBefore && onGround)
    {
        Vector3f ouchness = Maths::projectAlongLine(&velBefore, &groundNormal);

        if (ouchness.lengthSquared() > 14.0f*14.0f)
        {
            //TODO calculate and apply fall damage;

            if (landSoundTimer > 0.2f)
            {
                AudioPlayer::play(51, nullptr);
            }
        }
        else if (ouchness.lengthSquared() > 4.0f*4.0f)
        {
            if (landSoundTimer > 0.2f)
            {
                AudioPlayer::play(49, nullptr);
            }
        }

        landSoundTimer = 0.0f;
    }

    if (onGround)
    {
        timeSinceOnGround = 0.0f;
        lastGroundNormal = groundNormal;
    }
    else
    {
        timeSinceOnGround+=dt;
    }

    // Footstep sounds
    if (timeSinceOnGround <= 0.02f && slideTimer < 0.0f)
    {
        float stepTimerBefore = stepTimer;
        stepTimer+=vel.length()*dt;

        if (stepTimerBefore < 2.0f && stepTimer >= 2.0f)
        {
            stepTimer-=2.0f;
            AudioPlayer::play(36 + (int)(Maths::random()*4), nullptr);
        }
    }

    if (onGround)
    {
        if (vel.length() > 10.0f/36.7816091954f) //If going fast, slow down from drag equation
        {
            Vector3f gravityDir(0, -1, 0);
            //float sameness = fabsf(groundNormal.dot(&gravityDir));
            //printf("ground normal = %f %f %f\n", groundNormal.x, groundNormal.y, groundNormal.z);
            //printf("sameness = %f\n", sameness);
            float sameness = 1.0f;
            vel = Maths::applyDrag(&vel, -sameness*DRAG_GROUND, dt); //Slow vel down due to friction on ground
        }
        else //If going slow, slow down from linear equation
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
        vel = Maths::applyDrag(&vel, -DRAG_AIR, dt); //Slow vel down due air drag, but just horizontally
        vel.y = ySpd;
    }

    //printf("%f\t%f\n", (float)glfwGetTime(), 36.7816091954f*vel.length());

    if (Input::inputs.INPUT_LB && !Input::inputs.INPUT_PREVIOUS_LB)
    {
        Ball* b = new Ball("Ball0", position, lookDir.scaleCopy(10));
        b->position.y += HUMAN_HEIGHT;
        Global::addEntity(b);
    }
    
    updateTransformationMatrix();
}

float Player::getPushValueGround(float deltaTime)
{
    double fps = 1.0/deltaTime;

    double val;

    if (Input::inputs.INPUT_ACTION3) //Running
    {
        val = 66.56795 + (17926930.0 - 66.56795)/(1.0 + pow(fps/0.00004730095, 1.080826));
    }
    else //Walking
    {
        val = 44.35799 + (17431400.0 - 44.35799)/(1.0 + pow(fps/0.00002812362, 1.065912));
    }

    if (isCrouching)
    {
        val = val - 25.0f;
    }

    return (float)val;
}

float Player::getPushValueAir(float deltaTime)
{
    double fps = 1.0/deltaTime;
    return (float)(9.917682 + (14.17576 - 9.917682)/(1.0 + pow(fps/1.386981, 1.042293)));
}

float Player::getJumpValue(float deltaTime)
{
    double fps = 1.0/deltaTime;
    return (float)(5.314317 + (60497.54 - 5.314317)/(1.0 + pow(fps/0.000111619, 0.9930026)));
}

void Player::updateCamera()
{
    Vector3f yAxis(0, 1, 0);

    lookDir = Maths::rotatePoint(&lookDir, &yAxis, -Input::inputs.INPUT_X2);
    lookDir.normalize();

    Vector3f perpen = lookDir.cross(&yAxis);
    perpen.normalize();

    const float maxViewAng = Maths::toRadians(0.1f);

    if (Input::inputs.INPUT_Y2 > 0) //mouse is going down
    {
        Vector3f down(0, -1, 0);
        float angBetweenCamAndDown = Maths::angleBetweenVectors(&lookDir, &down);
        float angToRotate = Input::inputs.INPUT_Y2;
        if (angBetweenCamAndDown - angToRotate > maxViewAng)
        {
            lookDir = Maths::rotatePoint(&lookDir, &perpen, -Input::inputs.INPUT_Y2);
        }
        else
        {
            lookDir = Maths::rotatePoint(&lookDir, &perpen, -(angBetweenCamAndDown - maxViewAng));
        }
    }
    else
    {
        Vector3f up(0, 1, 0);
        float angBetweenCamAndUp = Maths::angleBetweenVectors(&lookDir, &up);
        float angToRotate = -Input::inputs.INPUT_Y2;
        if (angBetweenCamAndUp - angToRotate > maxViewAng)
        {
            lookDir = Maths::rotatePoint(&lookDir, &perpen, -Input::inputs.INPUT_Y2);
        }
        else
        {
            lookDir = Maths::rotatePoint(&lookDir, &perpen, (angBetweenCamAndUp - maxViewAng));
        }
    }
    lookDir.normalize();

    if (isCrouching || slideTimer > 0.0f)
    {
        eyeHeightSmooth  = Maths::approach(eyeHeightSmooth, HUMAN_HEIGHT/2 - 0.07f, 12.0f, dt);
    }
    else
    {
        eyeHeightSmooth  = Maths::approach(eyeHeightSmooth, HUMAN_HEIGHT - 0.07f, 12.0f, dt);
    }

    Vector3f eye(&position);
    eye.y += eyeHeightSmooth;

    Vector3f target(&eye);
    target = target + lookDir;

    Vector3f up = Maths::rotatePoint(&lookDir, &perpen, Maths::PI/2);
    up.normalize();

    Global::gameCamera->setViewMatrixValues(&eye, &target, &up);
}

void Player::swingYourArm()
{
    Vector3f camDir = Global::gameCamera->target - Global::gameCamera->eye;
    camDir.setLength(ARM_REACH);

    Vector3f target = Global::gameCamera->eye + camDir;
    CollisionResult result = CollisionChecker::checkCollision(&Global::gameCamera->eye, &target);

    Entity* hitEntity = nullptr;

    float distToCollisionSquared = 10000000000.0f;
    if (result.hit)
    {
        distToCollisionSquared = result.distanceToPosition*result.distanceToPosition;
        hitEntity = result.entity;
    }

    for (Entity* e : Global::gameEntities)
    {
        switch (e->getEntityType())
        {
            case ENTITY_BALL:
            {
                Vector3f ballCollisionSpot;
                if (Maths::lineSegmentIntersectsSphere(&Global::gameCamera->eye, &target, &e->position, e->scale, &ballCollisionSpot))
                {
                    float thisDistSquared = (Global::gameCamera->eye - ballCollisionSpot).lengthSquared();
                    if (thisDistSquared < distToCollisionSquared)
                    {
                        distToCollisionSquared = thisDistSquared;
                        hitEntity = e;
                    }
                }
                break;
            }

            default:
                break;
        }
    }

    if (hitEntity != nullptr)
    {
        camDir.normalize();
        Vector3f hitPosition = Global::gameCamera->eye + camDir.scaleCopy(sqrtf(distToCollisionSquared));
        hitEntity->getHit(&hitPosition, &camDir, weapon);
    }
    else if (result.hit)
    {
        AudioPlayer::play(50, nullptr);
    }
    else //miss
    {
        AudioPlayer::play(52, nullptr);
    }
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

void Player::die()
{
    health = 0.0f;
    AudioPlayer::play(59, nullptr);
}
