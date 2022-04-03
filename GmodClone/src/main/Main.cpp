#ifdef _WIN32
#define _CRT_SECURE_NO_DEPRECATE
#define NOMINMAX
#endif

#ifdef _WIN32
#include <winsock2.h>
#include <Windows.h>
#include <ws2tcpip.h>
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
#include <shared_mutex>
#include <condition_variable>
#include <algorithm>

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
#include "../loader/objloader.hpp"
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
#include "../entities/healthcube.hpp"
#include "../entities/glass.hpp"
#include "../toolbox/levelloader.hpp"
#include "../entities/boombox.hpp"
#include "../audio/source.hpp"
#include "../entities/rockplatform.hpp"
#include "../entities/chandelier.hpp"
#include "../entities/fenceplatform.hpp"
#include "../entities/stepfallplatform.hpp"
#include "../entities/bat.hpp"
#include "../entities/winzone.hpp"
#include "../entities/onlineplayer.hpp"
#include "../entities/fallblock.hpp"

Message::Message(const Message &other)
{
    length = other.length;
    for (int i = 0; i < 250; i++)
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

std::shared_mutex Global::gameOnlinePlayersSharedMutex;
std::unordered_map<std::string, OnlinePlayer*> Global::gameOnlinePlayers;
std::vector<GUIText*> Global::gameOnlinePlayerNametagsToDelete;
std::vector<GUIText*> Global::gameOnlinePlayerPingTexts;

std::vector<std::string> Global::serverSettings;
TcpClient* Global::serverClient = nullptr;
int Global::pingToServer = 0;

std::shared_mutex Global::gameEntitiesSharedMutex;
std::unordered_set<Entity*> Global::gameEntities;
std::vector<Entity*> gameEntitiesToAdd;
std::vector<Entity*> gameEntitiesToDelete;

float dt = 0;
double timeOld = 0;
double timeNew = 0;
Camera* Global::gameCamera   = nullptr;
Player* Global::player       = nullptr;

std::list<TexturedModel*> Global::stageModels;
Dummy* Global::stageEntity  = nullptr;

Light* Global::lights[4] = {nullptr, nullptr, nullptr, nullptr};
Vector3f Global::skyColor(1, 1, 1);

FontType* Global::fontConsolas = nullptr;

bool Global::renderParticles = true;

bool Global::framerateUnlock = false;

bool Global::camThirdPerson = true;

bool Global::useFullscreen = false;

extern unsigned int SCR_WIDTH;
extern unsigned int SCR_HEIGHT;

int Global::countNew = 0;
int Global::countDelete = 0;
int Global::gameState = 0;

int Global::gameTotalPlaytime = 0;

int Global::levelId = LVL_HUB;
std::shared_mutex Global::levelMutex;
std::string Global::levelToLoad = "";

bool Global::renderWithCulling = false;
bool Global::displayFPS = true;
//float Global::fpsTarget = 120.0f;
float Global::fpsLimit = 60.0f;
int Global::currentCalculatedFPS = 0;
int Global::renderCount = 0;

std::unordered_map<std::string, int> heapObjects;

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

    Maths::initRandom((unsigned long)time(nullptr));

    Maths::random();

    Global::nickname = readFileLines("Nickname.ini")[0];
    if (Global::nickname == "nickname")
    {
        Global::nickname = "user_";
        int n = (int)(8*Maths::random()) + 4;
        for (int i = 0; i < n; i++)
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

    #if !defined(DEV_MODE) && defined(_WIN32)
    FreeConsole();
    #endif

    Input::init();

    //This camera is never deleted.
    Camera cam;
    Global::gameCamera = &cam;

    Master_init();

    Global::fontConsolas = new FontType(Loader::loadTexture("res/Fonts/consolas.png"), "res/Fonts/consolas.fnt"); INCR_NEW("FontType");

    TextMaster::init();

    GuiManager::init();
    GuiTextureResources::loadGuiTextures();

    AudioMaster::init();
    AudioPlayer::loadSoundEffects();
    AudioPlayer::loadBGM();

    Ball::loadModels();
    CollisionBlock::loadModels();
    OnlinePlayer::loadModels();
    RedBarrel::loadModels();
    HealthCube::loadModels();
    Glass::loadModels();
    Ladder::loadModels();
    BoomBox::loadModels();
    RockPlatform::loadModels();
    Chandelier::loadModels();
    FencePlatform::loadModels();
    StepFallPlatform::loadModels();
    Bat::loadModels();
    WinZone::loadModels();
    FallBlock::loadModels();

    Global::serverSettings = readFileLines("ServerSettings.ini");
    Global::serverClient = new TcpClient(Global::serverSettings[0].c_str(), std::stoi(Global::serverSettings[1]), 1); INCR_NEW("TcpClient");
    std::thread* t1 = nullptr;
    std::thread* t2 = nullptr;
    if (Global::serverClient->isOpen())
    {
        t1 = new std::thread(Global::readThreadBehavoir,  Global::serverClient); INCR_NEW("std::thread");
        t2 = new std::thread(Global::writeThreadBehavior, Global::serverClient); INCR_NEW("std::thread");
    }
    else
    {
        Global::addChatMessage("Could not connect to " + Global::serverSettings[0], Vector3f(1, 0.5f, 0.5f));
    }

    // These lights never get deleted
    Global::lights[0] = new Light; INCR_NEW("Light");
    Global::lights[1] = new Light; INCR_NEW("Light");
    Global::lights[2] = new Light; INCR_NEW("Light");
    Global::lights[3] = new Light; INCR_NEW("Light");

    // The sun
    Global::lights[0]->direction.set(-0.2f, -1, -0.4f);
    Global::lights[0]->direction.normalize();
    Global::lights[0]->attenuation.set(1, 0, 0);

    long long secSinceEpoch = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    glfwSetTime(0);

    Global::serverTimeOffset = 0.0;

    int frameCount = 0;
    double previousTime = 0;

    Global::gameState = GAME_STATE_RUNNING;

    Global::stageEntity = new Dummy(&Global::stageModels); INCR_NEW("Entity");

    Global::player = new Player; INCR_NEW("Entity");

    LevelLoader::loadLevel("hub");
    Global::timeUntilRoundStarts = -1.0f;
    Global::timeUntilRoundEnds = 10000000.0f;

    GUIText* fpsText = new GUIText("0", 0.02f, Global::fontConsolas, 1.0f, 0.0f, 2, true); INCR_NEW("GUIText");

    while (displayWantsToClose() == 0 &&
           Global::gameState != GAME_STATE_EXITING &&
           Global::gameState != GAME_STATE_WINDOWCLOSE)
    {
        ANALYSIS_START("Frame Time");

        timeNew = glfwGetTime();

        //spin lock to meet the target fps, and gives extremely consistent dt's.
        // also of course uses a ton of cpu.
        if (Global::gameState == GAME_STATE_RUNNING && Global::framerateUnlock && Global::fpsLimit > 0.0f)
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

        float timeUntilRoundStartsBefore = Global::timeUntilRoundStarts;
        Global::timeUntilRoundStarts -= dt;

        if (timeUntilRoundStartsBefore > 3.0f && Global::timeUntilRoundStarts <= 3.0f)
        {
            AudioPlayer::play(63, nullptr);
            if (Global::timeUntilRoundStartsText != nullptr)
            {
                Global::timeUntilRoundStartsText->deleteMe();
                delete Global::timeUntilRoundStartsText; INCR_DEL("GUIText");
            }
            Global::timeUntilRoundStartsText = new GUIText("3", 0.333f, Global::fontConsolas, 0.5f, 0.5f, 4, true);
        }
        if (timeUntilRoundStartsBefore > 2.0f && Global::timeUntilRoundStarts <= 2.0f)
        {
            if (Global::timeUntilRoundStartsText != nullptr)
            {
                Global::timeUntilRoundStartsText->deleteMe();
                delete Global::timeUntilRoundStartsText; INCR_DEL("GUIText");
            }
            Global::timeUntilRoundStartsText = new GUIText("2", 0.333f, Global::fontConsolas, 0.5f, 0.5f, 4, true);
        }
        if (timeUntilRoundStartsBefore > 1.0f && Global::timeUntilRoundStarts <= 1.0f)
        {
            if (Global::timeUntilRoundStartsText != nullptr)
            {
                Global::timeUntilRoundStartsText->deleteMe();
                delete Global::timeUntilRoundStartsText; INCR_DEL("GUIText");
            }
            Global::timeUntilRoundStartsText = new GUIText("1", 0.333f, Global::fontConsolas, 0.5f, 0.5f, 4, true);
        }
        if (Global::timeUntilRoundStarts <= 0.0f)
        {
            if (Global::timeUntilRoundStartsText != nullptr)
            {
                Global::timeUntilRoundStartsText->deleteMe();
                delete Global::timeUntilRoundStartsText; INCR_DEL("GUIText");
                Global::timeUntilRoundStartsText = nullptr;
            }
        }

        if (Global::timeUntilRoundEnds > 0.0f && Global::timeUntilRoundStarts <= 0.0f)
        {
            Global::timeUntilRoundEnds -= dt;

            if (Global::timeUntilRoundEnds <= 0.0f)
            {
                float px = Global::player->position.x;
                float py = Global::player->position.y;
                float pz = Global::player->position.z;

                if (px > Global::safeZoneStart.x &&
                    py > Global::safeZoneStart.y &&
                    pz > Global::safeZoneStart.z &&
                    px < Global::safeZoneEnd.x   &&
                    py < Global::safeZoneEnd.y   &&
                    pz < Global::safeZoneEnd.z)
                {
                    //safe
                }
                else
                {
                    //dead
                    if (Global::player->health > 0)
                    {
                        AudioPlayer::play(66, nullptr);
                        Global::player->health = 0;
                    }
                }

                if (Global::levelId == LVL_MAP6)
                {
                    std::vector<float> playerZoneTimes;
                    playerZoneTimes.push_back(Global::player->inZoneTime);

                    Global::gameOnlinePlayersSharedMutex.lock_shared();
                    for (auto const& entry : Global::gameOnlinePlayers)
                    {
                        playerZoneTimes.push_back(entry.second->inZoneTime);
                    }
                    Global::gameOnlinePlayersSharedMutex.unlock_shared();

                    if (playerZoneTimes.size() > 1)
                    {
                        std::sort(playerZoneTimes.begin(), playerZoneTimes.end());

                        float middleTime = playerZoneTimes[playerZoneTimes.size()/2];

                        if (Global::player->inZoneTime < middleTime)
                        {
                            AudioPlayer::play(66, nullptr);
                            Global::player->health = 0;
                        }
                    }
                }
            }
        }

        Input::pollInputs();

        frameCount++;
        Global::renderCount++;

        long long nextSecSinceEpoch = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        int epocSecDiff = (int)(nextSecSinceEpoch - secSinceEpoch);
        secSinceEpoch = nextSecSinceEpoch;
        Global::gameTotalPlaytime+=epocSecDiff;

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

        ANALYSIS_START("Entity Management");
        //entities managment
        if (gameEntitiesToAdd.size() > 0)
        {
            Global::gameEntitiesSharedMutex.lock();
            for (Entity* entityToAdd : gameEntitiesToAdd)
            {
                Global::gameEntities.insert(entityToAdd);
            }
            gameEntitiesToAdd.clear();
            Global::gameEntitiesSharedMutex.unlock();
        }

        if (gameEntitiesToDelete.size() > 0)
        {
            Global::gameEntitiesSharedMutex.lock();
            std::unordered_set<Entity*> entitiesImGoingToDelete;
            for (Entity* entityToDelete : gameEntitiesToDelete)
            {
                entitiesImGoingToDelete.insert(entityToDelete);
                Global::gameEntities.erase(entityToDelete);
            }
            gameEntitiesToDelete.clear();
            Global::gameEntitiesSharedMutex.unlock();

            for (Entity* e : entitiesImGoingToDelete)
            {
                delete e; INCR_DEL("Entity");
            }
        }

        ANALYSIS_DONE("Entity Management");

        switch (Global::gameState)
        {
            case GAME_STATE_RUNNING:
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

                for (Entity* e : Global::gameEntities)
                {
                    e->step();
                }

                Global::gameOnlinePlayersSharedMutex.lock_shared();
                for (auto const& e : Global::gameOnlinePlayers)
                {
                    e.second->step();
                }
                Global::gameOnlinePlayersSharedMutex.unlock_shared();

                Global::player->step();

                ModelTexture::updateAnimations(dt);
                Global::gameCamera->refresh();
                //ParticleMaster::update(Global::gameCamera);

                ANALYSIS_DONE("Object Logic");
                break;
            }

            case GAME_STATE_PAUSED:
            {
                //vsync during pausing. no need to stress the system.
                glfwSwapInterval(1);
                break;
            }

            case GAME_STATE_TITLE:
            {
                //vsync during title. no need to stress the system.
                glfwSwapInterval(1);
                Global::gameCamera->refresh();
                //ParticleMaster::update(Global::gameCamera);
                break;
            }

            default:
                break;
        }

        Vector3f camVel = cam.vel.scaleCopy(0.016666f);
        AudioMaster::updateListenerData(&cam.eye, &cam.target, &cam.up, &camVel);
        AudioPlayer::setListenerIsUnderwater(false);

        Global::updateMusic();

        if (Global::timeUntilRoundEnds > 1000.0f)
        {
            if (Global::timeUntilRoundEndText != nullptr)
            {
                Global::timeUntilRoundEndText->deleteMe();
                delete Global::timeUntilRoundEndText; INCR_DEL("GUIText");
                Global::timeUntilRoundEndText = nullptr;
            }
        }
        else
        {
            std::string timeLeft = std::to_string((int)Global::timeUntilRoundEnds);
            if (Global::timeUntilRoundEndText == nullptr)
            {
                Global::timeUntilRoundEndText = new GUIText(timeLeft, 0.05f, Global::fontConsolas, 0.993f, 0.965f - 0.05f, 8, true); INCR_NEW("GUIText");
            }
            else if (Global::timeUntilRoundEndText->textString != timeLeft)
            {
                Global::timeUntilRoundEndText->deleteMe();
                delete Global::timeUntilRoundEndText; INCR_DEL("GUIText");
                Global::timeUntilRoundEndText = new GUIText(timeLeft, 0.05f, Global::fontConsolas, 0.993f, 0.965f - 0.05f, 8, true); INCR_NEW("GUIText");
            }
        }

        //prepare entities to render
        for (Entity* e : Global::gameEntities)
        {
            Master_processEntity(e);
        }

        Global::gameOnlinePlayersSharedMutex.lock_shared();
        for (auto const& e : Global::gameOnlinePlayers)
        {
            Master_processEntity(e.second);
        }
        Global::gameOnlinePlayersSharedMutex.unlock_shared();

        Master_processEntity(Global::player);
        Master_processEntity(Global::stageEntity);

        Master_render(&cam, 0, 0, 0, 0, 0.0f);

        Master_clearAllEntities();

        Global::updateChatMessages();

        if (Global::player->weapon == WEAPON_GUN)
        {
            GuiManager::addGuiToRender(GuiTextureResources::textureCrosshair);
        }

        extern GLFWwindow* window;
        if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS)
        {
            GuiManager::addGuiToRender(GuiTextureResources::texturePlayersBG);
            GuiTextureResources::texturePlayersBG->size.y = 0.05f*(1 + (int)Global::gameOnlinePlayers.size()) + 0.01f;
            if (Global::gameOnlinePlayerPingTexts.size() == 0)
            {
                {
                    std::string t = Global::nickname;
                    int p = Global::pingToServer;

                    int spacesToDraw = 22 - ((int)t.size() + Maths::numDigits(p));

                    for (int i = 0; i < spacesToDraw; i++)
                    {
                        t = t + " ";
                    }

                    t = t + std::to_string(p);

                    t = t + "ms";

                    GUIText* mePing = new GUIText(t, 0.05f, Global::fontConsolas, 0.5f, 0.1f, 1, true); INCR_NEW("GUIText");
                    Global::gameOnlinePlayerPingTexts.push_back(mePing);
                }

                int n = 1;
                Global::gameOnlinePlayersSharedMutex.lock_shared();
                for (auto const& entry : Global::gameOnlinePlayers)
                {
                    std::string t = entry.first;
                    int p = entry.second->pingMs;

                    int spacesToDraw = 22 - ((int)t.size() + Maths::numDigits(p));

                    for (int i = 0; i < spacesToDraw; i++)
                    {
                        t = t + " ";
                    }

                    t = t + std::to_string(p);

                    t = t + "ms";

                    GUIText* text = new GUIText(t, 0.05f, Global::fontConsolas, 0.5f, 0.1f + 0.05f*n, 1, true); INCR_NEW("GUIText");
                    Global::gameOnlinePlayerPingTexts.push_back(text);

                    n++;
                }
                Global::gameOnlinePlayersSharedMutex.unlock_shared();
            }
        }

        if (Global::gameOnlinePlayerNametagsToDelete.size() > 0)
        {
            Global::gameOnlinePlayersSharedMutex.lock();
            for (GUIText* text : Global::gameOnlinePlayerNametagsToDelete)
            {
                text->deleteMe();
                delete text; INCR_DEL("GUIText");
            }
            Global::gameOnlinePlayerNametagsToDelete.clear();
            Global::gameOnlinePlayersSharedMutex.unlock();
        }

        if (Global::timeUntilRoundStarts > 5.0f)
        {
            GuiManager::addGuiToRender(GuiTextureResources::textureMapBG);

            float a = Maths::clamp(0.0f, (11.75f - Global::timeUntilRoundStarts)/2.0f, 1.0f);

            switch (Global::levelId)
            {
                case LVL_MAP1: GuiManager::addGuiToRender(GuiTextureResources::textureMap1); GuiTextureResources::textureMap1->alpha = a; break;
                case LVL_MAP2: GuiManager::addGuiToRender(GuiTextureResources::textureMap2); GuiTextureResources::textureMap2->alpha = a; break;
                case LVL_MAP4: GuiManager::addGuiToRender(GuiTextureResources::textureMap4); GuiTextureResources::textureMap4->alpha = a; break;
                case LVL_MAP5: GuiManager::addGuiToRender(GuiTextureResources::textureMap5); GuiTextureResources::textureMap5->alpha = a; break;
                case LVL_MAP6: GuiManager::addGuiToRender(GuiTextureResources::textureMap6); GuiTextureResources::textureMap6->alpha = a; break;
                case LVL_MAP7: GuiManager::addGuiToRender(GuiTextureResources::textureMap7); GuiTextureResources::textureMap7->alpha = a; break;
                default: break;
            }
        }

        GuiManager::render();
        GuiManager::clearGuisToRender();

        if (Global::timeUntilRoundStarts < 5.0f)
        {
            TextMaster::render();
        }

        updateDisplay();

        if (previousTime > timeNew)
        {
            previousTime = timeNew;
        }

        //if (glfwGetKey(window, GLFW_KEY_TAB) != GLFW_PRESS)
        {
            if (Global::gameOnlinePlayerPingTexts.size() > 0)
            {
                for (GUIText* t : Global::gameOnlinePlayerPingTexts)
                {
                    t->deleteMe();
                    delete t; INCR_DEL("GUIText");
                }
                Global::gameOnlinePlayerPingTexts.clear();
            }
        }

        if (timeNew - previousTime >= 1.0)
        {
            Global::currentCalculatedFPS = (int)(std::round(frameCount/(timeNew - previousTime)));
            if (fpsText != nullptr)
            {
                fpsText->deleteMe();
                delete fpsText; INCR_DEL("GUIText");
                fpsText = nullptr;
            }

            if (Global::displayFPS)
            {
                fpsText = new GUIText(std::to_string(Global::currentCalculatedFPS), 0.02f, Global::fontConsolas, 1.0f, 0.0f, 2, true); INCR_NEW("GUIText");
            }
            //std::fprintf(stdout, "fps: %f\n", frameCount / (timeNew - previousTime));
            //printf("%f\t%f\n", (float)glfwGetTime(), 36.7816091954f*ball->vel.length());

            //std::fprintf(stdout, "diff: %d\n", Global::countNew - Global::countDelete);
            //std::unordered_map<std::string, int>::iterator it;
            //for (it = heapObjects.begin(); it != heapObjects.end(); it++)
            //{
                //printf("%s: %d\n", it->first.c_str(), it->second);
            //}
            //printf("\n");

            //Loader::printInfo();
            //std::fprintf(stdout, "entity counts: %d %d %d\n", gameEntities.size(), gameEntitiesPass2.size(), gameTransparentEntities.size());
            frameCount = 0;
            previousTime = timeNew;


            //unsigned long long totalT = Global::getRawUtcSystemTime() - Global::serverStartTime;

            Global::syncedGlobalTime = glfwGetTime() + Global::serverTimeOffset;

            //printf("time = %f\n", Global::syncedGlobalTime);
        }

        Global::levelMutex.lock();
        if (Global::levelToLoad != "")
        {
            LevelLoader::loadLevel(Global::levelToLoad);
            Global::levelToLoad = "";
        }
        Global::levelMutex.unlock();

        //std::fprintf(stdout, "dt: %f\n", dt);

        ANALYSIS_DONE("Frame Time");
        ANALYSIS_REPORT();
    }

    Global::gameState = GAME_STATE_EXITING;

    Master_cleanUp();
    Loader::cleanUp();
    TextMaster::cleanUp();
    closeDisplay();

    if (t1 != nullptr)
    {
        t1->join();
        delete t1; INCR_DEL("std::thread");
    }

    if (t2 != nullptr)
    {
        t2->join();
        delete t2; INCR_DEL("std::thread");
    }

    return 0;
}

void Global::addEntity(Entity* entityToAdd)
{
    Global::gameEntitiesSharedMutex.lock();
    gameEntitiesToAdd.push_back(entityToAdd);
    Global::gameEntitiesSharedMutex.unlock();
}

void Global::deleteEntity(Entity* entityToDelete)
{
    Global::gameEntitiesSharedMutex.lock();
    gameEntitiesToDelete.push_back(entityToDelete);
    Global::gameEntitiesSharedMutex.unlock();
}

void Global::deleteAllEntites()
{
    std::unordered_set<Entity*> entitiesImGoingToDelete;

    Global::gameEntitiesSharedMutex.lock();
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

    Global::gameEntitiesSharedMutex.unlock();
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

float Global::calcAspectRatio()
{
    extern unsigned int SCR_WIDTH;
    extern unsigned int SCR_HEIGHT;
    return ((float)SCR_WIDTH)/SCR_HEIGHT;
}

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

double Global::serverTimeOffset = 0.0;

bool checkConnection(int numBytesExpected, int numBytesActual, std::string message, TcpClient* client)
{
    if (!client->isOpen() || numBytesActual != numBytesExpected)
    {
        printf("Error: Check connection failed because '%s'\n", message.c_str());
        printf("       Expected bytes: %d. Bytes actual: %d\n", numBytesExpected, numBytesActual);

        return false;
    }

    return true;
}

void Global::readThreadBehavoir(TcpClient* client)
{
    #define CHECK_CONNECTION_R(NUM_BYTES, MESSAGE) if (!checkConnection(NUM_BYTES, numRead, MESSAGE, client)) { Global::addChatMessage("Disconnected from Server", Vector3f(1, 0.5f, 0.5f)); return; }

    while (Global::gameState != GAME_STATE_EXITING && client->isOpen())
    {
        char cmd;
        int numRead = client->read(&cmd, 1, 5);

        if (numRead == 1)
        {
            switch (cmd)
            {
                case 0: //no nop
                    printf("Warning: Read a No Op messsage from the server.\n");
                    break;

                case 1: //sync server time msg
                {
                    double serverTime;
                    numRead = client->read(&serverTime, 8, 5); CHECK_CONNECTION_R(8, "Could not read initial server time message");
                    Global::serverTimeOffset = serverTime - glfwGetTime();
                    break;
                }

                //case 2 - 3 are all related to players
                case  2: //player update
                case  3:
                {
                    int nameLen;
                    numRead = client->read(&nameLen, 4, 5); CHECK_CONNECTION_R(4, "Could not read player name len");

                    if (nameLen >= 32)
                    {
                        printf("Player name is too big (%d)\n", nameLen);
                        Global::addChatMessage("Disconnected from Server", Vector3f(1, 0.5f, 0.5f));
                        return;
                    }

                    char nameBuf[33] = {0};
                    numRead = client->read(nameBuf, nameLen, 5); CHECK_CONNECTION_R(nameLen, "Could not read player name");

                    std::string name = nameBuf;

                    OnlinePlayer* onlinePlayer = nullptr;

                    Global::gameOnlinePlayersSharedMutex.lock_shared();
                    if (Global::gameOnlinePlayers.find(name) != Global::gameOnlinePlayers.end())
                    {
                        onlinePlayer = Global::gameOnlinePlayers[name];
                    }
                    Global::gameOnlinePlayersSharedMutex.unlock_shared();

                    switch (cmd)
                    {
                        case 2: //general purpose player update
                        {
                            if (onlinePlayer == nullptr)
                            {
                                onlinePlayer = new OnlinePlayer(name, 0, 0, 0); INCR_NEW("OnlinePlayer");

                                Global::gameOnlinePlayersSharedMutex.lock();
                                Global::gameOnlinePlayers[name] = onlinePlayer;
                                Global::gameOnlinePlayersSharedMutex.unlock();
                                printf("%s connected.\n", name.c_str());
                                Global::addChatMessage(name + " joined", Vector3f(0.5f, 1, 0.5f));
                            }

                            char buf[223];
                            numRead = client->read(buf, 223, 5); CHECK_CONNECTION_R(223, "Could not read player update");

                            int idx = 0;

                            double otherServerSyncedTime;

                            memcpy(&onlinePlayer->inputAction3,       &buf[idx], 1); idx+=1;
                            memcpy(&onlinePlayer->inputAction4,       &buf[idx], 1); idx+=1;
                            memcpy(&onlinePlayer->inputX,             &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->inputY,             &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->inputX2,            &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->inputY2,            &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->position.x,         &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->position.y,         &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->position.z,         &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->vel.x,              &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->vel.y,              &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->vel.z,              &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->groundNormal.x,     &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->groundNormal.y,     &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->groundNormal.z,     &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->wallNormal.x,       &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->wallNormal.y,       &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->wallNormal.z,       &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->onGround,           &buf[idx], 1); idx+=1;
                            memcpy(&onlinePlayer->isTouchingWall,     &buf[idx], 1); idx+=1;
                            memcpy(&onlinePlayer->slideTimer,         &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->storedSlideSpeed,   &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->wallJumpTimer,      &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->storedWallNormal.x, &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->storedWallNormal.y, &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->storedWallNormal.z, &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->eyeHeightSmooth,    &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->timeSinceOnGround,  &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->lastGroundNormal.x, &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->lastGroundNormal.y, &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->lastGroundNormal.z, &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->ladderTimer,        &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->isOnLadder,         &buf[idx], 1); idx+=1;
                            memcpy(&onlinePlayer->externalVel.x,      &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->externalVel.y,      &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->externalVel.z,      &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->lookDir.x,          &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->lookDir.y,          &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->lookDir.z,          &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->isCrouching,        &buf[idx], 1); idx+=1;
                            memcpy(&onlinePlayer->weapon,             &buf[idx], 1); idx+=1;
                            memcpy(&onlinePlayer->health,             &buf[idx], 1); idx+=1;
                            memcpy(&onlinePlayer->inZoneTime,         &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->isOnRope,           &buf[idx], 1); idx+=1;
                            memcpy(&onlinePlayer->ropeAnchor.x,       &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->ropeAnchor.y,       &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->ropeAnchor.z,       &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->ropeLength,         &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->animTimerStand,     &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->animTimerWalk,      &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->animTimerRun,       &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->animTimerCrouch,    &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->animTimerSlide,     &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->animTimerJump,      &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->animTimerFall,      &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->animTimerClimb,     &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->animTimerSwing,     &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->animTimerCrawl,     &buf[idx], 4); idx+=4;
                            memcpy(&onlinePlayer->animType,           &buf[idx], 1); idx+=1;
                            memcpy(&onlinePlayer->animTypePrevious,   &buf[idx], 1); idx+=1;
                            memcpy(&onlinePlayer->animBlend,          &buf[idx], 4); idx+=4;
                            memcpy(&otherServerSyncedTime,            &buf[idx], 8); idx+=8;
                            memcpy(&onlinePlayer->pingMs,             &buf[idx], 4);

                            double myServerSyncedTime = glfwGetTime() + Global::serverTimeOffset;

                            double diff = myServerSyncedTime - otherServerSyncedTime;

                            // If diff is > 0, the other players position is old and we should interpolate it to a newer value.
                            // If diff is < 0, the other players position is "in the future" relative to us and so we should interpolate it backwards.
                            Vector3f distToMove = onlinePlayer->vel + onlinePlayer->externalVel;
                            onlinePlayer->position = onlinePlayer->position + distToMove.scaleCopy((float)diff);

                            break;
                        }

                        case 3: //player disconnected
                            Global::gameOnlinePlayersSharedMutex.lock();
                            if (Global::gameOnlinePlayers.find(name) != Global::gameOnlinePlayers.end())
                            {
                                onlinePlayer = Global::gameOnlinePlayers[name];
                                Global::gameOnlinePlayers.erase(name);
                                printf("%s disconnected.\n", name.c_str());
                                Global::addChatMessage(name + " left", Vector3f(1, 1, 0.5f));
                            }
                            Global::gameOnlinePlayersSharedMutex.unlock();

                            if (onlinePlayer != nullptr)
                            {
                                printf("Deleting player %s\n", onlinePlayer->name.c_str());
                                delete onlinePlayer; INCR_DEL("OnlinePlayer");
                                onlinePlayer = nullptr;
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

                    numRead = client->read(&x,      4, 5); CHECK_CONNECTION_R(4, "Could not read player hit x");
                    numRead = client->read(&y,      4, 5); CHECK_CONNECTION_R(4, "Could not read player hit y");
                    numRead = client->read(&z,      4, 5); CHECK_CONNECTION_R(4, "Could not read player hit z");
                    numRead = client->read(&xDir,   4, 5); CHECK_CONNECTION_R(4, "Could not read player hit xd");
                    numRead = client->read(&yDir,   4, 5); CHECK_CONNECTION_R(4, "Could not read player hit yd");
                    numRead = client->read(&zDir,   4, 5); CHECK_CONNECTION_R(4, "Could not read player hit zd");
                    numRead = client->read(&weapon, 1, 5); CHECK_CONNECTION_R(1, "Could not read player hit weapon");

                    Vector3f velToAdd(xDir, yDir, zDir);
                    velToAdd.normalize();
                    if (velToAdd.y < 0 && Global::player->onGround)
                    {
                        velToAdd.y =  0.2f;
                    }

                    switch (weapon)
                    {
                        case WEAPON_FIST:
                        {
                            velToAdd.setLength(3.0f);
                            Global::player->vel = Global::player->vel + velToAdd;
                            AudioPlayer::play(75, nullptr);
                            break;
                        }

                        case WEAPON_BAT:
                        {
                            velToAdd.setLength(18.0f);
                            Global::player->vel = Global::player->vel + velToAdd;
                            AudioPlayer::play(74, nullptr);
                            break;
                        }

                        case WEAPON_GUN:
                        {
                            Global::player->health -= 10;
                            break;
                        }

                        default: break;
                    }

                    break;
                }

                case 5: //someone is sending us audio
                {
                    int sfxId;
                    Vector3f pos;

                    numRead = client->read(&sfxId, 4, 5); CHECK_CONNECTION_R(4, "Could not read audio id");
                    numRead = client->read(&pos.x, 4, 5); CHECK_CONNECTION_R(4, "Could not read audio x");
                    numRead = client->read(&pos.y, 4, 5); CHECK_CONNECTION_R(4, "Could not read audio y");
                    numRead = client->read(&pos.z, 4, 5); CHECK_CONNECTION_R(4, "Could not read audio z");

                    AudioPlayer::play(sfxId, &pos);
                    break;
                }

                case 6: //we have picked up a health pack
                {
                    int healthNameLen;
                    char healthName[33] = {0};
                    char healAmount;

                    numRead = client->read(&healthNameLen,         4, 5); CHECK_CONNECTION_R(4,             "Could not heal name len");
                    numRead = client->read(healthName, healthNameLen, 5); CHECK_CONNECTION_R(healthNameLen, "Could not heal name");
                    numRead = client->read(&healAmount,            1, 5); CHECK_CONNECTION_R(1,             "Could not heal amount");

                    if (healAmount > 0)
                    {
                        int newHealth = (int)Global::player->health;

                        newHealth = Maths::clamp(0, newHealth + (int)healAmount, 100);

                        Global::player->health = (char)newHealth;
                        AudioPlayer::play(61, nullptr);
                    }

                    std::string healthNameToDelete = healthName;

                    Global::gameEntitiesSharedMutex.lock_shared();
                    for (Entity* e : Global::gameEntities)
                    {
                        switch (e->getEntityType())
                        {
                            case ENTITY_HEALTH_CUBE:
                            {
                                if (e->name == healthNameToDelete)
                                {
                                    e->visible = false;
                                }
                                break;
                            }

                            default:
                                break;
                        }
                    }
                    Global::gameEntitiesSharedMutex.unlock_shared();

                    break;
                }

                case 7: // round time start
                {
                    numRead = client->read(&Global::timeUntilRoundStarts, 4, 5); CHECK_CONNECTION_R(4, "Could not read round time start");
                    break;
                }

                case 8: //set the status of a glass plane
                {
                    int glassNameLen;
                    char glassName[33] = {0};
                    char isReal;
                    char isBroken;

                    numRead = client->read(&glassNameLen,        4, 5); CHECK_CONNECTION_R(4,            "Could not read glass name len");
                    numRead = client->read(glassName, glassNameLen, 5); CHECK_CONNECTION_R(glassNameLen, "Could not read glass name");
                    numRead = client->read(&isReal,              1, 5); CHECK_CONNECTION_R(1,            "Could not read glass isReal");
                    numRead = client->read(&isBroken,            1, 5); CHECK_CONNECTION_R(1,            "Could not read glass isBroken");

                    std::string glassNameToUpdate = glassName;

                    Global::gameEntitiesSharedMutex.lock_shared();
                    for (Entity* e : Global::gameEntities)
                    {
                        switch (e->getEntityType())
                        {
                            case ENTITY_GLASS:
                            {
                                if (e->name == glassNameToUpdate)
                                {
                                    Glass* g = (Glass*)e;
                                    g->isReal = (bool)isReal;
                                    g->hasBroken = (bool)isBroken;
                                }
                                break;
                            }

                            default:
                                break;
                        }
                    }
                    Global::gameEntitiesSharedMutex.unlock_shared();

                    break;
                }

                case 9: //load a new level
                {
                    int levelNameLen;
                    char levelName[33] = {0};

                    numRead = client->read(&levelNameLen,        4, 5); CHECK_CONNECTION_R(4,            "Could not read level name len");
                    numRead = client->read(levelName, levelNameLen, 5); CHECK_CONNECTION_R(levelNameLen, "Could not read level name");

                    levelMutex.lock();
                    Global::levelToLoad = levelName;
                    levelMutex.unlock();

                    break;
                }

                case 10: // round end messages
                {
                    numRead = client->read(&Global::timeUntilRoundEnds, 4, 5); CHECK_CONNECTION_R(4, "Could not read round end time");
                    break;
                }

                case 11: //set the status of a rock platform
                {
                    int rockNameLen;
                    char rockName[33] = {0};
                    float rockBreakTimer;

                    numRead = client->read(&rockNameLen,       4, 5); CHECK_CONNECTION_R(4,           "Could not read rock platform name len");
                    numRead = client->read(rockName, rockNameLen, 5); CHECK_CONNECTION_R(rockNameLen, "Could not read rock platform name");
                    numRead = client->read(&rockBreakTimer,    4, 5); CHECK_CONNECTION_R(4,           "Could not read rock platform timer");

                    std::string rockNameToUpdate = rockName;

                    Global::gameEntitiesSharedMutex.lock_shared();
                    for (Entity* e : Global::gameEntities)
                    {
                        switch (e->getEntityType())
                        {
                            case ENTITY_ROCK_PLATFORM:
                            {
                                if (e->name == rockNameToUpdate)
                                {
                                    RockPlatform* r = (RockPlatform*)e;
                                    r->timeUntilBreaks = rockBreakTimer;
                                }
                                break;
                            }

                            default:
                                break;
                        }
                    }
                    Global::gameEntitiesSharedMutex.unlock_shared();

                    break;
                }

                case 12: // Set the position and vel of the player
                {
                    numRead = client->read(&Global::player->position.x, 4, 5); CHECK_CONNECTION_R(4, "Could not read set player x");
                    numRead = client->read(&Global::player->position.y, 4, 5); CHECK_CONNECTION_R(4, "Could not read set player y");
                    numRead = client->read(&Global::player->position.z, 4, 5); CHECK_CONNECTION_R(4, "Could not read set player z");
                    numRead = client->read(&Global::player->vel     .x, 4, 5); CHECK_CONNECTION_R(4, "Could not read set player vx");
                    numRead = client->read(&Global::player->vel     .y, 4, 5); CHECK_CONNECTION_R(4, "Could not read set player vy");
                    numRead = client->read(&Global::player->vel     .z, 4, 5); CHECK_CONNECTION_R(4, "Could not read set player vz");

                    break;
                }

                case 13: // We have been sent our ping to the server
                {
                    int p;
                    numRead = client->read(&p, 4, 5); CHECK_CONNECTION_R(4, "Could not read ping");
                    if (p < 0)
                    {
                        p = 0;
                    }
                    Global::pingToServer = p;
                    break;
                }

                case 14: // Signal that a step fall platform has been stepped on.
                {
                    int platNameLen;
                    char platName[33] = {0};

                    numRead = client->read(&platNameLen,       4, 5); CHECK_CONNECTION_R(4,           "Could not step fall platform name len");
                    numRead = client->read(platName, platNameLen, 5); CHECK_CONNECTION_R(platNameLen, "Could not step fall platform name");

                    std::string platNameToUpdate = platName;

                    Global::gameEntitiesSharedMutex.lock_shared();
                    for (Entity* e : Global::gameEntities)
                    {
                        switch (e->getEntityType())
                        {
                            case ENTITY_STEP_FALL_PLATFORM:
                            {
                                if (e->name == platNameToUpdate)
                                {
                                    StepFallPlatform* sf = (StepFallPlatform*)e;
                                    if (sf->timeUntilBreaks > 100.0f)
                                    {
                                        sf->timeUntilBreaks = 2.0f;
                                    }
                                }
                                break;
                            }

                            default:
                                break;
                        }
                    }
                    Global::gameEntitiesSharedMutex.unlock_shared();

                    break;
                }

                case 15: //we have picked up a bat
                {
                    int batNameLen;
                    char batName[33] = {0};
                    char batIsOurs;

                    numRead = client->read(&batNameLen,      4, 5); CHECK_CONNECTION_R(4,          "Could not heal name len");
                    numRead = client->read(batName, batNameLen, 5); CHECK_CONNECTION_R(batNameLen, "Could not heal name");
                    numRead = client->read(&batIsOurs,       1, 5); CHECK_CONNECTION_R(1,          "Could not heal amount");

                    if (batIsOurs > 0)
                    {
                        Global::player->weapon = WEAPON_BAT;
                        AudioPlayer::play(61, nullptr);
                    }

                    std::string batNameToDelete = batName;

                    Global::gameEntitiesSharedMutex.lock_shared();
                    for (Entity* e : Global::gameEntities)
                    {
                        switch (e->getEntityType())
                        {
                            case ENTITY_BAT:
                            {
                                if (e->name == batNameToDelete)
                                {
                                    e->visible = false;
                                }
                                break;
                            }

                            default:
                                break;
                        }
                    }
                    Global::gameEntitiesSharedMutex.unlock_shared();

                    break;
                }

                case 16: //set the status of a fall block
                {
                    int fblockNameLen;
                    char fblockName[33] = {0};
                    float fblockX;
                    float fblockZ;
                    float fblockPhaseTimer;

                    numRead = client->read(&fblockNameLen,         4, 5); CHECK_CONNECTION_R(4,             "Could not read fblock name len");
                    numRead = client->read(fblockName, fblockNameLen, 5); CHECK_CONNECTION_R(fblockNameLen, "Could not read fblock name");
                    numRead = client->read(&fblockX,               4, 5); CHECK_CONNECTION_R(4,             "Could not read fblock x");
                    numRead = client->read(&fblockZ,               4, 5); CHECK_CONNECTION_R(4,             "Could not read fblock z");
                    numRead = client->read(&fblockPhaseTimer,      4, 5); CHECK_CONNECTION_R(4,             "Could not read fblock phase timer");

                    std::string fblockNameToUpdate = fblockName;

                    Global::gameEntitiesSharedMutex.lock_shared();
                    for (Entity* e : Global::gameEntities)
                    {
                        switch (e->getEntityType())
                        {
                            case ENTITY_FALL_BLOCK:
                            {
                                if (e->name == fblockNameToUpdate)
                                {
                                    FallBlock* fblock = (FallBlock*)e;
                                    fblock->position.x = fblockX;
                                    fblock->position.z = fblockZ;
                                    fblock->phaseTimer = fblockPhaseTimer;
                                    fblock->phaseTimerOffset = fblockPhaseTimer;
                                }
                                break;
                            }

                            default:
                                break;
                        }
                    }
                    Global::gameEntitiesSharedMutex.unlock_shared();

                    break;
                }

                default:
                {
                    printf("Error: Received unknown command %d from server\n", cmd);
                    Global::addChatMessage("Disconnected from Server", Vector3f(1, 0.5f, 0.5f));
                    return;
                }
            }
        }
        else if (numRead == 0) //time out
        {

        }
        else //error, we have been disconnected
        {
            break;
        }
    }

    Global::addChatMessage("Disconnected from Server", Vector3f(1, 0.5f, 0.5f));
    printf("Read thread all done\n");
}

void Global::writeThreadBehavior(TcpClient* client)
{
    #define CHECK_CONNECTION_W(NUM_BYTES, MESSAGE) if (!checkConnection(NUM_BYTES, numWritten, MESSAGE, client)) { Global::addChatMessage("Disconnected from Server", Vector3f(1, 0.5f, 0.5f)); return; }

    // Send name message
    int nameLen = (int)Global::nickname.size();
    int numWritten = client->write(&nameLen, 4, 5); CHECK_CONNECTION_W(4, "Could not write name length to server");

    numWritten = client->write(Global::nickname.c_str(), nameLen, 5); CHECK_CONNECTION_W(nameLen, "Could not write name to server");

    double lastSentTimeMsg = glfwGetTime();
    double lastSentPlayerMsg = glfwGetTime();

    const double PLAYER_UPDATE_INTERVAL = 0.01;

    while (Global::gameState != GAME_STATE_EXITING && client->isOpen())
    {
        std::unique_lock<std::mutex> lock{Global::msgOutMutex};
        Global::msgCondVar.wait_for(lock, std::chrono::milliseconds(1), [&]() // This does lead to strange delays in multiples of 16 ms. But getting rid of it isnt much better.
        {
            // If any of these conditions are true, aquire the lock
            return (Global::messagesToSend.size() > 0                          ||
                    glfwGetTime() - lastSentPlayerMsg > PLAYER_UPDATE_INTERVAL ||
                    Global::gameState == GAME_STATE_EXITING                    ||
                    !client->isOpen());
        });

        if (Global::messagesToSend.size() > 0)
        {
            for (Message msg : Global::messagesToSend)
            {
                numWritten = client->write(msg.buf, msg.length, 5); CHECK_CONNECTION_W(msg.length, "Could not write command " + std::to_string(msg.buf[0]) + " to the server");
            }
            Global::messagesToSend.clear();
        }

        lock.unlock();

        if (glfwGetTime() - lastSentTimeMsg > 1.0)
        {
            // Send time message
            char cmd = 1;
            numWritten = client->write(&cmd, 1, 5); CHECK_CONNECTION_W(1, "Could not write time command to server");

            double currTime = glfwGetTime() + Global::serverTimeOffset;
            numWritten = client->write(&currTime, 8, 5); CHECK_CONNECTION_W(8, "Could not write time to server");

            lastSentTimeMsg = glfwGetTime();
        }

        if (glfwGetTime() - lastSentPlayerMsg > PLAYER_UPDATE_INTERVAL)
        {
            //Send main player message
            if (Global::player != nullptr)
            {
                char buf[220];
                buf[0] = 2;

                int idx = 1;

                double currServerSyncedTime = glfwGetTime() + Global::serverTimeOffset;

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
                memcpy(&buf[idx], (char*)&Global::player->health,             1); idx += 1;
                memcpy(&buf[idx], (char*)&Global::player->inZoneTime,         4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->isOnRope,           1); idx += 1;
                memcpy(&buf[idx], (char*)&Global::player->ropeAnchor.x,       4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->ropeAnchor.y,       4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->ropeAnchor.z,       4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->ropeLength,         4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->animTimerStand,     4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->animTimerWalk,      4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->animTimerRun,       4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->animTimerCrouch,    4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->animTimerSlide,     4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->animTimerJump,      4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->animTimerFall,      4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->animTimerClimb,     4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->animTimerSwing,     4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->animTimerCrawl,     4); idx += 4;
                memcpy(&buf[idx], (char*)&Global::player->animType,           1); idx += 1;
                memcpy(&buf[idx], (char*)&Global::player->animTypePrevious,   1); idx += 1;
                memcpy(&buf[idx], (char*)&Global::player->animBlend,          4); idx += 4;
                memcpy(&buf[idx], (char*)&currServerSyncedTime,               8); idx += 8;

                numWritten = client->write(buf, 220, 5); CHECK_CONNECTION_W(220, "Could not write player update");
            }

            lastSentPlayerMsg = glfwGetTime();
        }
    }

    //Send a disconnect message
    Message msg;
    msg.length = 1;
    msg.buf[0] = 15;
    numWritten = client->write(msg.buf, 1, 2); CHECK_CONNECTION_W(1, "Could not write disconnect message");

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

        GUIText* tex = new GUIText(msg, 0.02f, Global::fontConsolas, 0.01f, 0.99f-0.025f, 6, true); INCR_NEW("GUIText");
        tex->color = chatColorsToAdd[i];

        for (int c = 0; c < (int)chatTexts.size(); c++)
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

    for (int i = 0; i < (int)chatTexts.size(); i++)
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
                localChatText = new GUIText(Input::chatInput, 0.02f, Global::fontConsolas, 0.01f, 0.99f, 6, true); INCR_NEW("GUIText");
            }
        }
    }
}

std::vector<Message> Global::messagesToSend;
std::mutex Global::msgOutMutex;
std::condition_variable Global::msgCondVar;

void Global::sendMessageToServer(Message msg)
{
    if (Global::serverClient->isOpen())
    {
        Global::msgOutMutex.lock();
        Global::messagesToSend.push_back(msg);
        Global::msgOutMutex.unlock();
    }
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

float Global::timeUntilRoundStarts = 0.0f;
GUIText* Global::timeUntilRoundStartsText = nullptr;
float Global::timeUntilRoundEnds = 100.0f;
GUIText* Global::timeUntilRoundEndText = nullptr;

Vector3f Global::safeZoneStart;
Vector3f Global::safeZoneEnd;

void Global::updateMusic()
{
    // Update the music when time gets low
    Source* bgmSource = AudioPlayer::getSource(14);

    if (Global::levelId == LVL_MAP2) //No BGM until last 15 seconds
    {
        if (Global::timeUntilRoundEnds > 30.0f)
        {
            if (bgmSource->isPlaying())
            {
                AudioPlayer::stopBGM();
                bgmSource->setPitch(1.0f);
            }
        }
        else if (Global::timeUntilRoundEnds > 15.0f)
        {
            if (!bgmSource->isPlaying())
            {
                AudioPlayer::playBGM(0);
                bgmSource->setPitch(1.0f);
            }
        }
        else if (Global::timeUntilRoundEnds > 0.0f)
        {
            if (!bgmSource->isPlaying())
            {
                AudioPlayer::playBGM(0);
            }
            bgmSource->setPitch(1.0f + (15.0f - Global::timeUntilRoundEnds)/15.0f);
        }
        else
        {
            if (bgmSource->isPlaying())
            {
                AudioPlayer::stopBGM();
                bgmSource->setPitch(1.0f);
                AudioPlayer::play(67, nullptr);
            }
        }
    }
    else if (Global::levelId == LVL_MAP4 || Global::levelId == LVL_MAP6)
    {
        if (Global::timeUntilRoundStarts > 0.0f)
        {
            if (bgmSource->isPlaying())
            {
                AudioPlayer::stopBGM();
                bgmSource->setPitch(1.0f);
            }
        }
        else if (Global::timeUntilRoundEnds > 15.0f)
        {
            if (!bgmSource->isPlaying())
            {
                AudioPlayer::playBGM(1);
                bgmSource->setPitch(1.0f);
            }
        }
        else if (Global::timeUntilRoundEnds > 0.0f)
        {
            if (!bgmSource->isPlaying())
            {
                AudioPlayer::playBGM(1);
            }
            bgmSource->setPitch(1.0f + (15.0f - Global::timeUntilRoundEnds)/15.0f);
        }
        else
        {
            if (bgmSource->isPlaying())
            {
                AudioPlayer::stopBGM();
                bgmSource->setPitch(1.0f);
                AudioPlayer::play(67, nullptr);
            }
        }
    }
    else if (Global::levelId == LVL_MAP5 || Global::levelId == LVL_MAP7)
    {
        if (Global::timeUntilRoundStarts > 0.0f)
        {
            if (bgmSource->isPlaying())
            {
                AudioPlayer::stopBGM();
                bgmSource->setPitch(1.0f);
            }
        }
        else if (Global::timeUntilRoundEnds > 0.0f)
        {
            if (!bgmSource->isPlaying())
            {
                AudioPlayer::playBGM(2);
                bgmSource->setPitch(1.0f);
            }
        }
        else
        {
            if (bgmSource->isPlaying())
            {
                AudioPlayer::stopBGM();
                bgmSource->setPitch(1.0f);
                AudioPlayer::play(67, nullptr);
            }
        }
    }
}
