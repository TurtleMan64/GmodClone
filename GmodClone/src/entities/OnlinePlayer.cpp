#include <list>
#include <cstring>

#include "onlineplayer.hpp"
#include "../main/main.hpp"
#include "../loader/objloader.hpp"
#include "../toolbox/maths.hpp"

#include "../toolbox/vector.hpp"
#include "player.hpp"
#include "../collision/collisionchecker.hpp"
#include "../collision/triangle3d.hpp"
#include "../toolbox/input.hpp"
#include "../main/main.hpp"
#include "camera.hpp"
#include "ball.hpp"
#include "ladder.hpp"
#include "dummy.hpp"
#include "../fontMeshCreator/guitext.hpp"
#include "../audio/audioplayer.hpp"

std::list<TexturedModel*> OnlinePlayer::modelsHead;
std::list<TexturedModel*> OnlinePlayer::modelsFall;
std::list<TexturedModel*> OnlinePlayer::modelsJump;
std::list<TexturedModel*> OnlinePlayer::modelsSlide;
std::list<TexturedModel*> OnlinePlayer::modelsSquat;
std::list<TexturedModel*> OnlinePlayer::modelsStand;

extern float dt;

OnlinePlayer::OnlinePlayer(std::string name, float x, float y, float z)
{
    this->name = name;
    visible = true;
    position.set(x, y, z);
    vel.set(0.000001f, 0, 0);
    lookDir.set(0, 0, -1);
    groundNormal.set(0, 1, 0);
    wallNormal.set(1, 0, 0);
    lastGroundNormal.set(0, 1, 0);
    updateTransformationMatrix();

    head = new Dummy(&OnlinePlayer::modelsHead); INCR_NEW("Dummy");

    entitiesToRender.push_back(this);
    entitiesToRender.push_back(head);
}

OnlinePlayer::~OnlinePlayer()
{
    if (head != nullptr)
    {
        delete head; INCR_DEL("Dummy");
        head = nullptr;
    }

    if (nametag != nullptr)
    {
        Global::gameOnlinePlayersSharedMutex.lock();
        Global::gameOnlinePlayerNametagsToDelete.push_back(nametag);
        Global::gameOnlinePlayersSharedMutex.unlock();

        nametag = nullptr;
    }
}

void OnlinePlayer::step()
{
    Vector3f yAxis(0, -1, 0);

    updateCamera();

    // Sliding
    //float slideTimerBefore = slideTimer;
    //slideTimer-=dt;
    //
    //// If holding crouch after slide is done, start crouching
    //if (slideTimer <= 0.0f && slideTimerBefore > 0.0f)
    //{
    //    if (inputAction4)
    //    {
    //        isCrouching = true;
    //    }
    //    else
    //    {
    //        CollisionResult result = CollisionChecker::checkCollision(position.x, position.y + COLLISION_RADIUS*3, position.z, COLLISION_RADIUS - 0.01f);
    //        if (result.hit)
    //        {
    //            isCrouching = true;
    //        }
    //    }
    //}
    //
    //// If you are in the air, cancel slide
    //if (timeSinceOnGround > AIR_JUMP_TOLERANCE)
    //{
    //    slideTimer = -SLIDE_TIMER_COOLDOWN;
    //}
    //
    //// When sliding, maintain speed
    //if (slideTimer > 0.0f)
    //{
    //    vel.setLength(storedSlideSpeed);
    //}
    //
    //// Crouching
    //if (inputAction4)
    //{
    //    if ((slideTimer < 0.0f && vel.lengthSquared() < SLIDE_SPEED_REQUIRED*SLIDE_SPEED_REQUIRED) || (!onGround && slideTimer < 0.0f))
    //    {
    //        if (!isCrouching)
    //        {
    //            isCrouching = true;
    //            position.y += COLLISION_RADIUS*2;
    //            eyeHeightSmooth -= COLLISION_RADIUS*2;
    //        }
    //    }
    //}
    //else
    //{
    //    if (isCrouching)
    //    {
    //        CollisionResult result = CollisionChecker::checkCollision(position.x, position.y + COLLISION_RADIUS*3, position.z, COLLISION_RADIUS - 0.01f);
    //        if (!result.hit)
    //        {
    //            isCrouching = false;
    //
    //            if (!onGround)
    //            {
    //                //determine if we move the player back down ( so they cant get infinite height by spamming crouch
    //                float distanceToFloor = -1.0f;
    //                const int NUM_ITERATIONS = 200; //can probably reduce this, but makes things possibly choppier
    //                for (int i = 0; i < NUM_ITERATIONS; i++)
    //                {
    //                    float yOff = i*COLLISION_RADIUS*(2.0f/NUM_ITERATIONS);
    //                    result = CollisionChecker::checkCollision(position.x, (position.y + COLLISION_RADIUS) - yOff, position.z, COLLISION_RADIUS - 0.01f);
    //                    if (result.hit)
    //                    {
    //                        distanceToFloor = yOff;
    //                        break;
    //                    }
    //                }
    //
    //                if (distanceToFloor < 0.0f)
    //                {
    //                    position.y -= COLLISION_RADIUS*2;
    //                    eyeHeightSmooth += COLLISION_RADIUS*2;
    //                }
    //                else
    //                {
    //                    position.y -= distanceToFloor;
    //                    eyeHeightSmooth += distanceToFloor;
    //                }
    //            }
    //        }
    //    }
    //}
    //
    //// Player direction input
    //if (slideTimer < 0.0f)
    //{
    //    float stickAngle = atan2f(inputY, inputX) - Maths::PI/2; //angle you are holding on the stick, with 0 being up
    //    float stickRadius = sqrtf(inputX*inputX + inputY*inputY);
    //
    //    Vector3f dirForward = Maths::projectOntoPlane(&lookDir, &yAxis);
    //    dirForward.setLength(stickRadius);
    //
    //    Vector3f velToAdd = Maths::rotatePoint(&dirForward, &yAxis, stickAngle + Maths::PI);
    //
    //    if (onGround)
    //    {
    //        vel = vel + velToAdd.scaleCopy(getPushValueGround(dt)*dt);
    //    }
    //    else
    //    {
    //        vel = vel + velToAdd.scaleCopy(getPushValueAir(dt)*dt);
    //    }
    //}
    //
    //// Gravity
    //vel = vel + yAxis.scaleCopy(FORCE_GRAVITY*dt);
    //
    //ladderTimer -= dt;
    //
    //// Wall Jumping
    //wallJumpTimer-=dt;
    //if (!onGround && isTouchingWall)
    //{
    //    wallJumpTimer = WALL_JUMP_TIMER_MAX;
    //    storedWallNormal = wallNormal;
    //}
    //
    //isOnLadder = false;
    //
    //// Go through entities and move / get moved by entities
    //for (Entity* e : Global::gameEntities)
    //{
    //    switch (e->getEntityType())
    //    {
    //        case ENTITY_LADDER:
    //        {
    //            if (ladderTimer >= 0.0f)
    //            {
    //                break;
    //            }
    //
    //            Ladder* ladder = (Ladder*)e;
    //            Vector3f diff = e->position - position;
    //            if (fabsf(diff.x) < ladder->size.x + COLLISION_RADIUS &&
    //                fabsf(diff.z) < ladder->size.z + COLLISION_RADIUS &&
    //                position.y + HUMAN_HEIGHT > e->position.y &&
    //                position.y < e->position.y + ladder->size.y)
    //            {
    //                bool collided = false;
    //
    //                Vector3f center = position;
    //                center.y += COLLISION_RADIUS;
    //                float out1;
    //                Vector3f out2;
    //                for (Triangle3D* tri : ladder->cm->triangles)
    //                {
    //                    if (Maths::sphereIntersectsTriangle(&center, COLLISION_RADIUS, tri, &out1, &out2))
    //                    {
    //                        collided = true;
    //                        break;
    //                    }
    //                }
    //
    //                if (!collided && !isCrouching && slideTimer < 0.0f)
    //                {
    //                    center.y += COLLISION_RADIUS*2;
    //                    for (Triangle3D* tri : ladder->cm->triangles)
    //                    {
    //                        if (Maths::sphereIntersectsTriangle(&center, COLLISION_RADIUS, tri, &out1, &out2))
    //                        {
    //                            collided = true;
    //                            break;
    //                        }
    //                    }
    //                }
    //
    //                if (collided)
    //                {
    //                    float stickAngle = atan2f(inputY, inputX) - Maths::PI/2; //angle you are holding on the stick, with 0 being up
    //                    float stickRadius = sqrtf(inputX*inputX + inputY*inputY);
    //
    //                    Vector3f dirForward = lookDir;
    //                    dirForward.setLength(stickRadius);
    //
    //                    if (inputAction3)
    //                    {
    //                        dirForward.scale(5);
    //                    }
    //                    else
    //                    {
    //                        dirForward.scale(3);
    //                    }
    //
    //                    vel = Maths::rotatePoint(&dirForward, &Global::gameCamera->up, -stickAngle + Maths::PI);
    //
    //                    isOnLadder = true;
    //                }
    //            }
    //            break;
    //        }
    //
    //        default:
    //            break;
    //    }
    //}
    //
    //position = position + vel.scaleCopy(dt) + externalVel.scaleCopy(dt);
    //
    //bool hitAny = false;
    //std::vector<Triangle3D*> collisionResults;
    //
    //Vector3f velBefore = vel;
    //
    //Entity* previousCollideEntity = collideEntityImTouching;
    //collideEntityImTouching = nullptr;
    //
    //const float MIN_BOUNCE_SPD = 6.0f;
    //
    ////bottom sphere collision
    //for (int c = 0; c < 20; c++)
    //{
    //    Vector3f spotToTest = position;
    //    spotToTest.y += COLLISION_RADIUS;
    //    CollisionResult result = CollisionChecker::checkCollision(&spotToTest, COLLISION_RADIUS);
    //    if (result.hit)
    //    {
    //        //resolve this collision
    //        float distanceToMoveAway = COLLISION_RADIUS - result.distanceToPosition;
    //        Vector3f directionToMove = result.directionToPosition.scaleCopy(-1);
    //        spotToTest = spotToTest + directionToMove.scaleCopy(distanceToMoveAway);
    //        position = spotToTest;
    //        position.y -= COLLISION_RADIUS;
    //
    //        if (result.tri->type == 1)
    //        {
    //            vel = Maths::bounceVector(&vel, &result.tri->normal, 1.0f);
    //
    //            Vector3f velAlongLine = Maths::projectAlongLine(&vel, &result.tri->normal);
    //            if (velAlongLine.lengthSquared() < MIN_BOUNCE_SPD*MIN_BOUNCE_SPD)
    //            {
    //                vel = Maths::projectOntoPlane(&vel, &result.tri->normal);
    //                vel = vel + result.tri->normal.scaleCopy(MIN_BOUNCE_SPD);
    //            }
    //
    //            break;
    //        }
    //        else if (result.tri->type == 2)
    //        {
    //            die();
    //            break;
    //        }
    //
    //        //move along the new plane
    //        vel = Maths::projectOntoPlane(&vel, &directionToMove);
    //        hitAny = true;
    //        collisionResults.push_back(result.tri);
    //
    //        if (result.entity != nullptr)
    //        {
    //            collideEntityImTouching = result.entity;
    //        }
    //    }
    //    else
    //    {
    //        break;
    //    }
    //}
    //
    ////top sphere collision
    //if (!isCrouching && slideTimer <= 0.0f)
    //{
    //    for (int c = 0; c < 20; c++)
    //    {
    //        Vector3f spotToTest = position;
    //        spotToTest.y += COLLISION_RADIUS*3;
    //        CollisionResult result = CollisionChecker::checkCollision(&spotToTest, COLLISION_RADIUS);
    //        if (result.hit)
    //        {
    //            //resolve this collision
    //            float distanceToMoveAway = COLLISION_RADIUS - result.distanceToPosition;
    //            Vector3f directionToMove = result.directionToPosition.scaleCopy(-1);
    //            spotToTest = spotToTest + directionToMove.scaleCopy(distanceToMoveAway);
    //            position = spotToTest;
    //            position.y -= COLLISION_RADIUS*3;
    //
    //            if (result.tri->type == 1)
    //            {
    //                vel = Maths::bounceVector(&vel, &result.tri->normal, 1.0f);
    //                if (vel.lengthSquared() < MIN_BOUNCE_SPD*MIN_BOUNCE_SPD)
    //                {
    //                    vel = result.tri->normal;
    //                    vel.setLength(MIN_BOUNCE_SPD);
    //                }
    //
    //                break;
    //            }
    //            else if (result.tri->type == 2)
    //            {
    //                die();
    //                break;
    //            }
    //
    //            //move along the new plane
    //            vel = Maths::projectOntoPlane(&vel, &directionToMove);
    //
    //            if (result.entity != nullptr)
    //            {
    //                collideEntityImTouching = result.entity;
    //            }
    //        }
    //        else
    //        {
    //            break;
    //        }
    //    }
    //}
    //
    //if (collideEntityImTouching == nullptr && previousCollideEntity != nullptr)
    //{
    //    vel = vel + externalVel;
    //}
    //
    //externalVel.set(0, 0, 0);
    //
    //onGround = false;
    //isTouchingWall = false;
    //
    //bool touchedAWall = false;
    //if (hitAny)
    //{
    //    isTouchingWall = true;
    //
    //    Vector3f normalGroundSum;
    //    Vector3f normalWallSum;
    //
    //    for (Triangle3D* tri : collisionResults)
    //    {
    //        if (tri->normal.y > 0.5f) //dont add walls into this calculation, since we use it to determine jump direction.
    //        {
    //            normalGroundSum = normalGroundSum + tri->normal;
    //            onGround = true;
    //            isTouchingWall = false;
    //        }
    //        else
    //        {
    //            normalWallSum = normalWallSum + tri->normal;
    //            touchedAWall = true;
    //        }
    //    }
    //
    //    normalGroundSum.normalize();
    //    normalWallSum.normalize();
    //
    //    groundNormal = normalGroundSum;
    //    wallNormal = normalWallSum;
    //}
    //
    //// End the slide if contact with wall is too direct
    //if (touchedAWall && slideTimer > 0.0f)
    //{
    //    Vector3f velNorm = velBefore;
    //    velNorm.normalize();
    //
    //    if (velNorm.dot(&wallNormal) < -0.6f)
    //    {
    //        slideTimer = 0.0f;
    //
    //        if (inputAction4 || CollisionChecker::checkCollision(position.x, position.y + COLLISION_RADIUS*3, position.z, COLLISION_RADIUS - 0.01f).hit)
    //        {
    //            isCrouching = true;
    //        }
    //    }
    //}
    //
    //if (onGround)
    //{
    //    timeSinceOnGround = 0.0f;
    //    lastGroundNormal = groundNormal;
    //}
    //else
    //{
    //    timeSinceOnGround+=dt;
    //}
    //
    //if (onGround)
    //{
    //    if (vel.length() > 10.0f/36.7816091954f) //If going fast, slow down from drag equation
    //    {
    //        vel = Maths::applyDrag(&vel, -DRAG_GROUND, dt); //Slow vel down due to friction on ground
    //    }
    //    else //If going slow, slow down from linear equation
    //    {
    //        float down = 60*dt*(10.0f/36.7816091954f);
    //        if (vel.length() < down)
    //        {
    //            vel.scale(0.0000001f);
    //        }
    //        else
    //        {
    //            vel.setLength(vel.length() - down);
    //        }
    //    }
    //}
    //else
    //{
    //    float ySpd = vel.y;
    //    vel.y = 0;
    //    vel = Maths::applyDrag(&vel, -DRAG_AIR, dt); //Slow vel down due air drag, but just horizontally
    //    vel.y = ySpd;
    //}

    position = position + vel.scaleCopy(dt) + externalVel.scaleCopy(dt);

    visible = true;

    if (isCrouching)
    {
        Vector3f dir = lookDir;
        dir.y = 0;
        dir.setLength(0.081732f);

        head->position = position + dir;
        head->position.y += 0.839022f;

        head->visible = true;
    }
    else if (slideTimer > 0.0f)
    {
        head->visible = false;
    }
    else if (vel.y > 1.0f)
    {
        head->visible = false;
    }
    else if (vel.y < -1.0f)
    {
        head->visible = false;
    }
    else
    {
        Vector3f dir = lookDir;
        dir.y = 0;
        dir.setLength(0.024672f);

        head->position = position + dir;
        head->position.y += 1.52786f;

        head->visible = true;
    }

    Maths::sphereAnglesFromPosition(&lookDir, &rotY, &rotZ);
    head->rotZ = rotZ;
    head->rotY = rotY;

    rotZ = 0;

    //printf("%f\t%f\n", glfwGetTime(), position.x);
    //printf("dt = %f vel = %f pos = %f\n", dt, vel.x, position.x);

    updateTransformationMatrix();
    head->updateTransformationMatrix();

    if (health <= 0)
    {
        visible = false;
        head->visible = false;
    }

    Vector3f nametagPos = position;
    nametagPos.y += eyeHeightSmooth + 0.25f;

    Vector3f diff = Global::gameCamera->eye - nametagPos;

    Vector2f screenPos = Maths::calcScreenCoordsOfWorldPoint(&nametagPos);

    if (nametag == nullptr)
    {
        nametag = new GUIText(name, 0.1f, Global::fontConsolas, -0.5f, -0.5f, 4, true); INCR_NEW("GUIText");
    }

    if (health <= 0 || diff.lengthSquared() > 20*20)
    {
        nametag->visible = false;
    }
    else
    {
        //todo: enable this once checkCollision has been optimized for line segments

        CollisionResult result = CollisionChecker::checkCollision(&Global::gameCamera->eye, &nametagPos);
        nametag->visible = !result.hit;

        //nametag->visible = true;
    }

    nametag->fontSize = 0.15f / diff.length();
    nametag->position = screenPos;
}

std::vector<Entity*>* OnlinePlayer::getEntitiesToRender()
{
    return &entitiesToRender;
}

std::list<TexturedModel*>* OnlinePlayer::getModels()
{
    if (isCrouching)
    {
        return &OnlinePlayer::modelsSquat;
    }
    else if (slideTimer > 0.0f)
    {
        return &OnlinePlayer::modelsSlide;
    }
    else if (vel.y > 1.0f)
    {
        return &OnlinePlayer::modelsJump;
    }
    else if (vel.y < -1.0f)
    {
        return &OnlinePlayer::modelsFall;
    }

    return &OnlinePlayer::modelsStand;
}

int OnlinePlayer::getEntityType()
{
    return ENTITY_ONLINE_PLAYER;
}

void OnlinePlayer::getHit(Vector3f* hitPos, Vector3f* hitDir, int wpn)
{
    Message msg;
    msg.buf[0] = 4;

    int nameLen = (int)name.size();
    memcpy(&msg.buf[1], &nameLen, 4);
    memcpy(&msg.buf[5], name.c_str(), nameLen);

    memcpy(&msg.buf[ 5 + nameLen], &hitPos->x, 4);
    memcpy(&msg.buf[ 9 + nameLen], &hitPos->y, 4);
    memcpy(&msg.buf[13 + nameLen], &hitPos->z, 4);
    memcpy(&msg.buf[17 + nameLen], &hitDir->x, 4);
    memcpy(&msg.buf[21 + nameLen], &hitDir->y, 4);
    memcpy(&msg.buf[25 + nameLen], &hitDir->z, 4);

    char dub = (char)wpn;
    memcpy(&msg.buf[29 + nameLen], &dub, 1);

    msg.length = 1 + 4 + nameLen + 24 + 1;

    Global::sendMessageToServer(msg);

    switch (wpn)
    {
        case WEAPON_FIST: AudioPlayer::play(75, nullptr); break;
        case WEAPON_BAT:  AudioPlayer::play(74, nullptr); break;
        case WEAPON_GUN:  AudioPlayer::play(74, nullptr); break;
        default: break;
    }
}

float OnlinePlayer::getPushValueGround(float deltaTime)
{
    double fps = 1.0/deltaTime;

    double val;

    if (inputAction3) //Running
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

float OnlinePlayer::getPushValueAir(float deltaTime)
{
    double fps = 1.0/deltaTime;
    return (float)(9.917682 + (14.17576 - 9.917682)/(1.0 + pow(fps/1.386981, 1.042293)));
}

void OnlinePlayer::updateCamera()
{
    Vector3f yAxis(0, 1, 0);

    inputX2 = 0;
    inputY2 = 0;

    lookDir = Maths::rotatePoint(&lookDir, &yAxis, -inputX2);
    lookDir.normalize();

    Vector3f perpen = lookDir.cross(&yAxis);
    perpen.normalize();

    const float maxViewAng = Maths::toRadians(0.1f);

    if (inputY2 > 0) //mouse is going down
    {
        Vector3f down(0, -1, 0);
        float angBetweenCamAndDown = Maths::angleBetweenVectors(&lookDir, &down);
        float angToRotate = inputY2;
        if (angBetweenCamAndDown - angToRotate > maxViewAng)
        {
            lookDir = Maths::rotatePoint(&lookDir, &perpen, -inputY2);
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
        float angToRotate = -inputY2;
        if (angBetweenCamAndUp - angToRotate > maxViewAng)
        {
            lookDir = Maths::rotatePoint(&lookDir, &perpen, -inputY2);
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
}

void OnlinePlayer::loadModels()
{
    if (OnlinePlayer::modelsStand.size() == 0)
    {
        ObjLoader::loadModel(&OnlinePlayer::modelsHead , "res/Models/Human/", "ShrekHead");
        ObjLoader::loadModel(&OnlinePlayer::modelsFall , "res/Models/Human/", "ShrekFall");
        ObjLoader::loadModel(&OnlinePlayer::modelsJump , "res/Models/Human/", "ShrekJump");
        ObjLoader::loadModel(&OnlinePlayer::modelsSlide, "res/Models/Human/", "ShrekSlide");
        ObjLoader::loadModel(&OnlinePlayer::modelsSquat, "res/Models/Human/", "ShrekSquat");
        ObjLoader::loadModel(&OnlinePlayer::modelsStand, "res/Models/Human/", "ShrekStand");
    }
}
