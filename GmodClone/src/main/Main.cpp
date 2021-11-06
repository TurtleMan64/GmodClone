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
#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#endif

std::string Global::pathToEXE;

std::unordered_set<Entity*> gameEntities;
std::list<Entity*> gameEntitiesToAdd;
std::list<Entity*> gameEntitiesToDelete;

//vector that we treat as a 2D array. 
std::vector<std::unordered_set<Entity*>> gameChunkedEntities;
std::list<Entity*> gameChunkedEntitiesToAdd;
std::list<Entity*> gameChunkedEntitiesToDelete;
float chunkedEntitiesMinX = 0;
float chunkedEntitiesMinZ = 1;
float chunkedEntitiesChunkSize = 1;
int chunkedEntitiesWidth = 1;
int chunkedEntitiesHeight = 1;


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

bool Global::debugDisplay = false;
bool Global::frozen = false;
bool Global::step = false;

bool Global::useHighQualityWater = true;
unsigned Global::HQWaterReflectionWidth = 1280;
unsigned Global::HQWaterReflectionHeight = 720;
unsigned Global::HQWaterRefractionWidth = 1280;
unsigned Global::HQWaterRefractionHeight = 720;

bool Global::renderParticles = true;

bool Global::renderBloom = false;

bool Global::framerateUnlock = false;

bool Global::useFullscreen = false;

bool Global::renderShadowsFar = false;
bool Global::renderShadowsClose = false;
int Global::shadowsFarQuality = 0;


//extern bool INPUT_JUMP;
//extern bool INPUT_ACTION;
//extern bool INPUT_ACTION2;
//
//extern bool INPUT_PREVIOUS_JUMP;
//extern bool INPUT_PREVIOUS_ACTION;
//extern bool INPUT_PREVIOUS_ACTION2;

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

Global::PlayableCharacter Global::currentCharacterType = Global::PlayableCharacter::Sonic;
std::unordered_map<Global::PlayableCharacter, std::string> Global::characterNames;

int Global::gameArcadeIndex = 0;
std::vector<std::pair<int, Global::PlayableCharacter>> Global::gameArcadeLevelIds; //level ids and the character id

std::vector<int> Global::gameLevelIdsSonic;
std::vector<int> Global::gameLevelIdsTails;
std::vector<int> Global::gameLevelIdsKnuckles;

//std::list<std::string> Global::raceLog;
bool Global::shouldLogRace = false;

int Global::raceLogSize;

bool Global::spawnAtCheckpoint = false;
Vector3f Global::checkpointPlayerPos;
Vector3f Global::checkpointPlayerDir;
Vector3f Global::checkpointCamDir;
float Global::checkpointTime = 0;

bool Global::unlockedSonicDoll = true;
bool Global::unlockedMechaSonic = true;
bool Global::unlockedDage4 = true;
bool Global::unlockedManiaSonic = true;
bool Global::unlockedAmy = true;

std::unordered_map<int, int> Global::stageNpcCounts;

void increaseProcessPriority();

void doListenThread();

void listen();

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

    #ifdef DEV_MODE
    std::thread listenThread(doListenThread);
    #endif

    increaseProcessPriority();

    Global::countNew = 0;
    Global::countDelete = 0;

    srand(0);

    createDisplay();

    Global::characterNames[Global::PlayableCharacter::Sonic] = "Sonic";
    Global::characterNames[Global::PlayableCharacter::Tails] = "Tails";
    Global::characterNames[Global::PlayableCharacter::Knuckles] = "Knuckles";

    Global::loadSaveData();

    //The levels you play in arcade mode, in order
    Global::gameArcadeLevelIds.push_back(std::make_pair(LVL_TUTORIAL,        Global::PlayableCharacter::Sonic));
    Global::gameArcadeLevelIds.push_back(std::make_pair(LVL_METAL_HARBOR,    Global::PlayableCharacter::Knuckles));
    Global::gameArcadeLevelIds.push_back(std::make_pair(LVL_RADICAL_HIGHWAY, Global::PlayableCharacter::Tails));
    Global::gameArcadeLevelIds.push_back(std::make_pair(LVL_GREEN_FOREST,    Global::PlayableCharacter::Sonic));
    Global::gameArcadeLevelIds.push_back(std::make_pair(LVL_SKY_RAIL,        Global::PlayableCharacter::Sonic));

    Global::gameLevelIdsSonic.push_back(LVL_TUTORIAL);
    Global::gameLevelIdsSonic.push_back(LVL_DRAGON_ROAD);
    Global::gameLevelIdsSonic.push_back(LVL_GREEN_FOREST);
    Global::gameLevelIdsSonic.push_back(LVL_METAL_HARBOR);
    Global::gameLevelIdsSonic.push_back(LVL_SKY_RAIL);
    Global::gameLevelIdsSonic.push_back(LVL_PYRAMID_CAVE);
    Global::gameLevelIdsSonic.push_back(LVL_RADICAL_HIGHWAY);
    Global::gameLevelIdsSonic.push_back(LVL_GREEN_HILL_ZONE);
    Global::gameLevelIdsSonic.push_back(LVL_CITY_ESCAPE);
    Global::gameLevelIdsSonic.push_back(LVL_WINDY_VALLEY);
    Global::gameLevelIdsSonic.push_back(LVL_SEASIDE_HILL);
    Global::gameLevelIdsSonic.push_back(LVL_FROG_FOREST);
    Global::gameLevelIdsSonic.push_back(LVL_TEST);
    Global::gameLevelIdsSonic.push_back(LVL_SPEED_HIGHWAY);
    Global::gameLevelIdsSonic.push_back(LVL_SACRED_SKY);
    Global::gameLevelIdsSonic.push_back(LVL_TWINKLE_CIRCUIT);

    Global::gameLevelIdsTails.push_back(LVL_TUTORIAL);
    Global::gameLevelIdsTails.push_back(LVL_GREEN_FOREST);
    Global::gameLevelIdsTails.push_back(LVL_METAL_HARBOR);
    Global::gameLevelIdsTails.push_back(LVL_PYRAMID_CAVE);
    Global::gameLevelIdsTails.push_back(LVL_RADICAL_HIGHWAY);
    Global::gameLevelIdsTails.push_back(LVL_GREEN_HILL_ZONE);
    Global::gameLevelIdsTails.push_back(LVL_CITY_ESCAPE);
    Global::gameLevelIdsTails.push_back(LVL_WINDY_VALLEY);
    Global::gameLevelIdsTails.push_back(LVL_SEASIDE_HILL);
    Global::gameLevelIdsTails.push_back(LVL_FROG_FOREST);
    Global::gameLevelIdsTails.push_back(LVL_TEST);
    Global::gameLevelIdsTails.push_back(LVL_SPEED_HIGHWAY);
    Global::gameLevelIdsTails.push_back(LVL_SACRED_SKY);
    Global::gameLevelIdsTails.push_back(LVL_CLOUD_TEMPLE);

    Global::gameLevelIdsKnuckles.push_back(LVL_DRY_LAGOON);
    Global::gameLevelIdsKnuckles.push_back(LVL_DELFINO_PLAZA);
    Global::gameLevelIdsKnuckles.push_back(LVL_NOKI_BAY);

    //create NPC list
    Global::stageNpcCounts[LVL_SKY_RAIL] = 1;
    Global::stageNpcCounts[LVL_DRY_LAGOON] = 1;
    Global::stageNpcCounts[LVL_METAL_HARBOR] = 4;
    Global::stageNpcCounts[LVL_RADICAL_HIGHWAY] = 5;
    Global::stageNpcCounts[LVL_DRAGON_ROAD] = 7;

    #if !defined(DEV_MODE) && defined(_WIN32)
    FreeConsole();
    #endif

    Input::init();

    //This camera is never deleted.
    Camera cam;
    Global::gameCamera = &cam;

    Master_init();




    //This light never gets deleted.
    Light lightSun;
    Global::gameLightSun = &lightSun;

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

    std::string folder = "Waterworld";

    std::list<TexturedModel*> modelsStage;
    ObjLoader::loadModel(&modelsStage, "res/Models/" + folder + "/", "Waterworld");
    Dummy* entityStage = new Dummy(&modelsStage);
    Global::addEntity(entityStage);

    std::list<TexturedModel*> modelsSphere;
    ObjLoader::loadModel(&modelsSphere, "res/Models/", "Sphere");
    Global::player = new Player(&modelsSphere);
    Global::addEntity(Global::player);

    Global::addEntity(new Human(0, 0, 0));

    CollisionModel* cm = ObjLoader::loadCollisionModel("Models/" + folder + "/", "Waterworld");
    for (int i = 0; i < cm->triangles.size(); i++)
    {
        CollisionChecker::addTriangle(cm->triangles[i]);
    }
    CollisionChecker::calculateDatastructure();


    while (Global::gameState != STATE_EXITING && displayWantsToClose() == 0)
    {
        ANALYSIS_START("Frame Time");

        timeNew = glfwGetTime();

        #ifndef WIN32LOL
        //spin lock to meet the target fps, and gives extremely consistent dt's.
        // also of course uses a ton of cpu.
        if (Global::gameState == STATE_RUNNING && Global::framerateUnlock)
        {
            double dtFrameNeedsToTake = 1.0/((double)Global::fpsLimit);
            while ((timeNew - timeOld) < dtFrameNeedsToTake)
            {
                timeNew = glfwGetTime();
            }
        }
        #else
        //another idea: windows only. if you put the thread/process into a above normal priority,
        // and call Sleep, it will actually sleep and return pretty consistently close
        // to the amount you slept for. in my testing, it would never sleep for more
        // than 2 milliseconds longer than what it was given (max was 1.5323 ms more than
        // the sleep amount). So, we can sleep for 2ms less than the time needed, and then
        // busy wait loop for the remaining time.
        // This actually works really well but also has the same problem as the busy wait loop,
        // which is the video looks choppy at bad fps targets. For example, if you set the target to 
        // 60fps on a 60fps monitor, then it looks fine. But, if you set the target to 90fps, then
        // it looks very choppy.
        // EDIT: Sleep no longer does this and always does like intervals of 16ms... so this is pointless now...
        if (Global::gameState == STATE_RUNNING && Global::framerateUnlock)
        {
            if (Global::fpsLimit > 0.0f)
            {
                double dtFrameNeedsToTake = 1.0/((double)Global::fpsLimit);
                timeNew = glfwGetTime();
        
                const double sleepBuffer = 0.00175; //sleep will hopefully never take longer than this to return
                double sleepTime = (dtFrameNeedsToTake - (timeNew - timeOld)) - sleepBuffer;
                int msToSleep = (int)(sleepTime*1000);
                if (msToSleep >= 1)
                {
                    Sleep(msToSleep);
                }
        
                timeNew = glfwGetTime();
                while ((timeNew - timeOld) < dtFrameNeedsToTake)
                {
                    timeNew = glfwGetTime();
                }
            }
        }
        #endif

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

        //chunked entities mamanegement
        for (auto entityToAdd : gameChunkedEntitiesToAdd)
        {
            int realIndex = Global::getChunkIndex(entityToAdd->getX(), entityToAdd->getZ());
            gameChunkedEntities[realIndex].insert(entityToAdd);
        }
        gameChunkedEntitiesToAdd.clear();

        for (auto entityToDelete : gameChunkedEntitiesToDelete)
        {
            int realIndex = Global::getChunkIndex(entityToDelete->getX(), entityToDelete->getZ());
            size_t numDeleted = gameChunkedEntities[realIndex].erase(entityToDelete);
            if (numDeleted == 0)
            {
                for (int i = 0; i < (int)gameChunkedEntities.size(); i++)
                {
                    numDeleted = gameChunkedEntities[i].erase(entityToDelete);
                    if (numDeleted > 0)
                    {
                        break;
                    }
                }

                if (numDeleted == 0)
                {
                    std::fprintf(stdout, "Error: Tried to delete a chunked entity that wasn't in the lists.\n");
                }
                else
                {
                    delete entityToDelete; INCR_DEL("Entity");
                }
            }
            else
            {
                delete entityToDelete; INCR_DEL("Entity");
            }
        }
        gameChunkedEntitiesToDelete.clear();
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

                //printf("\nbefore player %f\n", Global::gameCamera->eye.x);
                //if (Global::gameMainPlayer != nullptr)
                {
                    //Global::gameMainPlayer->step();
                }
                //printf("after player %f\n", Global::gameCamera->eye.x);
                for (Entity* e : gameEntities)
                {
                    ///printf("    new entity %f  %s\n", Global::gameCamera->eye.x, typeid(*e).name());
                    e->step();
                }
                //printf("after kart %f\n", Global::gameCamera->eye.x);
                if (gameChunkedEntities.size() > 0)
                {
                    Global::getNearbyEntities(cam.eye.x, cam.eye.z, 2, &entityChunkedList);
                    for (std::unordered_set<Entity*>* entitySet : entityChunkedList)
                    {
                        for (Entity* e : (*entitySet))
                        {
                            e->step();
                        }
                    }
                }
                if (Global::gameStageManager != nullptr)
                {
                    Global::gameStageManager->step();
                }

                //skySphere.step();
                ModelTexture::updateAnimations(dt);
                Global::gameCamera->refresh();
                //ParticleMaster::update(Global::gameCamera);
                Global::gameClock+=dt;

                if (Global::debugDisplay && Global::frozen)
                {
                    Global::gameState = STATE_DEBUG;
                }

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
                if (Global::step)
                {
                    Global::gameState = STATE_RUNNING;
                    Global::step = false;
                }
                
                if (Global::debugDisplay == false || Global::frozen == false)
                {
                    Global::gameState = STATE_RUNNING;
                }
                break;
            }

            default:
                break;
        }

        //Global::menuManager.step();

        //Stage::updateVisibleChunks();
        //SkyManager::calculateValues();

        //prepare entities to render
        for (Entity* e : gameEntities)
        {
            Master_processEntity(e);
        }
        if (gameChunkedEntities.size() > 0)
        {
            for (std::unordered_set<Entity*>* entitySet : entityChunkedList)
            {
                for (Entity* e : (*entitySet))
                {
                    Master_processEntity(e);
                }
            }
        }

        if (Global::gameStageManager != nullptr)
        {
            Master_processEntity(Global::gameStageManager);
        }
        //Master_processEntity(&stage);
        //Master_renderShadowMaps(&lightSun);
        //Master_processEntity(&skySphere);

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

    #ifdef DEV_MODE
    listenThread.detach();
    #endif

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

void Global::checkErrorAL(const char* /*description*/)
{

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

void Global::saveGhostData()
{
   
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

void Global::increaseRingCount(int /*rings*/)
{

}

int Global::calculateRankAndUpdate()
{
  
}

void doListenThread()
{
#ifdef _WIN32
    DWORD dwError;

    if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL))
    {
        dwError = GetLastError();
        _tprintf(TEXT("Failed to enter above normal mode (%d)\n"), (int)dwError);
    }
#endif

    listen();
}

//listen on stdin for coordinates
void listen()
{
    int loop = 1;
    std::string input;

    while (loop == 1)
    {
        getlineSafe(std::cin, input);

        if (input == "exit")
        {
            loop = 0;
        }
        else if (input.size() > 1)
        {
            fprintf(stdout, "input = '%s'\n", input.c_str());
            //Global::gamePlayer->setGroundSpeed(0, 0);
            //Global::gamePlayer->setxVelAir(0);
            //Global::gamePlayer->setxVelAir(0);
            //Global::gamePlayer->setyVel(0);
            if (input.c_str()[0] == 'w')
            {
                const char* data = &input.c_str()[1];
                Global::waterHeight = std::stof(data);
            }
            //if (input == "goff")
            //{
            //    Global::gamePlayer->setGravity(0);
            //}
            //else if (input == "gon")
            //{
            //    Global::gamePlayer->setGravity(0.08f);
            //}
            //else if (input.c_str()[0] == 'x')
            //{
            //    const char* data = &input.c_str()[1];
            //    Global::gamePlayer->setX(std::stof(data));
            //    Global::gamePlayer->setGravity(0);
            //}
            //else if (input.c_str()[0] == 'y')
            //{
            //    const char* data = &input.c_str()[1];
            //    Global::gamePlayer->setY(std::stof(data));
            //    Global::gamePlayer->setGravity(0);
            //}
            //else if (input.c_str()[0] == 'z')
            //{
            //    const char* data = &input.c_str()[1];
            //    Global::gamePlayer->setZ(std::stof(data));
            //    Global::gamePlayer->setGravity(0);
            //}
            //else
            //{
            //    char lineBuf[256];
            //    memcpy(lineBuf, input.c_str(), input.size()+1);
            //
            //    int splitLength = 0;
            //    char** lineSplit = split(lineBuf, ' ', &splitLength);
            //
            //    if (splitLength == 3)
            //    {
            //        Global::gamePlayer->setX(std::stof(lineSplit[0]));
            //        Global::gamePlayer->setY(std::stof(lineSplit[1]));
            //        Global::gamePlayer->setZ(std::stof(lineSplit[2]));
            //        Global::gamePlayer->setGravity(0);
            //    }
            //    free(lineSplit);
            //}
        }
    }
}

//Returns the index of 'gameChunkedEntities' for the (x, z) location
int Global::getChunkIndex(float x, float z)
{
    float relativeX = x - chunkedEntitiesMinX;
    float relativeZ = z - chunkedEntitiesMinZ;

    //const float DIVIDE_OFFSET = 1.0f;

    //ensure the coords arent out of bounds
    //relativeX = std::fmaxf(0.0f, relativeX);
    //relativeX = std::fminf(relativeX, chunkedEntitiesWidth*chunkedEntitiesChunkSize - DIVIDE_OFFSET);

    //relativeZ = std::fmaxf(0.0f, relativeZ);
    //relativeZ = std::fminf(relativeZ, chunkedEntitiesHeight*chunkedEntitiesChunkSize - DIVIDE_OFFSET);

    //calculate 2d array indices
    int iX = (int)(relativeX/chunkedEntitiesChunkSize);
    int iZ = (int)(relativeZ/chunkedEntitiesChunkSize);

    iX = std::max(0, iX);
    iX = std::min(iX, chunkedEntitiesWidth-1);

    iZ = std::max(0, iZ);
    iZ = std::min(iZ, chunkedEntitiesHeight-1);

    //calculate index in gameNearbyEntities that corresponds to iX, iZ
    int realIndex = iX + iZ*chunkedEntitiesWidth;

    if (realIndex >= (int)gameChunkedEntities.size())
    {
        std::fprintf(stderr, "Error: Index out of bounds on gameNearbyEntities. THIS IS VERY BAD.\n");
        std::fprintf(stdout, "    x = %f       z = %f\n", x, z);
        std::fprintf(stdout, "    relativeX = %f      relativeZ = %f\n", relativeX, relativeZ);
        std::fprintf(stdout, "    iX = %d        iZ = %d\n", iX, iZ);
        std::fprintf(stdout, "    chunkedEntitiesWidth = %d             chunkedEntitiesHeight = %d\n", chunkedEntitiesWidth, chunkedEntitiesHeight);
        std::fprintf(stdout, "    chunkedEntitiesChunkSize = %f\n", chunkedEntitiesChunkSize);
        std::fprintf(stdout, "    realIndex = %d          gameChunkedEntities.size() = %d\n", realIndex, (int)gameChunkedEntities.size());
        return 0;
    }

    return realIndex;
}

void Global::getNearbyEntities(float x, float z, int renderDistance, std::list<std::unordered_set<Entity*>*>* list)
{
    list->clear();

    switch (renderDistance)
    {
        case 0:
        {
            list->push_back(&gameChunkedEntities[Global::getChunkIndex(x, z)]);
            break;
        }

        case 1:
        {
            std::unordered_set<int> chunkIdxs;
            float w = chunkedEntitiesChunkSize/2;
            chunkIdxs.insert(Global::getChunkIndex(x-w, z-w));
            chunkIdxs.insert(Global::getChunkIndex(x+w, z-w));
            chunkIdxs.insert(Global::getChunkIndex(x-w, z+w));
            chunkIdxs.insert(Global::getChunkIndex(x+w, z+w));
            for (int i : chunkIdxs)
            {
                list->push_back(&gameChunkedEntities[i]);
            }
            break;
        }

        case 2:
        {
            std::unordered_set<int> chunkIdxs;
            float w = chunkedEntitiesChunkSize;
            chunkIdxs.insert(Global::getChunkIndex(x-w, z-w));
            chunkIdxs.insert(Global::getChunkIndex(x+0, z-w));
            chunkIdxs.insert(Global::getChunkIndex(x+w, z-w));
            chunkIdxs.insert(Global::getChunkIndex(x-w, z+0));
            chunkIdxs.insert(Global::getChunkIndex(x+0, z+0));
            chunkIdxs.insert(Global::getChunkIndex(x+w, z+0));
            chunkIdxs.insert(Global::getChunkIndex(x-w, z+w));
            chunkIdxs.insert(Global::getChunkIndex(x+0, z+w));
            chunkIdxs.insert(Global::getChunkIndex(x+w, z+w));
            for (int i : chunkIdxs)
            {
                list->push_back(&gameChunkedEntities[i]);
            }
            break;
        }

        default:
        {
            std::fprintf(stderr, "Error: Render distance not out of range.\n");
            break;
        }
    }
}

//Return a list of nearby entity sets. Returns either 1 chunk, 2 chunks, or 4 chunks,
// depending on the minDistance value.
void Global::getNearbyEntities(float x, float z, std::list<std::unordered_set<Entity*>*>* list, float minDistance)
{
    list->clear();

    std::unordered_set<int> chunkIdxs;
    float w = minDistance;
    chunkIdxs.insert(Global::getChunkIndex(x-w, z-w));
    chunkIdxs.insert(Global::getChunkIndex(x+w, z-w));
    chunkIdxs.insert(Global::getChunkIndex(x-w, z+w));
    chunkIdxs.insert(Global::getChunkIndex(x+w, z+w));
    for (int i : chunkIdxs)
    {
        list->push_back(&gameChunkedEntities[i]);
    }
}

void Global::recalculateEntityChunks(float minX, float maxX, float minZ, float maxZ, float chunkSize)
{
    if (gameChunkedEntities.size() != 0)
    {
        std::fprintf(stderr, "Error: Trying to recalculate entity chunks when gameChunkedEntities is not 0.\n");
        return;
    }

    float xDiff = maxX - minX;
    float zDiff = maxZ - minZ;

    int newWidth  = (int)(xDiff/chunkSize) + 1;
    int newHeight = (int)(zDiff/chunkSize) + 1;

    int count = newWidth*newHeight;
    for (int i = 0; i < count; i++)
    {
        std::unordered_set<Entity*> set;
        gameChunkedEntities.push_back(set);
    }

    chunkedEntitiesMinX = minX;
    chunkedEntitiesMinZ = minZ;
    chunkedEntitiesChunkSize = chunkSize;
    chunkedEntitiesWidth = newWidth;
    chunkedEntitiesHeight = newHeight;
}

void Global::addChunkedEntity(Entity* entityToAdd)
{
    gameChunkedEntitiesToAdd.push_back(entityToAdd);
}

void Global::deleteChunkedEntity(Entity* entityToAdd)
{
    gameChunkedEntitiesToDelete.push_back(entityToAdd);
}

void Global::deleteAllChunkedEntities()
{
    //Make sure no entities get left behind in transition
    for (Entity* entityToAdd : gameChunkedEntitiesToAdd)
    {
        delete entityToAdd; INCR_DEL("Entity");
    }
    gameChunkedEntitiesToAdd.clear();

    gameChunkedEntitiesToDelete.clear();

    for (std::unordered_set<Entity*> set : gameChunkedEntities)
    {
        for (Entity* e : set)
        {
            delete e; INCR_DEL("Entity");
        }
        set.clear();
    }
    gameChunkedEntities.clear();
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
