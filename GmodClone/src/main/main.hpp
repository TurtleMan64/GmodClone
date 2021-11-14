#ifndef MAIN_H
#define MAIN_H

class Entity;
class Camera;
class Light;
class Fbo;
class Player;
class OnlinePlayer;
class FontType;

#include <string>
#include <random>
#include <list>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "../toolbox/vector.hpp"

#define STATE_PAUSED 0
#define STATE_RUNNING 1
#define STATE_EXITING 2
#define STATE_CUTSCENE 3
#define STATE_TITLE 4
#define STATE_DEBUG 5

#define DEV_MODE
//#define PERFORMANCE_ANALYSIS

#ifdef DEV_MODE
    #include <thread>
    #define INCR_NEW(NAME) Global::debugNew(NAME);
    #define INCR_DEL(NAME) Global::debugDel(NAME);
#else
    #define INCR_NEW(NAME) ;
    #define INCR_DEL(NAME) ;
#endif

#ifdef PERFORMANCE_ANALYSIS
    #define ANALYSIS_START(NAME) Global::performanceAnalysisStart(NAME);
    #define ANALYSIS_DONE(NAME)  Global::performanceAnalysisDone(NAME);
    #define ANALYSIS_REPORT()    Global::performanceAnalysisReport();
#else
    #define ANALYSIS_START(NAME) ;
    #define ANALYSIS_DONE(NAME)  ;
    #define ANALYSIS_REPORT()    ;
#endif


class Global
{
public:
    static std::string pathToEXE;
    static std::string nickname;
    // Time in seconds that is synced between us and the server and the other players.
    static double syncedGlobalTime;
    static std::unordered_set<Entity*> gameEntities;
    static std::unordered_map<std::string, OnlinePlayer*> gameOnlinePlayers;
    static std::vector<std::string> serverSettings;
    //static MenuManager menuManager;
    //static Timer* mainHudTimer;
    static Camera* gameCamera;
    static Player* player;
    //static TC_Kart* gameKart;
    static Entity* gameStageManager;
    //static Stage* gameStage;
    //static SkySphere* gameSkySphere;
    static Light* gameLightSun;
    static Light* gameLightMoon;
    static FontType* gameFont;
    static int countNew;
    static int countDelete;
    static int gameState;
    static int levelId;
    static std::string levelName;
    static std::string levelNameDisplay;
    static bool shouldLoadLevel;
    static bool isNewLevel;
    static bool isAutoCam;
    static int gameRingCount;
    static int gameScore;
    static int gameLives;
    static float gameClock;
    //static WaterRenderer* gameWaterRenderer;
    //static WaterFrameBuffers* gameWaterFBOs;
    //static std::vector<WaterTile*> gameWaterTiles;
    static int gameMainVehicleSpeed;
    static float startStageTimer;
    static float finishStageTimer;
    static Fbo* gameMultisampleFbo;
    static Fbo* gameOutputFbo;
    static Fbo* gameOutputFbo2;
    static float deathHeight;
    static float waterHeight;
    static int gameMissionNumber;
    static bool gameIsNormalMode;
    static bool gameIsHardMode;
    static bool gameIsChaoMode;
    static bool gameIsRingMode;
    static bool gameIsRaceMode;
    static float gameRaceTimeLimit;
    static std::string gameMissionDescription;
    static int gameRingTarget; //How many rings you have to collect to finish the stage in Ring mode
    static bool gameIsArcadeMode;
    static int gameTotalPlaytime; //in seconds
    static float gameArcadePlaytime;
    static bool stageUsesWater;
    static Vector3f stageWaterColor;
    static float stageWaterBlendAmount;
    static float stageWaterMurkyAmount;
    //static FontType* fontVipnagorgialla;
    static bool renderWithCulling; //if the stage uses backface culling or not
    static bool displayFPS;
    static int currentCalculatedFPS;
    static int renderCount;
    //static const int syncToDisplayEveryXFrames = 1; //if for some reason you want to render at double, triple, etc. the fps of the monitor
    //static float fpsTarget; //experiment to try to sleep to maintain a target fps
    static float fpsLimit; //experiment to try to busy wait to maintain a target fps
    static bool useFullscreen;
    static int displaySizeChanged; //This will be 1 for a single frame after the size of the window changes (set in callback)

    static std::unordered_map<std::string, std::string> gameSaveData;

    static bool renderParticles;

    static bool framerateUnlock;

    //how many npcs are in each level
    static std::unordered_map<int, int> stageNpcCounts;

    static void loadSaveData();

    static void saveSaveData();

    static void saveConfigData();

    //Entities passed here should be made on the heap, as they will be explicitly deleted later
    static void addEntity(Entity* entityToAdd);
    static void deleteEntity(Entity* entityToDelete);
    static void deleteAllEntites();

    static float calcAspectRatio();

    static void createTitleCard();

    static void clearTitleCard();

    static void debugNew(const char* name);

    static void debugDel(const char* name);

    static void performanceAnalysisStart(const char* name);

    static void performanceAnalysisDone(const char* name);

    static void performanceAnalysisReport();

    static void addChatMessage(std::string msg, Vector3f color);

    static void updateChatMessages();
};
#endif
