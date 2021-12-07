#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <AL/al.h>
#include <list>

#include <chrono>
#include <thread>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "levelloader.hpp"
#include "../main/main.hpp"
#include "../collision/collisionchecker.hpp"
#include "../collision/collisionmodel.hpp"
#include "../audio/audioplayer.hpp"
#include "../guis/guimanager.hpp"
#include "../toolbox/getline.hpp"
#include "../toolbox/maths.hpp"
#include "../toolbox/split.hpp"
#include "../toolbox/input.hpp"
#include "../entities/camera.hpp"
#include "../entities/light.hpp"
#include "../entities/player.hpp"
#include "../entities/ball.hpp"
#include "../entities/collisionblock.hpp"
#include "../entities/entity.hpp"
#include "../entities/glass.hpp"
#include "../entities/healthcube.hpp"
#include "../entities/ladder.hpp"
#include "../entities/redbarrel.hpp"
#include "../loader/objloader.hpp"
#include "../entities/boombox.hpp"
#include "../entities/rockplatform.hpp"
#include "../network/tcpclient.hpp"
#include "../entities/chandelier.hpp"
#include "../entities/fenceplatform.hpp"
#include "../entities/stepfallplatform.hpp"

void LevelLoader::loadLevel(std::string mapName)
{
    //convert name to lowercase name
    char nameLower[32] = {0};
    if ((int)mapName.size() < 32)
    {
        for (int i = 0; i < (int)mapName.size(); i++)
        {
            char c = mapName[i];

            if (c >= 65 && c <= 90)
            {
                c += 32;
            }

            nameLower[i] = c;
        }
    }

    std::string lower = nameLower;
    std::string fname = lower + ".map";

    std::ifstream file(Global::pathToEXE + "res/Maps/" + fname);
    if (!file.is_open())
    {
        printf("Error: Cannot load file '%s'\n", (Global::pathToEXE + "res/Maps/" + fname).c_str());
        file.close();
        std::string msg = "Can't load map '" + fname + "'";
        Global::addChatMessage(msg, Vector3f(1, 0.5f, 0.5f));
        return;
    }

    if      (fname == "hub.map")  Global::levelId = LVL_HUB;
    else if (fname == "map1.map") Global::levelId = LVL_MAP1;
    else if (fname == "map2.map") Global::levelId = LVL_MAP2;
    else if (fname == "map3.map") Global::levelId = LVL_MAP3;
    else if (fname == "eq.map")   Global::levelId = LVL_EQ;
    else if (fname == "map4.map") Global::levelId = LVL_MAP4;
    else if (fname == "test.map") Global::levelId = LVL_TEST;
    else if (fname == "map5.map") Global::levelId = LVL_MAP5;

    Global::deleteAllEntites();

    if (Global::player->health > 0)
    {
        Global::player->health = 100;
    }
    else if (Global::levelId == LVL_TEST)
    {
        Global::player->health = 100;
    }
    Global::player->reset();

    Global::timeUntilRoundStarts = 7.0f;

    std::chrono::high_resolution_clock::time_point timeStart = std::chrono::high_resolution_clock::now();
    bool waitForSomeTime = true;

    //Reset all lights except sun
    Global::lights[1]->attenuation.set(10000000, 1, 1);
    Global::lights[2]->attenuation.set(10000000, 1, 1);
    Global::lights[3]->attenuation.set(10000000, 1, 1);

    Global::lights[1]->color.set(1, 1, 1);
    Global::lights[2]->color.set(1, 1, 1);
    Global::lights[3]->color.set(1, 1, 1);

    //Run through the header content

    std::string modelVisualLine;
    getlineSafe(file, modelVisualLine);
    std::vector<std::string> visualModel = split(modelVisualLine, ' ');
    Entity::deleteModels(&Global::stageModels);
    ObjLoader::loadModel(&Global::stageModels, visualModel[0], visualModel[1]);

    std::string modelCollisionLine;
    getlineSafe(file, modelCollisionLine);
    std::vector<std::string> collisionModel = split(modelCollisionLine, ' ');
    CollisionChecker::deleteAllTriangles();
    CollisionModel* cm = ObjLoader::loadCollisionModel(collisionModel[0], collisionModel[1]);
    for (int i = 0; i < (int)cm->triangles.size(); i++)
    {
        CollisionChecker::addTriangle(cm->triangles[i]);
    }
    CollisionChecker::constructChunkDatastructure();
    delete cm; INCR_DEL("CollisionModel");

    std::string playerSpawnZoneLine;
    getlineSafe(file, playerSpawnZoneLine);
    std::vector<std::string> spawnZones = split(playerSpawnZoneLine, ' ');
    Vector3f spawnStart = Vector3f(toF(spawnZones[0]), toF(spawnZones[1]), toF(spawnZones[2]));
    Vector3f spawnEnd   = Vector3f(toF(spawnZones[3]), toF(spawnZones[4]), toF(spawnZones[5]));
    Vector3f spawnZone  = spawnEnd - spawnStart;
    Global::player->position = spawnStart;
    Global::player->position.x += Maths::random()*spawnZone.x;
    Global::player->position.y += Maths::random()*spawnZone.y;
    Global::player->position.z += Maths::random()*spawnZone.z;

    std::string playerSafeZoneLine;
    getlineSafe(file, playerSafeZoneLine);
    std::vector<std::string> safeZones = split(playerSafeZoneLine, ' ');
    Global::safeZoneStart = Vector3f(toF(safeZones[0]), toF(safeZones[1]), toF(safeZones[2]));
    Global::safeZoneEnd   = Vector3f(toF(safeZones[3]), toF(safeZones[4]), toF(safeZones[5]));

    std::string skyColorLine;
    getlineSafe(file, skyColorLine);
    std::vector<std::string> skyColors = split(skyColorLine, ' ');
    Global::skyColor = Vector3f(toF(skyColors[0]), toF(skyColors[1]), toF(skyColors[2]));

    std::string sunDirectionLine;
    getlineSafe(file, sunDirectionLine);
    std::vector<std::string> sunDirection = split(sunDirectionLine, ' ');
    Global::lights[0]->direction = Vector3f(toF(sunDirection[0]), toF(sunDirection[1]), toF(sunDirection[2]));
    Global::lights[0]->direction.normalize();

    std::string camOrientationLine;
    getlineSafe(file, camOrientationLine);
    std::vector<std::string> camOrientation = split(camOrientationLine, ' ');
    Global::player->lookDir = Vector3f(toF(camOrientation[0]), toF(camOrientation[1]), toF(camOrientation[2]));
    Global::player->lookDir.normalize();

    std::string roundTimeLine;
    getlineSafe(file, roundTimeLine);
    std::vector<std::string> roundTime = split(roundTimeLine, ' ');
    Global::timeUntilRoundEnds = toF(roundTime[0]);

    //Now read through all the objects defined in the file

    std::string line;

    while (!file.eof())
    {
        getlineSafe(file, line);

        std::vector<std::string> tokens = split(line, ' ');

        if (tokens.size() > 0)
        {
            Input::pollInputs();

            processLine(tokens);
        }
    }

    file.close();

    // Spawn rock platforms
    if (Global::levelId == LVL_MAP4)
    {
        std::vector<RockPlatform*> rocks;
        for (int x = 0; x < 6; x++)
        {
            for (int z = 0; z < 6; z++)
            {
                int idx = z + 10*x;

                std::string id = std::to_string(idx);
                id = "RP" + id;

                RockPlatform* rock = new RockPlatform(id, Vector3f((x-3)*8.0f, 0, (z-3)*8.0f)); INCR_NEW("Entity");
                rocks.push_back(rock);
                Global::addEntity(rock);
            }
        }

        if (!Global::serverClient->isOpen())
        {
            // Put the player on a random rock
            int playerRock = (int)(rocks.size()*Maths::random());
            Global::player->position.x = rocks[playerRock]->position.x;
            Global::player->position.y = rocks[playerRock]->position.y;
            Global::player->position.z = rocks[playerRock]->position.z;
            Global::player->vel.scale(0);

            // Set timer for rocks that will break
            int rocksToDie = (int)rocks.size() - 5;

            for (int i = 0; i < rocksToDie; i++)
            {
                int idx = (int)(rocks.size()*Maths::random());
                rocks[idx]->timeUntilBreaks = 3.0f + 40*Maths::random();
                rocks.erase(rocks.begin() + idx);
            }
        }
    }

    // Spawn step fall platforms
    if (Global::levelId == LVL_MAP5)
    {
        int c = 0;
        for (int y = 15; y < 34; y+=8)
        {
            for (int x = -3; x <= 3; x++)
            {
                for (int z = -3; z <= 3; z++)
                {
                    std::string id = "SFP" + std::to_string(c);

                    StepFallPlatform* plat = new StepFallPlatform(id, Vector3f(x*3.0f, (float)y, z*3.0f)); INCR_NEW("Entity");
                    Global::addEntity(plat);

                    c++;
                }
            }
        }
    }

    if (waitForSomeTime)
    {
        int waitTargetMillis = 1; //how long loading screen should show at least (in milliseconds)

        std::chrono::high_resolution_clock::time_point timeEnd = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> time_span = timeEnd - timeStart;
        double durationMillis = time_span.count();

        int waitForMs = waitTargetMillis - (int)durationMillis;
        if (waitForMs > 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(waitForMs));
        }
    }

    switch (Global::levelId)
    {
        case LVL_EQ: Global::renderWithCulling = false; break;
        default:     Global::renderWithCulling =  true; break;
    }

    //printf("lvlid = %d culling = %d\n", Global::levelId, Global::renderWithCulling);

    //glfwSetTime(0);
    //extern double timeOld;
    //timeOld = 0.0;
}

void LevelLoader::processLine(std::vector<std::string>& dat)
{
    if (dat[0][0] == '#')
    {
        return;
    }

    int id = std::stoi(dat[0]);

    switch (id)
    {
        case ENTITY_BALL:
        {
            Ball* ball = new Ball(dat[1], Vector3f(toF(dat[2]), toF(dat[3]), toF(dat[4])), Vector3f(0,0,0)); INCR_NEW("Entity");
            Global::addEntity(ball);
            break;
        }

        case ENTITY_COLLISION_BLOCK:
        {
            CollisionBlock* block = new CollisionBlock(dat[1],
                Vector3f(toF(dat[2]), toF(dat[3]), toF(dat[4])), 
                toI(dat[5]), toF(dat[6]), toF(dat[7]), toF(dat[8]), 
                (bool)toI(dat[9]), 
                toF(dat[10]));
            INCR_NEW("Entity");
            Global::addEntity(block);
            break;
        }

        case ENTITY_RED_BARREL:
        {
            RedBarrel* barrel = new RedBarrel(dat[1], Vector3f(toF(dat[2]), toF(dat[3]), toF(dat[4]))); INCR_NEW("Entity");
            Global::addEntity(barrel);
            break;
        }

        case ENTITY_LADDER:
        {
            Ladder* ladder = new Ladder(dat[1],
                Vector3f(toF(dat[2]), toF(dat[3]), toF(dat[4])),
                Vector3f(toF(dat[5]), toF(dat[6]), toF(dat[7]))); INCR_NEW("Entity");
            Global::addEntity(ladder);
            break;
        }

        case ENTITY_HEALTH_CUBE:
        {
            HealthCube* health = new HealthCube(dat[1], Vector3f(toF(dat[2]), toF(dat[3]), toF(dat[4]))); INCR_NEW("Entity");
            Global::addEntity(health);
            break;
        }

        case ENTITY_GLASS:
        {
            Vector3f centerPos(Vector3f(toF(dat[3]), toF(dat[4]), toF(dat[5])));

            Vector3f topPos = centerPos; topPos.z += 2.4f;
            Vector3f botPos = centerPos; botPos.z -= 2.4f;

            Glass* glassTop = new Glass(dat[1], topPos); INCR_NEW("Entity");
            Glass* glassBot = new Glass(dat[2], botPos); INCR_NEW("Entity");

            float ran = Maths::random();
            if (ran > 0.5f)
            {
                glassTop->isReal = false;
            }
            else
            {
                glassBot->isReal = false;
            }

            Global::addEntity(glassTop);
            Global::addEntity(glassBot);
            break;
        }

        case ENTITY_BOOM_BOX:
        {
            BoomBox* box = new BoomBox(dat[1], Vector3f(toF(dat[2]), toF(dat[3]), toF(dat[4])), toF(dat[5])); INCR_NEW("Entity");
            Global::addEntity(box);
            break;
        }

        case ENTITY_ROCK_PLATFORM:
        {
            RockPlatform* rock = new RockPlatform(dat[1], Vector3f(toF(dat[2]), toF(dat[3]), toF(dat[4]))); INCR_NEW("Entity");
            Global::addEntity(rock);
            break;
        }

        case ENTITY_CHANDELIER:
        {
            Chandelier* chandelier = new Chandelier(
                dat[1], //name
                Vector3f(toF(dat[2]), toF(dat[3]), toF(dat[4])), //position
                toI(dat[5]), //type
                toF(dat[6]), //rotY
                toI(dat[7])); //lightIdx
            INCR_NEW("Entity");
            Global::addEntity(chandelier);
            break;
        }

        case ENTITY_FENCE_PLATFORM:
        {
            FencePlatform* fence = new FencePlatform(dat[1], Vector3f(toF(dat[2]), toF(dat[3]), toF(dat[4]))); INCR_NEW("Entity");
            Global::addEntity(fence);
            break;
        }

        case ENTITY_STEP_FALL_PLATFORM:
        {
            StepFallPlatform* plat = new StepFallPlatform(dat[1], Vector3f(toF(dat[2]), toF(dat[3]), toF(dat[4]))); INCR_NEW("Entity");
            Global::addEntity(plat);
            break;
        }

        default:
        {
            return;
        }
    }
}

float LevelLoader::toF(std::string input)
{
    return std::stof(input);
}

int LevelLoader::toI(std::string input)
{
    return std::stoi(input);
}
