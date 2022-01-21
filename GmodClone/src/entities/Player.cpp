#include <list>
#include <set>
#include <glad/glad.h>
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
#include "ladder.hpp"
#include "onlineplayer.hpp"
#include "../loader/objloader.hpp"
#include "dummy.hpp"
#include "../guis/guimanager.hpp"
#include "../guis/guitextureresources.hpp"
#include "../guis/guitexture.hpp"
#include "../network/tcpclient.hpp"

extern float dt;

std::list<TexturedModel*> modelsGun;
std::list<TexturedModel*> modelsBat;

Player::Player()
{
    scale = 0.5f;
    position.set(-4.85623f, 1.5322f, -12.5412f);
    vel.set(0.000001f, 0, 0);
    groundNormal.set(0, 1, 0);
    lastGroundNormal.set(0, 1, 0);
    wallNormal.set(1, 0, 0);
    lookDir.set(0, 0, -1);
    visible = false;

    ObjLoader::loadModel(&modelsGun , "res/Models/Gun/", "GunInHand");
    ObjLoader::loadModel(&modelsBat , "res/Models/Bat/", "Bat");

    weaponModel = new Dummy(&modelsGun); INCR_NEW("Dummy");
    weaponModel->visible = false;

    entitiesToRender.push_back(weaponModel);
}

Player::~Player() //this should never actually get called
{
    if (weaponModel != nullptr)
    {
        delete weaponModel; INCR_DEL("Dummy");
        weaponModel = nullptr;
    }
}

void Player::step()
{
    Vector3f yAxis(0, -1, 0);

    if (Global::levelId == LVL_MAP5 || Global::levelId == LVL_MAP4 || Global::levelId == LVL_MAP7)
    {
        if (Global::timeUntilRoundEnds > 0.0f)
        {
            if (position.y < Global::safeZoneStart.y - 2.0f && health > 0)
            {
                health = 0;
                AudioPlayer::play(66, nullptr);
                Global::sendAudioMessageToServer(66, &position);
            }
        }
    }

    if (health <= 0 && Global::levelId == LVL_HUB)
    {
        health = 100;
    }

    //If we are dead, fly around as a ghost
    if (health <= 0)
    {
        float stickAngle = atan2f(Input::inputs.INPUT_Y, Input::inputs.INPUT_X) - Maths::PI/2; //angle you are holding on the stick, with 0 being up
        float stickRadius = sqrtf(Input::inputs.INPUT_X*Input::inputs.INPUT_X + Input::inputs.INPUT_Y*Input::inputs.INPUT_Y);

        Vector3f perpen = lookDir.cross(&yAxis);
        Vector3f actualUp = Maths::rotatePoint(&lookDir, &perpen, Maths::PI/2);
        Vector3f toMove = Maths::rotatePoint(&lookDir, &actualUp, stickAngle + Maths::PI);

        toMove.setLength(stickRadius*dt);

        if (Input::inputs.INPUT_ACTION3)
        {
            toMove.scale(20.0f);
        }
        else
        {
            toMove.scale(8.0f);
        }

        position = position + toMove;

        updateCamera();

        GuiManager::addGuiToRender(GuiTextureResources::textureHealthbarBG);
        weaponModel->visible = false;

        vel.set(0, 0, 0);

        return;
    }

    if (Input::inputs.INPUT_SCROLL != 0)
    {
        if (Global::levelId == LVL_HUB)
        {
            AudioPlayer::play(56, nullptr);

            weapon += (char)Input::inputs.INPUT_SCROLL;
            weapon = weapon % 3;
            if (weapon < 0)
            {
                weapon = 2;
            }
        }
    }

    useWeaponTimer = fmaxf(0.0f, useWeaponTimer - dt);
    if (Input::inputs.INPUT_LEFT_CLICK && !Input::inputs.INPUT_PREVIOUS_LEFT_CLICK && useWeaponTimer <= 0.0f)
    {
        useWeapon();
    }

    // Sliding
    float slideTimerBefore = slideTimer;
    slideTimer-=dt;
    if (Input::inputs.INPUT_ACTION4 && !Input::inputs.INPUT_PREVIOUS_ACTION4)
    {
        if (timeSinceOnGround <= AIR_JUMP_TOLERANCE && !isCrouching && vel.lengthSquared() > SLIDE_SPEED_REQUIRED*SLIDE_SPEED_REQUIRED && slideTimer <= -SLIDE_TIMER_COOLDOWN)
        {
            AudioPlayer::play(1, nullptr);
            Global::sendAudioMessageToServer(1, &position);
            slideTimer = SLIDE_TIMER_DURATION;
            storedSlideSpeed = vel.length() + SLIDE_SPEED_ADDITION;
            vel = Maths::projectOntoPlane(&vel, &groundNormal);
            animTimerSlide = 0.0f;
        }
    }

    // If holding crouch after slide is done, start crouching
    if (slideTimer <= 0.0f && slideTimerBefore > 0.0f)
    {
        if (Input::inputs.INPUT_ACTION4)
        {
            isCrouching = true;
        }
        else
        {
            CollisionResult result = CollisionChecker::checkCollision(position.x, position.y + COLLISION_RADIUS*3, position.z, COLLISION_RADIUS - 0.01f);
            if (result.hit)
            {
                isCrouching = true;
            }
        }
    }

    // If you are in the air, cancel slide
    if (timeSinceOnGround > AIR_JUMP_TOLERANCE)
    {
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
            CollisionResult result = CollisionChecker::checkCollision(position.x, position.y + COLLISION_RADIUS*3, position.z, COLLISION_RADIUS - 0.01f);
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
                        result = CollisionChecker::checkCollision(position.x, (position.y + COLLISION_RADIUS) - yOff, position.z, COLLISION_RADIUS - 0.01f);
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

    ladderTimer -= dt;

    bool alreadyJumpedThisFrame = false;

    // Normal Jump
    if (Input::inputs.INPUT_ACTION1 && !Input::inputs.INPUT_PREVIOUS_ACTION1)
    {
        bool canJump = false;

        if (timeSinceOnGround <= AIR_JUMP_TOLERANCE)
        {
            canJump = true;

            // Don't jump if there is a ceiling above you
            if (slideTimer > 0.0f || isCrouching)
            {
                CollisionResult result = CollisionChecker::checkCollision(position.x, position.y + COLLISION_RADIUS*3, position.z, COLLISION_RADIUS - 0.01f);
                canJump = !result.hit;
            }
        }
        else if (isOnLadder && ladderTimer < 0.0f)
        {
            canJump = true;
            ladderTimer = LADDER_JUMP_COOLDOWN;
            lastGroundNormal.set(0, 1, 0);
            onGround = false;
            isTouchingWall = false;
            //didLadderJump = true;
        }

        if (canJump)
        {
            AudioPlayer::play(60, nullptr);
            Global::sendAudioMessageToServer(60, &position);
            vel = vel + lastGroundNormal.scaleCopy(getJumpValue(dt));
            timeSinceOnGround = AIR_JUMP_TOLERANCE + 0.0001f;
            alreadyJumpedThisFrame = true;
            animTimerJump = 0.0f;
            //Global::addChatMessage("Jump at " + std::to_string(position.x) + ", " + std::to_string(position.y) + ", " + std::to_string(position.z), Vector3f(1,1,1));
        }
    }

    // Wall Jumping
    wallJumpTimer-=dt;
    if (!onGround && isTouchingWall)
    {
        wallJumpTimer = WALL_JUMP_TIMER_MAX;
        storedWallNormal = wallNormal;
    }

    if (!onGround && !alreadyJumpedThisFrame && Input::inputs.INPUT_ACTION1 && !Input::inputs.INPUT_PREVIOUS_ACTION1)
    {
        bool canWallJump = wallJumpTimer >= 0.0f;
        if (!canWallJump)
        {
            CollisionResult result = CollisionChecker::checkCollision(position.x + 0.03f, position.y + COLLISION_RADIUS, position.z, COLLISION_RADIUS);
            if (result.hit && result.tri->normal.y <= 0.5f)
            {
                latestWallTriangle = result.tri;
                storedWallNormal = result.tri->normal;
                canWallJump = true;
            }
        }
        if (!canWallJump)
        {
            CollisionResult result = CollisionChecker::checkCollision(position.x - 0.03f, position.y + COLLISION_RADIUS, position.z, COLLISION_RADIUS);
            if (result.hit && result.tri->normal.y <= 0.5f)
            {
                latestWallTriangle = result.tri;
                storedWallNormal = result.tri->normal;
                canWallJump = true;
            }
        }
        if (!canWallJump)
        {
            CollisionResult result = CollisionChecker::checkCollision(position.x, position.y + COLLISION_RADIUS, position.z + 0.03f, COLLISION_RADIUS);
            if (result.hit && result.tri->normal.y <= 0.5f)
            {
                latestWallTriangle = result.tri;
                storedWallNormal = result.tri->normal;
                canWallJump = true;
            }
        }
        if (!canWallJump)
        {
            CollisionResult result = CollisionChecker::checkCollision(position.x, position.y + COLLISION_RADIUS, position.z - 0.03f, COLLISION_RADIUS);
            if (result.hit && result.tri->normal.y <= 0.5f)
            {
                latestWallTriangle = result.tri;
                storedWallNormal = result.tri->normal;
                canWallJump = true;
            }
        }

        if (canWallJump)
        {
            int sfxId = 43;
            if (latestWallTriangle != nullptr)
            {
                sfxId = getSoundEffectImpact(latestWallTriangle->sound);
            }
            AudioPlayer::play(sfxId, nullptr);
            Global::sendAudioMessageToServer(sfxId, &position);

            vel = vel + storedWallNormal.scaleCopy(WALL_JUMP_SPEED_HORIZONTAL);
            vel.y += getJumpValue(dt) - 1.0f;
            wallJumpTimer = -1.0f;

            animTimerJump = 0.0f;
        }
    }

    isOnLadder = false;

    // Go through online players and get pushed by them
    Global::gameOnlinePlayersSharedMutex.lock_shared();
    for (auto const& player : Global::gameOnlinePlayers)
    {
        OnlinePlayer* e = player.second;
        if (e->health > 0)
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
        }
    }
    Global::gameOnlinePlayersSharedMutex.unlock_shared();

    // Go through entities and interact with them
    for (Entity* e : Global::gameEntities)
    {
        switch (e->getEntityType())
        {
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

            case ENTITY_LADDER:
            {
                if (ladderTimer >= 0.0f)
                {
                    break;
                }

                Ladder* ladder = (Ladder*)e;
                Vector3f diff = e->position - position;
                if (fabsf(diff.x) < ladder->size.x + COLLISION_RADIUS &&
                    fabsf(diff.z) < ladder->size.z + COLLISION_RADIUS &&
                    position.y + HUMAN_HEIGHT > e->position.y &&
                    position.y < e->position.y + ladder->size.y)
                {
                    bool collided = false;

                    Vector3f center = position;
                    center.y += COLLISION_RADIUS;
                    float out1;
                    Vector3f out2;
                    for (Triangle3D* tri : ladder->cm->triangles)
                    {
                        if (Maths::sphereIntersectsTriangle(&center, COLLISION_RADIUS, tri, &out1, &out2))
                        {
                            collided = true;
                            break;
                        }
                    }

                    if (!collided && !isCrouching && slideTimer < 0.0f)
                    {
                        center.y += COLLISION_RADIUS*2;
                        for (Triangle3D* tri : ladder->cm->triangles)
                        {
                            if (Maths::sphereIntersectsTriangle(&center, COLLISION_RADIUS, tri, &out1, &out2))
                            {
                                collided = true;
                                break;
                            }
                        }
                    }

                    if (collided)
                    {
                        float stickAngle = atan2f(Input::inputs.INPUT_Y, Input::inputs.INPUT_X) - Maths::PI/2; //angle you are holding on the stick, with 0 being up
                        float stickRadius = sqrtf(Input::inputs.INPUT_X*Input::inputs.INPUT_X + Input::inputs.INPUT_Y*Input::inputs.INPUT_Y);

                        Vector3f dirForward = lookDir;
                        dirForward.setLength(stickRadius);

                        if (Input::inputs.INPUT_ACTION3)
                        {
                            dirForward.scale(5);
                        }
                        else
                        {
                            dirForward.scale(3);
                        }

                        vel = Maths::rotatePoint(&dirForward, &Global::gameCamera->up, -stickAngle + Maths::PI);

                        isOnLadder = true;
                    }
                }
                break;
            }

            case ENTITY_HEALTH_CUBE:
            {
                if (e->visible && health < 100 && health > 0 && !Global::serverClient->isOpen()) //if we are in a server, the server decides if we pick up the item
                {
                    Vector3f mySpot = position;
                    mySpot.y += COLLISION_RADIUS;

                    Vector3f diff = mySpot - e->position;

                    if (diff.lengthSquared() < (COLLISION_RADIUS + 0.3f)*(COLLISION_RADIUS + 0.3f))
                    {
                        health = (char)Maths::clamp(0, health + 25, 100);
                        e->visible = false;
                        AudioPlayer::play(61, nullptr);
                    }
                }
                break;
            }

            case ENTITY_BAT:
            {
                if (e->visible && health > 0 && weapon == WEAPON_FIST && !Global::serverClient->isOpen()) //if we are in a server, the server decides if we pick up the item
                {
                    Vector3f mySpot = position;
                    mySpot.y += COLLISION_RADIUS;

                    Vector3f diff = mySpot - e->position;

                    if (diff.lengthSquared() < (COLLISION_RADIUS + 0.3f)*(COLLISION_RADIUS + 0.3f))
                    {
                        weapon = WEAPON_BAT;
                        e->visible = false;
                        //TODO audio for picking up item
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

            if (result.tri->normal.y > 0.5f)
            {
                latestGroundTriangle = result.tri;
            }
            else
            {
                latestWallTriangle = result.tri;
            }

            if (result.tri->type == 1)
            {
                vel = Maths::bounceVector(&vel, &result.tri->normal, 1.0f);

                Vector3f velAlongLine = Maths::projectAlongLine(&vel, &result.tri->normal);
                if (velAlongLine.lengthSquared() < MIN_BOUNCE_SPD*MIN_BOUNCE_SPD)
                {
                    vel = Maths::projectOntoPlane(&vel, &result.tri->normal);
                    vel = vel + result.tri->normal.scaleCopy(MIN_BOUNCE_SPD);
                }
                int sfxId = (int)(2*Maths::random()) + 57;
                AudioPlayer::play(sfxId, nullptr);
                Global::sendAudioMessageToServer(sfxId, &position);
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
    if (!isCrouching && slideTimer <= 0.0f)
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

                if (result.tri->normal.y > 0.5f)
                {
                    latestGroundTriangle = result.tri;
                }
                else
                {
                    latestWallTriangle = result.tri;
                }

                if (result.tri->type == 1)
                {
                    vel = Maths::bounceVector(&vel, &result.tri->normal, 1.0f);
                    if (vel.lengthSquared() < MIN_BOUNCE_SPD*MIN_BOUNCE_SPD)
                    {
                        vel = result.tri->normal;
                        vel.setLength(MIN_BOUNCE_SPD);
                    }
                    int sfxId = (int)(2*Maths::random()) + 57;
                    AudioPlayer::play(sfxId, nullptr);
                    Global::sendAudioMessageToServer(sfxId, &position);
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
    externalVelPrev = externalVel;
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

            if (Input::inputs.INPUT_ACTION4 || CollisionChecker::checkCollision(position.x, position.y + COLLISION_RADIUS*3, position.z, COLLISION_RADIUS - 0.01f).hit)
            {
                isCrouching = true;
            }
        }
    }

    //Calculate landing on the ground
    landSoundTimer+=dt;
    if (!onGroundBefore && onGround)
    {
        float impactSpeed = Maths::projectAlongLine(&velBefore, &groundNormal).length();

        if (impactSpeed > FALL_DAMAGE_SPEED_START)
        {
            float perc = (impactSpeed - FALL_DAMAGE_SPEED_START)/(FALL_DAMAGE_SPEED_FATAL - FALL_DAMAGE_SPEED_START);

            int newHealth = (int)health;

            newHealth -= (int)(perc*100);

            if (newHealth < 0)
            {
                newHealth = 0;
            }
            health = (char)newHealth;

            if (landSoundTimer > 0.2f)
            {
                AudioPlayer::play(51, nullptr);
                Global::sendAudioMessageToServer(51, &position);
            }
        }
        else if (impactSpeed > 4.0f)
        {
            if (landSoundTimer > 0.2f)
            {
                int sfxId = 43;
                if (latestGroundTriangle != nullptr)
                {
                    sfxId = getSoundEffectImpact(latestGroundTriangle->sound);
                }
                AudioPlayer::play(sfxId, nullptr);
                Global::sendAudioMessageToServer(sfxId, &position);
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
            int sfxId = 8;
            if (latestGroundTriangle != nullptr)
            {
                sfxId = getSoundEffectFootstep(latestGroundTriangle->sound);
            }
            AudioPlayer::play(sfxId, nullptr);
            Global::sendAudioMessageToServer(sfxId, &position);
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
        //Ball* b = new Ball("Ball0", position, lookDir.scaleCopy(10)); INCR_NEW("Entity");
        //b->position.y += HUMAN_HEIGHT;
        //Global::addEntity(b);
    }

    extern float VFOV_ADDITION;
    extern float VFOV_BASE;
    if (Input::inputs.INPUT_RIGHT_CLICK && weapon == WEAPON_GUN)
    {
        VFOV_ADDITION = -VFOV_BASE/2;
    }
    else
    {
        VFOV_ADDITION = 0;
    }

    if (Global::timeUntilRoundStarts > 5.0f)
    {
        if (Global::levelId == LVL_MAP4 ||
            Global::levelId == LVL_MAP5 ||
            Global::levelId == LVL_MAP6 ||
            Global::levelId == LVL_MAP7)
        {
            lookDir = position.scaleCopy(-1);
            lookDir.y = 0;
            lookDir.normalize();
        }

        Input::inputs.INPUT_X2 = 0;
        Input::inputs.INPUT_Y2 = 0;
    }

    updateCamera();

    Vector3f xAx(1, 0, 0);
    Vector3f yAx(0, 1, 0);
    Vector3f zAx(0, 0, 1);

    switch (weapon)
    {
        case WEAPON_FIST:
        {
            weaponModel->visible = false;
            break;
        }

        case WEAPON_BAT:
        {
            weaponModel->visible = true;
            weaponModel->setModels(&modelsBat);
            weaponModel->position = Global::gameCamera->eye;

            Maths::sphereAnglesFromPosition(&lookDir, &weaponModel->rotY, &weaponModel->rotZ);

            Matrix4f* mat = &weaponModel->transformationMatrix;

            mat->setIdentity();
            mat->translate(&weaponModel->position);

            mat->rotate(Maths::toRadians(weaponModel->rotY), &yAx);
            mat->rotate(Maths::toRadians(weaponModel->rotZ), &zAx);

            Vector3f armOffset(0.25f, -0.4f, 0.2f);
            mat->translate(&armOffset);

            float batRot = 0.0f;
            if (useWeaponTimer > WEAPON_COOLDOWN_BAT - 0.07f)
            {
                float prog = -((useWeaponTimer - WEAPON_COOLDOWN_BAT)/0.07f);
                batRot = -prog*80;
            }
            else
            {
                float prog = useWeaponTimer/0.33f;
                batRot = -prog*80;
            }

            mat->rotate(Maths::toRadians(28), &xAx);
            mat->rotate(Maths::toRadians(70 + batRot), &zAx);

            break;
        }

        case WEAPON_GUN:
        {
            weaponModel->visible = true;
            weaponModel->setModels(&modelsGun);
            weaponModel->position = Global::gameCamera->eye;

            Maths::sphereAnglesFromPosition(&lookDir, &weaponModel->rotY, &weaponModel->rotZ);

            Matrix4f* mat = &weaponModel->transformationMatrix;

            mat->setIdentity();
            mat->translate(&weaponModel->position);

            mat->rotate(Maths::toRadians(weaponModel->rotY), &yAx);
            mat->rotate(Maths::toRadians(weaponModel->rotZ), &zAx);

            Vector3f armOffset(-0.103481f, -0.158837f, 0.132962f);
            if (useWeaponTimer > WEAPON_COOLDOWN_GUN - 0.06f)
            {
                armOffset.x += (useWeaponTimer - WEAPON_COOLDOWN_GUN)*0.3f;
            }
            mat->translate(&armOffset);

            break;
        }

        default: break;
    }

    if (health < 100)
    {
        float perc = (float)health/100.0f;
        GuiTextureResources::textureHealthbar->size.x = perc*(0.425f);
        GuiManager::addGuiToRender(GuiTextureResources::textureHealthbarBG);
        GuiManager::addGuiToRender(GuiTextureResources::textureHealthbar);
    }

    updateTransformationMatrix();

    float animSpd = vel.length();

    char animTypeNew = 0;

    if (isCrouching)
    {
        animTypeNew = 3;
        animTimerCrouch += animSpd*dt;
    }
    else if (slideTimer > 0.0f)
    {
        animTypeNew = 4;
        animTimerSlide += dt;
    }
    else if (timeSinceOnGround <= 0.02f) //on ground
    {
        if (animSpd < 0.2f) //stand
        {
            animTypeNew = 0;
            animTimerStand += dt;
        }
        else if (animSpd < 6.0f) //walk
        {
            animTypeNew = 1;
            //animTimerWalk += animSpd*dt;
            animTimerRun += 2.25f*animSpd*dt; //use the same timer for both run and walk
        }
        else //run
        {
            animTypeNew = 2;
            animTimerRun += 2.25f*animSpd*dt;
        }
    }
    else //in air
    {
        if (vel.y > 0.0f)
        {
            animTypeNew = 5;
            animTimerJump += dt;
        }
        else
        {
            animTypeNew = 6;
            animTimerFall += dt;
        }
    }

    if (animTypeNew != 6)
    {
        animTimerFall = 0.0f;
    }

    if (animType != animTypeNew)
    {
        animTypePrevious = animType;
        animType = animTypeNew;
        animBlend = 0.0f;
    }
    else
    {
        animBlend += 10*dt;
    }
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

    float inputX2 = Input::inputs.INPUT_X2;
    float inputY2 = Input::inputs.INPUT_Y2;

    if (Input::inputs.INPUT_RIGHT_CLICK && weapon == WEAPON_GUN)
    {
        inputX2*=0.5f;
        inputY2*=0.5f;
    }

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

    Vector3f eye(&position);
    eye.y += eyeHeightSmooth;

    Vector3f target(&eye);
    target = target + lookDir;

    Vector3f up = Maths::rotatePoint(&lookDir, &perpen, Maths::PI/2);
    up.normalize();

    Global::gameCamera->setViewMatrixValues(&eye, &target, &up);
}

void Player::useWeapon()
{
    Vector3f camDir = Global::gameCamera->target - Global::gameCamera->eye;

    switch (weapon)
    {
        case WEAPON_FIST: useWeaponTimer = WEAPON_COOLDOWN_FIST; camDir.setLength(WEAPON_REACH_FIST); break;
        case WEAPON_BAT : useWeaponTimer = WEAPON_COOLDOWN_BAT ; camDir.setLength(WEAPON_REACH_BAT ); break;
        case WEAPON_GUN : useWeaponTimer = WEAPON_COOLDOWN_GUN ; camDir.setLength(WEAPON_REACH_GUN ); break;
        default: break;
    }

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

    Global::gameOnlinePlayersSharedMutex.lock_shared();
    for (auto const& pair : Global::gameOnlinePlayers)
    {
        Entity* e = pair.second;

        Vector3f otherHead = e->position;
        OnlinePlayer* onlinePlayer = (OnlinePlayer*)e;
        if (onlinePlayer->isCrouching || onlinePlayer->slideTimer > 0.0f)
        {
            otherHead.y += COLLISION_RADIUS*2;
        }
        else
        {
            otherHead.y += COLLISION_RADIUS*4;
        }

        if (onlinePlayer->health <= 0)
        {
            continue;
        }

        Vector3f playerCollisionSpot;
        if (Maths::lineSegmentIntersectsCylinder(&Global::gameCamera->eye, &target, &e->position, &otherHead, COLLISION_RADIUS, &playerCollisionSpot))
        {
            float thisDistSquared = (Global::gameCamera->eye - playerCollisionSpot).lengthSquared();
            if (thisDistSquared < distToCollisionSquared)
            {
                distToCollisionSquared = thisDistSquared;
                hitEntity = e;
            }
        }
    }
    Global::gameOnlinePlayersSharedMutex.unlock_shared();

    //printf("distToCollision = %f\n", sqrtf(distToCollisionSquared));

    if (hitEntity != nullptr)
    {
        camDir.normalize();
        Vector3f hitPosition = Global::gameCamera->eye + camDir.scaleCopy(sqrtf(distToCollisionSquared));
        hitEntity->getHit(&hitPosition, &camDir, weapon);
    }
    else if (result.hit)
    {
        int sfxId = getSoundEffectImpact(result.tri->sound);
        AudioPlayer::play(sfxId, nullptr);
        Global::sendAudioMessageToServer(sfxId, &position);
    }
    else //miss
    {
        AudioPlayer::play(52, nullptr);
        Global::sendAudioMessageToServer(52, &position);
    }
}

int Player::getSoundEffectFootstep(int soundType)
{
    switch (soundType)
    {
        case 0: return  8 + (int)(Maths::random()*5);
        case 1: return 13 + (int)(Maths::random()*4);
        case 2: return 17 + (int)(Maths::random()*4);
        case 3: return 21 + (int)(Maths::random()*5);
        case 4: return 26 + (int)(Maths::random()*5);
        case 5: return 31 + (int)(Maths::random()*5);
        case 6: return 36 + (int)(Maths::random()*4);
        case 7: return 68 + (int)(Maths::random()*6);
        default: break;
    }

    return 8;
}

int Player::getSoundEffectImpact(int soundType)
{
    switch (soundType)
    {
        case 0: return 43;
        case 1: return 44;
        case 2: return 45;
        case 3: return 46;
        case 4: return 47;
        case 5: return 48;
        case 6: return 49;
        case 7: return 68;
        default: break;
    }

    return 43;
}

std::vector<Entity*>* Player::getEntitiesToRender()
{
    return &entitiesToRender;
}

std::list<TexturedModel*>* Player::getModels()
{
    return nullptr;
}

void Player::die()
{
    if (health > 0)
    {
        health = 0;
        AudioPlayer::play(66, nullptr);
        Global::sendAudioMessageToServer(66, &position);
    }
}

void Player::reset()
{
    latestGroundTriangle = nullptr;
    latestWallTriangle = nullptr;

    onGround = false;

    isTouchingWall = false;

    stepTimer = 0.0f;
    landSoundTimer = 0.0f;

    slideTimer = 0.0f;
    storedSlideSpeed = 0.0f;

    wallJumpTimer = 0.0f;
    storedWallNormal.set(1, 0, 0);

    eyeHeightSmooth = HUMAN_HEIGHT;

    timeSinceOnGround = 0.0f;
    lastGroundNormal.set(0, 1, 0);

    useWeaponTimer = 0.0f;

    ladderTimer = 0.0f;
    isOnLadder = false;

    collideEntityImTouching = nullptr;

    isCrouching = false;

    weapon = WEAPON_FIST;

    inZoneTime = 0.0f;

    vel.set(0, 0, 0);
    groundNormal.set(0, 1, 0);
    lastGroundNormal.set(0, 1, 0);
    wallNormal.set(1, 0, 0);
    lookDir.set(0, 0, -1);
    externalVel.set(0, 0, 0);
    externalVelPrev.set(0, 0, 0);
    visible = false;
    weaponModel->visible = false;
}
