#ifdef _WIN32
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>

#include <string>
#include <cstring>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <utility>

#include <chrono>
#include <ctime>
#include <random>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "main.hpp"
#include "../renderEngine/renderEngine.hpp"
#include "../toolbox/input.hpp"
#include "../models/models.hpp"
#include "../shaders/shaderprogram.hpp"
#include "../textures/modeltexture.hpp"
#include "../entities/entity.hpp"
#include "../toolbox/vector.hpp"
#include "../toolbox/matrix.hpp"
#include "../entities/camera.hpp"
#include "../entities/dummy.hpp"
#include "../loader/ObjLoader.hpp"
#include "../entities/light.hpp"
#include "../collision/collisionchecker.hpp"
#include "../toolbox/split.hpp"
#include "../toolbox/getline.hpp"
#include "../collision/collisionmodel.hpp"
#include "../entities/player.hpp"
#include "../entities/human.hpp"
#include "../audio/audiomaster.hpp"
#include "../audio/audioplayer.hpp"
#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#endif

std::string Global::pathToEXE;

std::unordered_set<Entity*> gameEntities;
std::list<Entity*> gameEntitiesToAdd;
std::list<Entity*> gameEntitiesToDelete;

float Global::waterHeight = 0.0f;

float dt = 0;
double timeOld = 0;
double timeNew = 0;
Camera*             Global::gameCamera       = nullptr;
Player*             Global::player           = nullptr;
Entity*             Global::gameStageManager = nullptr;
Light*              Global::gameLightSun     = nullptr;
Light*              Global::gameLightMoon    = nullptr;

float Global::finishStageTimer = -1;

Fbo* Global::gameMultisampleFbo = nullptr;
Fbo* Global::gameOutputFbo = nullptr;
Fbo* Global::gameOutputFbo2 = nullptr;

bool Global::renderParticles = true;

bool Global::framerateUnlock = false;

bool Global::useFullscreen = false;

extern unsigned int SCR_WIDTH;
extern unsigned int SCR_HEIGHT;

int Global::countNew = 0;
int Global::countDelete = 0;
int Global::gameState = 0;
int Global::levelID = 0;
float Global::startStageTimer = -1;
bool Global::shouldLoadLevel = false;
bool Global::isNewLevel = false;
bool Global::isAutoCam = true;
std::string Global::levelName = "";
std::string Global::levelNameDisplay = "";
int Global::gameRingCount = 0;
int Global::gameScore = 0;
int Global::gameLives = 4;
float Global::gameClock = 0.0f;
int Global::gameTotalPlaytime = 0;
float Global::gameArcadePlaytime = 0;
float Global::deathHeight = -100.0f;
int Global::gameMainVehicleSpeed = 0;

int Global::gameMissionNumber = 0;
std::string Global::gameMissionDescription = "";
bool Global::gameIsNormalMode = false;
bool Global::gameIsHardMode = false;
bool Global::gameIsChaoMode = false;
bool Global::gameIsRingMode = false;
bool Global::gameIsRaceMode = false;
float Global::gameRaceTimeLimit = 0.0f;
int  Global::gameRingTarget = 100;
bool Global::gameIsArcadeMode = false;
std::unordered_map<std::string, std::string> Global::gameSaveData;
bool Global::stageUsesWater = true;
Vector3f Global::stageWaterColor(0,0,0);
float Global::stageWaterBlendAmount = 0.0f;
float Global::stageWaterMurkyAmount = 0.0f;
bool Global::renderWithCulling = true;
bool Global::displayFPS = true;
//float Global::fpsTarget = 120.0f;
float Global::fpsLimit = 60.0f;
int Global::currentCalculatedFPS = 0;
int Global::renderCount = 0;
int Global::displaySizeChanged = 0;

void increaseProcessPriority();

int main(int argc, char** argv)
{
    if (argc > 0)
    {
        Global::pathToEXE = argv[0];

        #ifdef _WIN32
        int idx = (int)Global::pathToEXE.find_last_of('\\', Global::pathToEXE.size());
        Global::pathToEXE = Global::pathToEXE.substr(0, idx+1);
        #else
        int idx = (int)Global::pathToEXE.find_last_of('/', Global::pathToEXE.size());
        Global::pathToEXE = Global::pathToEXE.substr(0, idx+1);
        #endif
    }

    increaseProcessPriority();

    Global::countNew = 0;
    Global::countDelete = 0;

    srand(0);

    createDisplay();

    Global::loadSaveData();

    #if !defined(DEV_MODE) && defined(_WIN32)
    FreeConsole();
    #endif

    Input::init();

    //This camera is never deleted.
    Camera cam;
    Global::gameCamera = &cam;

    Master_init();

    AudioMaster::init();
    AudioPlayer::loadSoundEffects();


    //This light never gets deleted.
    Light lightSun;
    Global::gameLightSun = &lightSun;
    lightSun.direction.set(-0.2f, -1, -0.4f);
    lightSun.direction.normalize();

    //This light never gets deleted.
    Light lightMoon;
    Global::gameLightMoon = &lightMoon;


    //lightSun.getPosition()->x = 0;
    //lightSun.getPosition()->y = 0;
    //lightSun.getPosition()->z = 0;
    //lightMoon.getPosition()->y = -100000;


    long long secSinceEpoch = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    glfwSetTime(0);

    int frameCount = 0;
    double previousTime = 0;

    Global::gameState = STATE_RUNNING;

    //extern GLuint transparentDepthTexture;
    //GuiTexture* debugDepth = new GuiTexture(transparentDepthTexture, 0.2f, 0.8f, 0.3f, 0.3f, 0); INCR_NEW("GuiTexture");

    std::list<std::unordered_set<Entity*>*> entityChunkedList;

    std::string folder = "TestMap";

    std::list<TexturedModel*> modelsStage;
    ObjLoader::loadModel(&modelsStage, "res/Models/" + folder + "/", "TestMap");
    Dummy* entityStage = new Dummy(&modelsStage);
    Global::addEntity(entityStage);

    std::list<TexturedModel*> modelsSphere;
    ObjLoader::loadModel(&modelsSphere, "res/Models/", "Sphere");
    Global::player = new Player(&modelsSphere);
    Global::addEntity(Global::player);

    Global::addEntity(new Human(72.076897f, 0.313516f, 23.235739f));

    CollisionModel* cm = ObjLoader::loadCollisionModel("Models/" + folder + "/", "TestMap");
    for (int i = 0; i < cm->triangles.size(); i++)
    {
        CollisionChecker::addTriangle(cm->triangles[i]);
    }
    CollisionChecker::constructChunkDatastructure();


    while (Global::gameState != STATE_EXITING && displayWantsToClose() == 0)
    {
        ANALYSIS_START("Frame Time");

        timeNew = glfwGetTime();

        //spin lock to meet the target fps, and gives extremely consistent dt's.
        // also of course uses a ton of cpu.
        if (Global::gameState == STATE_RUNNING && Global::framerateUnlock && Global::fpsLimit > 0.0f)
        {
            double dtFrameNeedsToTake = 1.0/((double)Global::fpsLimit);
            while ((timeNew - timeOld) < dtFrameNeedsToTake)
            {
                timeNew = glfwGetTime();
            }
        }

        dt = (float)(timeNew - timeOld);
        dt = std::fminf(dt, 0.04f); //Anything lower than 25fps will slow the gameplay down
        //dt*=0.2f;
        timeOld = timeNew;

        Input::pollInputs();

        frameCount++;
        Global::renderCount++;

        long long nextSecSinceEpoch = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        int epocSecDiff = (int)(nextSecSinceEpoch - secSinceEpoch);
        secSinceEpoch = nextSecSinceEpoch;
        Global::gameTotalPlaytime+=epocSecDiff;

        if (Global::gameIsArcadeMode)
        {
            Global::gameArcadePlaytime+=dt;
        }

        GLenum err = glGetError();
        if (err != GL_NO_ERROR)
        {
            std::fprintf(stderr, "########  GL ERROR  ########\n");
            std::fprintf(stderr, "%d\n", err);
        }
        
        ALenum err2 = alGetError();
        if (err2 != AL_NO_ERROR)
        {
            std::fprintf(stderr, "########  AL ERROR  ########\n");
            std::fprintf(stderr, "%d\n", err2);
        }

        //long double thisTime = std::time(0);
        //std::fprintf(stdout, "time: %f time\n", thisTime);

        ANALYSIS_START("Entity Management");
        //entities managment
        for (auto entityToAdd : gameEntitiesToAdd)
        {
            gameEntities.insert(entityToAdd);
        }
        gameEntitiesToAdd.clear();

        for (auto entityToDelete : gameEntitiesToDelete)
        {
            gameEntities.erase(entityToDelete);
            delete entityToDelete; INCR_DEL("Entity");
        }
        gameEntitiesToDelete.clear();

        ANALYSIS_DONE("Entity Management");

        switch (Global::gameState)
        {
            case STATE_RUNNING:
            {
                ANALYSIS_START("Object Logic");
                //game logic

                //unlock framerate during gameplay
                if (Global::framerateUnlock)
                {
                    glfwSwapInterval(0);
                }
                else
                {
                    glfwSwapInterval(1);
                }

                if (Global::startStageTimer >= 0)
                {
                    Global::startStageTimer -= dt;
                    if (Global::startStageTimer < 0)
                    {
                        //Global::mainHudTimer->freeze(false);
                    }
                }

                for (Entity* e : gameEntities)
                {
                    e->step();
                }

                if (Global::gameStageManager != nullptr)
                {
                    Global::gameStageManager->step();
                }

                ModelTexture::updateAnimations(dt);
                Global::gameCamera->refresh();
                //ParticleMaster::update(Global::gameCamera);
                Global::gameClock+=dt;

                if (Global::gameIsRingMode)
                {
                    if (Global::gameRingCount >= Global::gameRingTarget && Global::finishStageTimer < 0)
                    {
                        Global::finishStageTimer = 0;
                        //Global::mainHudTimer->freeze(true);
                    }
                }
                ANALYSIS_DONE("Object Logic");
                break;
            }

            case STATE_PAUSED:
            {
                //vsync during pausing. no need to stress the system.
                glfwSwapInterval(1);
                break;
            }

            case STATE_CUTSCENE:
            {
                glfwSwapInterval(1);
                Global::gameCamera->refresh();
                break;
            }

            case STATE_TITLE:
            {
                //vsync during title. no need to stress the system.
                glfwSwapInterval(1);
                Global::gameCamera->refresh();
                //ParticleMaster::update(Global::gameCamera);
                break;
            }

            case STATE_DEBUG:
            {
                glfwSwapInterval(1);

                //if (Global::gameMainPlayer != nullptr)
                {
                    //Global::gamePlayer->debugAdjustCamera();
                }

                Input::pollInputs();
                break;
            }

            default:
                break;
        }

        Vector3f camVel = cam.vel.scaleCopy(0.016666f);
        AudioMaster::updateListenerData(&cam.eye, &cam.target, &cam.up, &camVel);
        AudioPlayer::setListenerIsUnderwater(false);

        //Global::menuManager.step();

        //Stage::updateVisibleChunks();
        //SkyManager::calculateValues();

        //prepare entities to render
        for (Entity* e : gameEntities)
        {
            Master_processEntity(e);
        }

        if (Global::gameStageManager != nullptr)
        {
            Master_processEntity(Global::gameStageManager);
        }

        float waterBlendAmount = 0.0f;
        if (cam.inWater || (cam.eye.y < Global::waterHeight && Global::stageUsesWater))
        {
            waterBlendAmount = Global::stageWaterBlendAmount;
        }

        //glEnable(GL_CLIP_DISTANCE1);
        Master_render(&cam, 0, 0, 0, 0, waterBlendAmount);
        //glDisable(GL_CLIP_DISTANCE1);


        Master_clearAllEntities();

        updateDisplay();

        Global::clearTitleCard();

        if (Global::shouldLoadLevel)
        {
            Global::shouldLoadLevel = false;
            //LevelLoader::loadLevel(Global::levelName);
        }

        if (previousTime > timeNew)
        {
            previousTime = timeNew;
        }

        if (timeNew - previousTime >= 1.0)
        {
            Global::currentCalculatedFPS = (int)(std::round(frameCount/(timeNew - previousTime)));
            //std::fprintf(stdout, "fps: %f\n", frameCount / (timeNew - previousTime));
            //printf("%f\t%f\n", (float)glfwGetTime(), 36.7816091954f*ball->vel.length());
            //std::fprintf(stdout, "diff: %d\n", Global::countNew - Global::countDelete);
            //Loader::printInfo();
            //std::fprintf(stdout, "entity counts: %d %d %d\n", gameEntities.size(), gameEntitiesPass2.size(), gameTransparentEntities.size());
            frameCount = 0;
            previousTime = timeNew;
        }

        Global::displaySizeChanged = std::max(0, Global::displaySizeChanged - 1);
        if (Global::displaySizeChanged == 1)
        {
            //recreate all fbos and other things to the new size of the window
            //if (Global::renderBloom)
            {
                //Global::gameMultisampleFbo->resize(SCR_WIDTH, SCR_HEIGHT); //memory leaks
                //Global::gameOutputFbo->resize(SCR_WIDTH, SCR_HEIGHT);
                //Global::gameOutputFbo2->resize(SCR_WIDTH, SCR_HEIGHT);
            }
        }
        //std::fprintf(stdout, "dt: %f\n", dt);

        ANALYSIS_DONE("Frame Time");
        ANALYSIS_REPORT();
    }

    Global::saveSaveData();

    Master_cleanUp();
    Loader::cleanUp();
    closeDisplay();

    return 0;
}

void Global::addEntity(Entity* entityToAdd)
{
    gameEntitiesToAdd.push_back(entityToAdd);
}

void Global::deleteEntity(Entity* entityToDelete)
{
    gameEntitiesToDelete.push_back(entityToDelete);
}

void Global::deleteAllEntites()
{
    //Make sure no entities get left behind in transition
    for (Entity* entityToAdd : gameEntitiesToAdd)
    {
        gameEntities.insert(entityToAdd);
    }
    gameEntitiesToAdd.clear();

    for (Entity* entityToDelete : gameEntitiesToDelete)
    {
        gameEntities.erase(entityToDelete);
        delete entityToDelete; INCR_DEL("Entity");
    }
    gameEntitiesToDelete.clear();


    //Delete all the rest
    for (Entity* entityToDelete : gameEntities)
    {
        delete entityToDelete; INCR_DEL("Entity");
    }
    gameEntities.clear();

    if (Global::gameStageManager != nullptr)
    {
        delete Global::gameStageManager; INCR_DEL("Entity");
        Global::gameStageManager = nullptr;
    }
}

void increaseProcessPriority()
{
    #ifdef _WIN32
    DWORD dwError;

    
    if (!SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS))
    {
        dwError = GetLastError();
        std::fprintf(stdout, "Failed to enter above normal mode (%d)\n", (int)dwError);
    }

    if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL))
    {
        dwError = GetLastError();
        std::fprintf(stdout, "Failed to enter above normal mode (%d)\n", (int)dwError);
    }
    

    
    //if (!SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS))
    //{
    //    dwError = GetLastError();
    //    _tprintf(TEXT("Failed to enter below normal mode (%d)\n"), (int)dwError);
    //}
    //
    //if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE))
    //{
    //    dwError = GetLastError();
    //    _tprintf(TEXT("Failed to enter below normal mode (%d)\n"), (int)dwError);
    //}
    

    #endif
}

void Global::loadSaveData()
{
    Global::gameSaveData.clear();

    std::ifstream file(Global::pathToEXE+"res/SaveData/SaveData.sav");
    if (!file.is_open())
    {
        std::fprintf(stdout, "No save data found. Creating new save file...\n");
        file.close();
        Global::saveSaveData();
    }
    else
    {
        std::string line;
        getlineSafe(file, line);

        while (!file.eof())
        {
            char lineBuf[512];
            memcpy(lineBuf, line.c_str(), line.size()+1);

            int splitLength = 0;
            char** lineSplit = split(lineBuf, ';', &splitLength);

            if (splitLength == 2)
            {
                Global::gameSaveData[lineSplit[0]] = lineSplit[1];
            }

            free(lineSplit);

            getlineSafe(file, line);
        }

        file.close();
    }

    if (Global::gameSaveData.find("PLAYTIME") != Global::gameSaveData.end())
    {
        Global::gameTotalPlaytime = std::stoi(Global::gameSaveData["PLAYTIME"]);
    }

    if (Global::gameSaveData.find("CAMERA") != Global::gameSaveData.end())
    {
        if (Global::gameSaveData["CAMERA"] == "AUTO")
        {
            Global::isAutoCam = true;
        }
        else
        {
            Global::isAutoCam = false;
        }
    }
}

void Global::saveSaveData()
{
    #ifdef _WIN32
    _mkdir((Global::pathToEXE + "res/SaveData").c_str());
    #else
    mkdir((Global::pathToEXE + "res/SaveData").c_str(), 0777);
    #endif

    std::ofstream file;
    file.open((Global::pathToEXE + "res/SaveData/SaveData.sav").c_str(), std::ios::out | std::ios::trunc);

    if (!file.is_open())
    {
        std::fprintf(stderr, "Error: Failed to create/access '%s'\n", (Global::pathToEXE + "res/SaveData/SaveData.sav").c_str());
        file.close();
    }
    else
    {
        Global::gameSaveData["PLAYTIME"] = std::to_string(Global::gameTotalPlaytime);

        if (Global::isAutoCam)
        {
            Global::gameSaveData["CAMERA"] = "AUTO";
        }
        else
        {
            Global::gameSaveData["CAMERA"] = "FREE";
        }

        std::unordered_map<std::string, std::string>::iterator it = Global::gameSaveData.begin();
 
        while (it != Global::gameSaveData.end())
        {
            file << it->first+";"+it->second+"\n";
            
            it++;
        }

        file.close();
    }
}

void Global::saveConfigData()
{
   
}

float Global::calcAspectRatio()
{
    extern unsigned int SCR_WIDTH;
    extern unsigned int SCR_HEIGHT;
    return ((float)SCR_WIDTH)/SCR_HEIGHT;
}

void Global::createTitleCard()
{
    
}

void Global::clearTitleCard()
{
    
}

std::unordered_map<std::string, int> heapObjects;

void Global::debugNew(const char* name)
{
    Global::countNew++;

    #ifdef DEV_MODE
    if (heapObjects.find(name) == heapObjects.end())
    {
        heapObjects[name] = 1;
    }
    else
    {
        int num = heapObjects[name];
        heapObjects[name] = num+1;
    }
    #else
    name;
    #endif
}

void Global::debugDel(const char* name)
{
    Global::countDelete++;

    #ifdef DEV_MODE
    if (heapObjects.find(name) == heapObjects.end())
    {
        std::fprintf(stdout, "Warning: trying to delete '%s' when there are none.\n", name);
        heapObjects[name] = 0;
    }
    else
    {
        int num = heapObjects[name];
        heapObjects[name] = num-1;
    }
    #else
    name;
    #endif
}

std::unordered_map<std::string, float> operationTotalTimes;
std::unordered_map<std::string, float> operationStartTimes;

void Global::performanceAnalysisStart(const char* name)
{
    auto startTime = operationStartTimes.find(name);
    if (startTime == operationStartTimes.end())
    {
        operationStartTimes[name] = (float)glfwGetTime();
    }
    else
    {
        printf("Starting operation %s but that operation is already happening...\n", name);
    }
}

void Global::performanceAnalysisDone(const char* name)
{
    auto startTime = operationStartTimes.find(name);
    if (startTime != operationStartTimes.end())
    {
        float timeTaken = ((float)glfwGetTime()) - startTime->second;
        operationStartTimes.erase(startTime);

        auto totalTime = operationTotalTimes.find(name);
        if (totalTime == operationTotalTimes.end())
        {
            operationTotalTimes[name] = timeTaken;
        }
        else
        {
            operationTotalTimes[name] = totalTime->second + timeTaken;
        }
    }
    else
    {
        printf("Operation %s hasn't been started yet. Or a recursive operation, which is a no no.\n", name);
    }
}

void Global::performanceAnalysisReport()
{
    printf("Performance Report\n");

    auto it = operationTotalTimes.begin();
    while (it != operationTotalTimes.end())
    {
        printf("%s    %f\n", it->first.c_str(), 1000*it->second);
        it++;
    }

    printf("\n");

    if (!operationStartTimes.empty())
    {
        printf("operationTotalTimes was not empty. Some operations were not finished when doing the report.\n");
    }
    operationStartTimes.clear();
    operationTotalTimes.clear();
}
