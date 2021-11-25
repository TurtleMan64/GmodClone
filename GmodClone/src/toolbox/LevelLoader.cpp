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

void LevelLoader::loadLevel(std::string mapName)
{
    std::string fname = mapName + ".map";

    std::ifstream file(Global::pathToEXE + "res/Maps/" + fname);
    if (!file.is_open())
    {
        printf("Error: Cannot load file '%s'\n", (Global::pathToEXE + "res/Maps/" + fname).c_str());
        file.close();
        std::string msg = "Can't load map '" + fname + "'";
        Global::addChatMessage(msg, Vector3f(1, 0.5f, 0.5f));
        return;
    }

    Global::deleteAllEntites();

    std::chrono::high_resolution_clock::time_point timeStart = std::chrono::high_resolution_clock::now();
    bool waitForSomeTime = true;

    if      (fname == "Hub.map")  Global::levelId = LVL_HUB;
    else if (fname == "Map1.map") Global::levelId = LVL_MAP1;
    else if (fname == "Map2.map") Global::levelId = LVL_MAP2;

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
    for (int i = 0; i < cm->triangles.size(); i++)
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

    std::string skyColorLine;
    getlineSafe(file, skyColorLine);
    std::vector<std::string> skyColors = split(skyColorLine, ' ');
    Global::skyColor = Vector3f(toF(skyColors[0]), toF(skyColors[1]), toF(skyColors[2]));

    std::string sunDirectionLine;
    getlineSafe(file, sunDirectionLine);
    std::vector<std::string> sunDirection = split(sunDirectionLine, ' ');
    Global::gameLightSun->direction = Vector3f(toF(sunDirection[0]), toF(sunDirection[1]), toF(sunDirection[2]));
    Global::gameLightSun->direction.normalize();

    std::string camOrientationLine;
    getlineSafe(file, camOrientationLine);
    std::vector<std::string> camOrientation = split(camOrientationLine, ' ');
    Global::player->lookDir = Vector3f(toF(camOrientation[0]), toF(camOrientation[1]), toF(camOrientation[2]));
    Global::player->lookDir.normalize();

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
            Glass* glass = new Glass(dat[1], Vector3f(toF(dat[2]), toF(dat[3]), toF(dat[4]))); INCR_NEW("Entity");
            Global::addEntity(glass);
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
