#ifdef _WIN32
#define _CRT_SECURE_NO_DEPRECATE
#define NOMINMAX
#endif

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
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
#include <thread>
#include <mutex>
#include <condition_variable>

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
#include "../entities/onlineplayer.hpp"
#include "../audio/audiomaster.hpp"
#include "../audio/audioplayer.hpp"
#include "../entities/ball.hpp"
#include "../entities/collisionblock.hpp"
#include "../entities/redbarrel.hpp"
#include "../network/tcpclient.hpp"
#include "../fontRendering/textmaster.hpp"
#include "../fontMeshCreator/fonttype.hpp"
#include "../fontMeshCreator/guitext.hpp"
#include "../guis/guimanager.hpp"
#include "../guis/guitextureresources.hpp"
#include "../guis/guitexture.hpp"
#include "../toolbox/maths.hpp"
#include "../entities/ladder.hpp"


Message::Message(const Message &other)
{
    length = other.length;
    for (int i = 0; i < 188; i++)
    {
        buf[i] = other.buf[i];
    }
}

Message::Message()
{
    
}

std::string Global::pathToEXE;

std::string Global::nickname;

double Global::syncedGlobalTime = 0.0;

std::unordered_set<Entity*> Global::gameEntities;

std::unordered_map<std::string, OnlinePlayer*> Global::gameOnlinePlayers;

std::vector<std::string> Global::serverSettings;

std::mutex gameEntitiesAddMutex;
std::vector<Entity*> gameEntitiesToAdd;
std::vector<Entity*> gameEntitiesToDelete;

float Global::waterHeight = 0.0f;

float dt = 0;
double timeOld = 0;
double timeNew = 0;
Camera*             Global::gameCamera       = nullptr;
Player*             Global::player           = nullptr;
Entity*             Global::gameStageManager = nullptr;
Light*              Global::gameLightSun     = nullptr;
Light*              Global::gameLightMoon    = nullptr;

FontType* Global::gameFont = nullptr;

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
int Global::levelId = 0;
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

    srand((unsigned long)time(nullptr));

    Global::nickname = readFileLines("Nickname.ini")[0];
    if (Global::nickname == "nickname")
    {
        Global::nickname = "user_";
        for (int i = 0; i < 8; i++)
        {
            Global::nickname = Global::nickname + std::to_string((int)(Maths::random()*10));
        }
    }

    if (Global::nickname.size() >= 16)
    {
        char name[16];
        name[15] = 0;
        for (int i = 0; i < 15; i++)
        {
            name[i] = Global::nickname[i];
        }
        Global::nickname = name;
    }

    Global::countNew = 0;
    Global::countDelete = 0;

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

    Global::gameFont = new FontType(Loader::loadTexture("res/Fonts/consolas.png"), "res/Fonts/consolas.fnt"); INCR_NEW("FontType");

    TextMaster::init();

    GuiManager::init();
    GuiTextureResources::loadGuiTextures();

    AudioMaster::init();
    AudioPlayer::loadSoundEffects();

    Ball::loadModels();
    CollisionBlock::loadModels();
    OnlinePlayer::loadModels();
    RedBarrel::loadModels();

    Global::serverSettings = readFileLines("ServerSettings.ini");
    TcpClient* client = new TcpClient(Global::serverSettings[0].c_str(), std::stoi(Global::serverSettings[1]), 2); INCR_NEW("TcpClient");
    std::thread* t1 = nullptr;
    std::thread* t2 = nullptr;
    if (client->isOpen())
    {
        t1 = new std::thread(Global::readThreadBehavoir,  client); INCR_NEW("std::thread");
        t2 = new std::thread(Global::writeThreadBehavior, client); INCR_NEW("std::thread");
    }
    else
    {
        Global::addChatMessage("Could not connect to " + Global::serverSettings[0], Vector3f(1, 0.5f, 0.5f));
    }

    //This light never gets deleted.
    Light lightSun;
    Global::gameLightSun = &lightSun;
    lightSun.direction.set(-0.2f, -1, -0.4f);
    lightSun.direction.normalize();

    long long secSinceEpoch = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    glfwSetTime(0);

    Global::serverStartTime = Global::getRawUtcSystemTime();

    int frameCount = 0;
    double previousTime = 0;

    Global::gameState = STATE_RUNNING;

    std::string folder = "Map1";

    std::list<TexturedModel*> modelsStage;
    ObjLoader::loadModel(&modelsStage, "res/Models/" + folder + "/", "Map1");
    Dummy* entityStage = new Dummy(&modelsStage); INCR_NEW("Entity");
    Global::addEntity(entityStage);

    //std::list<TexturedModel*> modelsSphere;
    //ObjLoader::loadModel(&modelsSphere, "res/Models/", "Sphere");
    Global::player = new Player; INCR_NEW("Entity");

    CollisionModel* cm = ObjLoader::loadCollisionModel("Models/" + folder + "/", "Collision");
    for (int i = 0; i < cm->triangles.size(); i++)
    {
        CollisionChecker::addTriangle(cm->triangles[i]);
    }
    CollisionChecker::constructChunkDatastructure();

    //OnlinePlayer* npc1 = new OnlinePlayer("Npc1", 73.076897f, 0.313516f, 23.235739f); INCR_NEW("Entity");
    //OnlinePlayer* npc2 = new OnlinePlayer("Npc2", 72.076897f, 0.313516f, 23.235739f); INCR_NEW("Entity");
    //OnlinePlayer* npc3 = new OnlinePlayer("Npc3", 71.076897f, 0.313516f, 23.235739f); INCR_NEW("Entity");
    //OnlinePlayer* npc4 = new OnlinePlayer("Npc4", 70.076897f, 0.313516f, 23.235739f); INCR_NEW("Entity");

    Ball* ball1 = new Ball("B1", Vector3f(72.991318f, 28.784624f, -46.660568f), Vector3f(0, 0, 0)); INCR_NEW("Entity");
    //Ball* ball2 = new Ball("Ball2", Vector3f(72.076897f, 1.313516f, 23.235739f), Vector3f(20, 10, 30)); INCR_NEW("Entity");
    //Ball* ball3 = new Ball("Ball3", Vector3f(71.076897f, 1.313516f, 23.235739f), Vector3f(30, 20, 10)); INCR_NEW("Entity");

    CollisionBlock* cb1 = new CollisionBlock("CB1", Vector3f(38.7795f,  10.0f, 44.5082f), 0, 4.0f, 1.0f, 14.0f, true, 0); INCR_NEW("Entity");
    CollisionBlock* cb2 = new CollisionBlock("CB2", Vector3f(-28.9653f, 24.0f, -46.243f), 1, 4.0f, 2.1f, 20.0f, true, 0); INCR_NEW("Entity");
    //CollisionBlock* cb3 = new CollisionBlock("CollisionBlock3", Vector3f(73.076897f, 1.313516f, 43.235739f), 1, 4.0f, 1.0f, 12.0f, false, 0); INCR_NEW("Entity");
    //CollisionBlock* cb4 = new CollisionBlock("CollisionBlock4", Vector3f(73.076897f, 1.313516f, 53.235739f), 1, 4.0f, 1.0f, 12.0f, true,  0); INCR_NEW("Entity");

    //RedBarrel* barrel1 = new RedBarrel("Barrel1", Vector3f(86.722473f, 0.313497f, -4.892693f)); INCR_NEW("Entity");

    Ladder* ladder1 = new Ladder("L1", Vector3f(-14.7011f, 0.0f, -6.00807f), Vector3f(0.0568f/2, 5.29103f, 0.55678f/2)); INCR_NEW("Entity");

    //Global::gameEntities.insert(npc1);
    //Global::gameEntities.insert(npc2);
    //Global::gameEntities.insert(npc3);
    //Global::gameEntities.insert(npc4);

    Global::gameEntities.insert(ball1);
    //Global::gameEntities.insert(ball2);
    //Global::gameEntities.insert(ball3);

    Global::gameEntities.insert(cb1);
    Global::gameEntities.insert(cb2);
    //Global::gameEntities.insert(cb3);
    //Global::gameEntities.insert(cb4);

    //Global::gameEntities.insert(barrel1);

    Global::gameEntities.insert(ladder1);

    //x, y = (0, 0) is top left, (1, 1) is bottom right
    //size = 1.0 = full screen height
    //alignment chart:
    //  0 1 2
    //  3 4 5
    //  6 7 8
    //GUIText(std::string text, float size, FontType* font, float x, float y, int alignment, bool visible);

    GUIText* fpsText = new GUIText("0", 0.02f, Global::gameFont, 1.0f, 0.0f, 2, true);

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

        Global::syncedGlobalTime += (timeNew - timeOld);

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
        gameEntitiesAddMutex.lock();
        for (Entity* entityToAdd : gameEntitiesToAdd)
        {
            Global::gameEntities.insert(entityToAdd);
        }
        gameEntitiesToAdd.clear();

        std::unordered_set<Entity*> entitiesImGoingToDelete;
        for (Entity* entityToDelete : gameEntitiesToDelete)
        {
            entitiesImGoingToDelete.insert(entityToDelete);
            Global::gameEntities.erase(entityToDelete);
        }
        gameEntitiesToDelete.clear();
        gameEntitiesAddMutex.unlock();

        for (Entity* e : entitiesImGoingToDelete)
        {
            delete e; INCR_DEL("Entity");
        }

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

                for (Entity* e : Global::gameEntities)
                {
                    e->step();
                }

                if (Global::player != nullptr)
                {
                    Global::player->step();
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
        for (Entity* e : Global::gameEntities)
        {
            Master_processEntity(e);
        }

        if (Global::player != nullptr)
        {
            Master_processEntity(Global::player);
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

        Global::updateChatMessages();

        GuiManager::render();
        GuiManager::clearGuisToRender();

        TextMaster::render();

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
            fpsText->deleteMe();
            delete fpsText;
            fpsText = new GUIText(std::to_string(Global::currentCalculatedFPS), 0.02f, Global::gameFont, 1.0f, 0.0f, 2, true);
            //std::fprintf(stdout, "fps: %f\n", frameCount / (timeNew - previousTime));
            //printf("%f\t%f\n", (float)glfwGetTime(), 36.7816091954f*ball->vel.length());
            //std::fprintf(stdout, "diff: %d\n", Global::countNew - Global::countDelete);
            //Loader::printInfo();
            //std::fprintf(stdout, "entity counts: %d %d %d\n", gameEntities.size(), gameEntitiesPass2.size(), gameTransparentEntities.size());
            frameCount = 0;
            previousTime = timeNew;


            unsigned long long totalT = Global::getRawUtcSystemTime() - Global::serverStartTime;

            Global::syncedGlobalTime = totalT/10000000.0;

            //printf("time = %f\n", Global::syncedGlobalTime);
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

    Global::gameState = STATE_EXITING;

    Global::saveSaveData();

    Master_cleanUp();
    Loader::cleanUp();
    TextMaster::cleanUp();
    closeDisplay();

    if (t1 != nullptr)
    {
        t1->join();
        delete t1;
    }

    if (t2 != nullptr)
    {
        t2->join();
        delete t2;
    }

    return 0;
}

void Global::addEntity(Entity* entityToAdd)
{
    gameEntitiesAddMutex.lock();
    gameEntitiesToAdd.push_back(entityToAdd);
    gameEntitiesAddMutex.unlock();
}

void Global::deleteEntity(Entity* entityToDelete)
{
    gameEntitiesAddMutex.lock();
    gameEntitiesToDelete.push_back(entityToDelete);
    gameEntitiesAddMutex.unlock();
}

void Global::deleteAllEntites()
{
    std::unordered_set<Entity*> entitiesImGoingToDelete;

    gameEntitiesAddMutex.lock();
    //Make sure no entities get left behind in transition
    for (Entity* entityToAdd : gameEntitiesToAdd)
    {
        entitiesImGoingToDelete.insert(entityToAdd);
    }
    gameEntitiesToAdd.clear();

    for (Entity* entityToDelete : gameEntitiesToDelete)
    {
        entitiesImGoingToDelete.insert(entityToDelete);
    }
    gameEntitiesToDelete.clear();

    for (Entity* entityToDelete : gameEntities)
    {
        entitiesImGoingToDelete.insert(entityToDelete);
    }
    gameEntities.clear();

    for (Entity* e : entitiesImGoingToDelete)
    {
        delete e; INCR_DEL("Entity");
    }

    gameEntitiesAddMutex.unlock();

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
    

    
    //if (!SetPriorityClass(GetCurrentProcess(), IdLE_PRIORITY_CLASS))
    //{
    //    dwError = GetLastError();
    //    _tprintf(TEXT("Failed to enter below normal mode (%d)\n"), (int)dwError);
    //}
    //
    //if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IdLE))
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

unsigned long long Global::serverStartTime = 0;

unsigned long long Global::getRawUtcSystemTime()
{
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);

    unsigned long high = (unsigned long)ft.dwHighDateTime;
    unsigned long low  = (unsigned long)ft.dwLowDateTime;

    unsigned long long totalT;
    memcpy(&totalT, &low, 4);
    memcpy(((char*)&totalT) + 4, &high, 4);

    return totalT;
}

void Global::readThreadBehavoir(TcpClient* client)
{
    while (Global::gameState != STATE_EXITING && client->isOpen())
    {
        char cmd;
        int numRead = client->read(&cmd, 1, 5);
        if (numRead < 0)
        {
            printf("Could not read cmd from server\n");
            Global::addChatMessage("Disconnected from Server", Vector3f(1, 0.5f, 0.5f));
            return;
        }
        else if (numRead == 1)
        {
            switch (cmd)
            {
                case 0: //no nop
                    break;

                case 1: //time msg
                {
                    numRead = client->read((char*)&Global::serverStartTime, 8, 5);
                    if (numRead != 8)
                    {
                        printf("Could not read time from server\n");
                        Global::addChatMessage("Disconnected from Server", Vector3f(1, 0.5f, 0.5f));
                        return;
                    }
                    break;
                }

                //case 2 - 3 are all related to players
                case  2: //player update
                case  3:
                {
                    int nameLen;
                    numRead = client->read((char*)&nameLen, 4, 5);
                    if (numRead != 4)
                    {
                        printf("Could not read name len from server\n");
                        Global::addChatMessage("Disconnected from Server", Vector3f(1, 0.5f, 0.5f));
                        return;
                    }

                    if (nameLen >= 32)
                    {
                        printf("Player name is too big\n");
                        Global::addChatMessage("Disconnected from Server", Vector3f(1, 0.5f, 0.5f));
                        return;
                    }

                    char nameBuf[32] = {0};
                    numRead = client->read(nameBuf, nameLen, 5);
                    if (numRead != nameLen)
                    {
                        printf("Could not read name from server\n");
                        Global::addChatMessage("Disconnected from Server", Vector3f(1, 0.5f, 0.5f));
                        return;
                    }

                    std::string name = nameBuf;

                    OnlinePlayer* onlinePlayer = nullptr;

                    if (Global::gameOnlinePlayers.find(name) != Global::gameOnlinePlayers.end())
                    {
                        onlinePlayer = Global::gameOnlinePlayers[name];
                    }

                    switch (cmd)
                    {
                        case 2: //general purpose player update
                        {
                            if (onlinePlayer == nullptr)
                            {
                                onlinePlayer = new OnlinePlayer(name, 0, 0, 0); INCR_NEW("Entity");

                                gameEntitiesAddMutex.lock();
                                gameEntitiesToAdd.push_back(onlinePlayer);
                                gameEntitiesAddMutex.unlock();

                                Global::gameOnlinePlayers[name] = onlinePlayer;
                                printf("%s connected!", name.c_str());
                                Global::addChatMessage(name + " joined", Vector3f(0.5f, 1, 0.5f));
                            }

                            char buf[144];
                            client->read(buf, 144, 5);

                            int idx = 0;

                            memcpy((char*)&onlinePlayer->inputAction3,       &buf[idx], 1); idx+=1;
                            memcpy((char*)&onlinePlayer->inputAction4,       &buf[idx], 1); idx+=1;
                            memcpy((char*)&onlinePlayer->inputX,             &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->inputY,             &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->inputX2,            &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->inputY2,            &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->position.x,         &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->position.y,         &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->position.z,         &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->vel.x,              &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->vel.y,              &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->vel.z,              &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->groundNormal.x,     &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->groundNormal.y,     &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->groundNormal.z,     &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->wallNormal.x,       &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->wallNormal.y,       &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->wallNormal.z,       &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->onGround,           &buf[idx], 1); idx+=1;
                            memcpy((char*)&onlinePlayer->isTouchingWall,     &buf[idx], 1); idx+=1;
                            memcpy((char*)&onlinePlayer->slideTimer,         &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->storedSlideSpeed,   &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->wallJumpTimer,      &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->storedWallNormal.x, &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->storedWallNormal.y, &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->storedWallNormal.z, &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->eyeHeightSmooth,    &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->timeSinceOnGround,  &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->lastGroundNormal.x, &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->lastGroundNormal.y, &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->lastGroundNormal.z, &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->ladderTimer,        &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->isOnLadder,         &buf[idx], 1); idx+=1;
                            memcpy((char*)&onlinePlayer->externalVel.x,      &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->externalVel.y,      &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->externalVel.z,      &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->lookDir.x,          &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->lookDir.y,          &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->lookDir.z,          &buf[idx], 4); idx+=4;
                            memcpy((char*)&onlinePlayer->isCrouching,        &buf[idx], 1); idx+=1;
                            memcpy((char*)&onlinePlayer->weapon,             &buf[idx], 1); idx+=1;
                            memcpy((char*)&onlinePlayer->health,             &buf[idx], 1);

                            break;
                        }

                        case 3: //player disconnected
                            if (Global::gameOnlinePlayers.find(name) != Global::gameOnlinePlayers.end())
                            {
                                onlinePlayer = Global::gameOnlinePlayers[name];
                                Global::gameOnlinePlayers.erase(name);
                                printf("%s Disconnected.\n", name.c_str());
                                Global::addChatMessage(name + " left", Vector3f(1, 1, 0.5f));
                            }

                            if (onlinePlayer != nullptr)
                            {
                                gameEntitiesAddMutex.lock();
                                gameEntitiesToDelete.push_back(onlinePlayer);
                                gameEntitiesAddMutex.unlock();
                            }
                            else
                            {
                                printf("There is no player named %s\n", name.c_str());
                            }
                            break;

                        default:
                            break;
                    }
                    break;
                }

                case 4: //we have been hit by another player
                {
                    float x;
                    float y;
                    float z;
                    float xDir;
                    float yDir;
                    float zDir;
                    char weapon;

                    client->read((char*)&x,      4, 5);
                    client->read((char*)&y,      4, 5);
                    client->read((char*)&z,      4, 5);
                    client->read((char*)&xDir,   4, 5);
                    client->read((char*)&yDir,   4, 5);
                    client->read((char*)&zDir,   4, 5);
                    client->read((char*)&weapon, 1, 5);

                    Vector3f velToAdd(xDir, yDir, zDir);
                    if (weapon == 0)
                    {
                        velToAdd.setLength(6.0f);
                    }
                    else
                    {
                        velToAdd.setLength(30.0f);
                    }

                    Global::player->vel = Global::player->vel + velToAdd;
                    Global::player->health -= 10;
                    break;
                }

                case 5: //someone is sending us audio
                {
                    int sfxId;
                    Vector3f pos;

                    client->read((char*)&sfxId, 4, 5);
                    client->read((char*)&pos.x, 4, 5);
                    client->read((char*)&pos.y, 4, 5);
                    client->read((char*)&pos.z, 4, 5);

                    AudioPlayer::play(sfxId, &pos);
                    break;
                }

                default:
                    break;
            }
        }
        else
        {
            //printf("Read timed out\n");
        }

        //Sleep(1);
    }

    printf("Read thread all done\n");
}

void Global::writeThreadBehavior(TcpClient* client)
{
    // Send name message
    int nameLen = (int)Global::nickname.size();
    int numWritten = client->write((char*)&nameLen, 4, 5);
    if (numWritten != 4)
    {
        printf("Could not write name length to server\n");
        Global::addChatMessage("Disconnected from Server", Vector3f(1, 0.5f, 0.5f));
        return;
    }

    numWritten = client->write((char*)Global::nickname.c_str(), nameLen, 5);
    if (numWritten != nameLen)
    {
        printf("Could not write name to server\n");
        Global::addChatMessage("Disconnected from Server", Vector3f(1, 0.5f, 0.5f));
        return;
    }

    double lastSentTimeMsg = glfwGetTime();
    double lastSentPlayerMsg = glfwGetTime();

    const double PLAYER_UPDATE_INTERVAL = 0.01;

    while (Global::gameState != STATE_EXITING && client->isOpen())
    {
        std::unique_lock<std::mutex> lock{Global::msgOutMutex};
        Global::msgCondVar.wait_for(lock, std::chrono::milliseconds(1), [&]()
        {
            // If any of these conditions are true, aquire the lock
            return (Global::messagesToSend.size() > 0                          ||
                    glfwGetTime() - lastSentPlayerMsg > PLAYER_UPDATE_INTERVAL ||
                    Global::gameState == STATE_EXITING                         ||
                    !client->isOpen());
        });

        if (Global::messagesToSend.size() > 0)
        {
            for (Message msg : Global::messagesToSend)
            {
                client->write(msg.buf, msg.length, 5);
            }
            Global::messagesToSend.clear();
        }

        lock.unlock();

        if (glfwGetTime() - lastSentTimeMsg > 1.0)
        {
            unsigned long long time = getRawUtcSystemTime();
            // Send time message
            char cmd = 1;
            numWritten = client->write(&cmd, 1, 5);
            if (numWritten != 1)
            {
                printf("Could not write time command to server\n");
                Global::addChatMessage("Disconnected from Server", Vector3f(1, 0.5f, 0.5f));
                return;
            }
            
            numWritten = client->write((char*)&time, 8, 5);
            
            if (numWritten != 8)
            {
                printf("Could not write time to server\n");
                Global::addChatMessage("Disconnected from Server", Vector3f(1, 0.5f, 0.5f));
                return;
            }

            lastSentTimeMsg = glfwGetTime();
        }

        if (glfwGetTime() - lastSentPlayerMsg > PLAYER_UPDATE_INTERVAL)
        {
            //Send main player message
            if (Global::player != nullptr)
            {
                char buf[145];
                buf[0] = 2;

                int idx = 1;

                memcpy(&buf[idx], (char*)&Input::inputs.INPUT_ACTION3,        1); idx += 1;
                memcpy(&buf[idx], (char*)&Input::inputs.INPUT_ACTION4,        1); idx += 1;
                memcpy(&buf[idx], (char*)&Input::inputs.INPUT_X,              4); idx += 4;
                memcpy(&buf[idx], (char*)&Input::inputs.INPUT_Y,              4); idx += 4;
                memcpy(&buf[idx], (char*)&Input::inputs.INPUT_X2,             4); idx += 4;
                memcpy(&buf[idx], (char*)&Input::inputs.INPUT_Y2,             4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->position.x,         4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->position.y,         4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->position.z,         4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->vel.x,              4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->vel.y,              4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->vel.z,              4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->groundNormal.x,     4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->groundNormal.y,     4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->groundNormal.z,     4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->wallNormal.x,       4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->wallNormal.y,       4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->wallNormal.z,       4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->onGround,           1); idx += 1;
                memcpy(&buf[idx], (char*)&Global::player->isTouchingWall,     1); idx += 1;
                memcpy(&buf[idx], (char*)&Global::player->slideTimer,         4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->storedSlideSpeed,   4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->wallJumpTimer,      4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->storedWallNormal.x, 4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->storedWallNormal.y, 4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->storedWallNormal.z, 4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->eyeHeightSmooth,    4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->timeSinceOnGround,  4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->lastGroundNormal.x, 4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->lastGroundNormal.y, 4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->lastGroundNormal.z, 4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->ladderTimer,        4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->isOnLadder,         1); idx += 1;
                memcpy(&buf[idx], (char*)&Global::player->externalVelPrev.x,  4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->externalVelPrev.y,  4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->externalVelPrev.z,  4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->lookDir.x,          4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->lookDir.y,          4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->lookDir.z,          4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->isCrouching,        1); idx += 1;
                memcpy(&buf[idx], (char*)&Global::player->weapon,             1); idx += 1;
                memcpy(&buf[idx], (char*)&Global::player->health,             1);

                //printf("external vel length = %f\n", Global::player->externalVel.length());

                client->write(buf, 145, 5);
            }

            lastSentPlayerMsg = glfwGetTime();
        }
    }

    printf("Write thread all done\n");
}

std::mutex chatMsgMutex;
std::vector<std::string> chatMessagesToAdd;
std::vector<Vector3f> chatColorsToAdd;
std::vector<float> chatMessagesTimestamp;
std::vector<GUIText*> chatTexts;
GUIText* localChatText = nullptr;

void Global::addChatMessage(std::string msg, Vector3f color)
{
    chatMsgMutex.lock();
    chatMessagesToAdd.push_back(msg);
    chatColorsToAdd.push_back(color);
    chatMsgMutex.unlock();
}

void Global::updateChatMessages()
{
    float currTime = (float)glfwGetTime();

    chatMsgMutex.lock();

    for (int i = 0; i < (int)chatMessagesToAdd.size(); i++)
    {
        std::string msg = chatMessagesToAdd[i];
        if (msg.size() >= 45)
        {
            char buf[46];
            buf[45] = 0;
            for (int c = 0; c < 45; c++)
            {
                buf[c] = msg[c];
            }
            msg = buf;
        }

        GUIText* tex = new GUIText(msg, 0.02f, Global::gameFont, 0.01f, 0.99f-0.025f, 6, true); INCR_NEW("GUIText");
        tex->color = chatColorsToAdd[i];

        for (int c = 0; c < chatTexts.size(); c++)
        {
            GUIText* t = chatTexts[c];
            t->position.y -= 0.025f;
        }

        chatTexts.push_back(tex);
        chatMessagesTimestamp.push_back(currTime);
    }
    chatMessagesToAdd.clear();
    chatColorsToAdd.clear();

    while (chatTexts.size() > 12)
    {
        GUIText* t = chatTexts.front();
        t->deleteMe();
        delete t; INCR_DEL("GUIText");
        chatTexts.erase(chatTexts.begin());
        chatMessagesTimestamp.erase(chatMessagesTimestamp.begin());
    }

    bool keepGoing = true;
    while (keepGoing && chatTexts.size() > 0)
    {
        float oldestTime = chatMessagesTimestamp.front();
        if (currTime - oldestTime > 6.0f)
        {
            keepGoing = true;

            GUIText* t = chatTexts.front();
            t->deleteMe();
            delete t; INCR_DEL("GUIText");
            chatTexts.erase(chatTexts.begin());
            chatMessagesTimestamp.erase(chatMessagesTimestamp.begin());
        }
        else
        {
            keepGoing = false;
        }
    }

    float highestAlpha = 0.0f;

    for (int i = 0; i < chatTexts.size(); i++)
    {
        GUIText* t = chatTexts[i];
        t->alpha = 0.85f;
        float tim = chatMessagesTimestamp[i];

        float diff = currTime - tim;
        if (diff > 5.0f)
        {
            float a = 1.0f - (diff - 5.0f);
            if (a < 0.0f)
            {
                a = 0.0f;
            }
            t->alpha = a*0.85f;

            if (a > highestAlpha)
            {
                highestAlpha = a;
            }
        }
        else
        {
            highestAlpha = 1.0f;
        }
    }

    chatMsgMutex.unlock();

    if (chatTexts.size() > 0)
    {
        GuiManager::addGuiToRender(GuiTextureResources::textureChatBG);
        GuiTextureResources::textureChatBG->size.y = chatTexts.size()*0.025f;
        GuiTextureResources::textureChatBG->alpha = highestAlpha*0.7f;
    }

    if (Input::localChatHasBeenUpdated)
    {
        if (localChatText != nullptr)
        {
            localChatText->deleteMe();
            delete localChatText; INCR_DEL("GUIText");
            localChatText = nullptr;
        }

        if (Input::isTypingInChat)
        {
            GuiManager::addGuiToRender(GuiTextureResources::textureLocalChatBG);

            if (Input::chatLength > 0)
            {
                localChatText = new GUIText(Input::chatInput, 0.02f, Global::gameFont, 0.01f, 0.99f, 6, true); INCR_NEW("GUIText");
            }
        }
    }
}

std::vector<Message> Global::messagesToSend;
std::mutex Global::msgOutMutex;
std::condition_variable Global::msgCondVar;

void Global::sendMessageToServer(Message msg)
{
    Global::msgOutMutex.lock();
    Global::messagesToSend.push_back(msg);
    Global::msgOutMutex.unlock();
}

void Global::sendAudioMessageToServer(int sfxId, Vector3f* position)
{
    Message msg;
    msg.buf[0] = 5;
    msg.length = 1 + 4 + 12;
    memcpy(&msg.buf[ 1], &sfxId, 4);
    memcpy(&msg.buf[ 5], &position->x, 4);
    memcpy(&msg.buf[ 9], &position->y, 4);
    memcpy(&msg.buf[13], &position->z, 4);

    Global::sendMessageToServer(msg);
}
