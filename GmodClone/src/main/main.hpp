#ifndef MAIN_H
#define MAIN_H

class Entity;
class Camera;
class Light;
class Fbo;
class Player;
class OnlinePlayer;
class FontType;
class TcpClient;
class Dummy;
class TexturedModel;
class GUIText;

#include <string>
#include <random>
#include <list>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>

#include "../toolbox/vector.hpp"

#define GAME_STATE_PAUSED 0
#define GAME_STATE_RUNNING 1
#define GAME_STATE_EXITING 2
#define GAME_STATE_CUTSCENE 3
#define GAME_STATE_TITLE 4
#define GAME_STATE_DEBUG 5

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

class Message
{
public:
    int length = 0;
    char buf[188] = {0};
    Message(const Message &other);
    Message();
};

#define LVL_HUB  0
#define LVL_MAP1 1
#define LVL_MAP2 2

class Global
{
public:
    static std::string pathToEXE;
    // Time in seconds that is synced between us and the server and the other players.
    static double syncedGlobalTime;

    static std::shared_mutex gameEntitiesSharedMutex;
    static std::unordered_set<Entity*> gameEntities;

    static int gameState;

    static int levelId;
    static std::string levelToLoad;

    static Camera* gameCamera;
    static Player* player;

    static std::list<TexturedModel*> stageModels;
    static Dummy* stageEntity;

    static Light* gameLightSun;
    static Vector3f skyColor;

    static FontType* fontConsolas;
    static int countNew;
    static int countDelete;

    static int gameTotalPlaytime; //in seconds
    static bool renderWithCulling; //if the stage uses backface culling or not
    static bool displayFPS;
    static int currentCalculatedFPS;
    static int renderCount;
    static float fpsLimit; //experiment to try to busy wait to maintain a target fps
    static bool useFullscreen;

    static bool renderParticles;

    static bool framerateUnlock;

    //Entities passed here should be made on the heap, as they will be explicitly deleted later
    static void addEntity(Entity* entityToAdd);
    static void deleteEntity(Entity* entityToDelete);
    static void deleteAllEntites();

    static float calcAspectRatio();

    static void debugNew(const char* name);

    static void debugDel(const char* name);

    static void performanceAnalysisStart(const char* name);

    static void performanceAnalysisDone(const char* name);

    static void performanceAnalysisReport();

    static std::string nickname;
    static std::shared_mutex gameOnlinePlayersSharedMutex;
    static std::unordered_map<std::string, OnlinePlayer*> gameOnlinePlayers;
    static std::vector<std::string> serverSettings;
    static TcpClient* serverClient;

    static void addChatMessage(std::string msg, Vector3f color);

    static void updateChatMessages();

    static unsigned long long serverStartTime; //when the server started up

    static unsigned long long getRawUtcSystemTime();

    static void readThreadBehavoir(TcpClient* client);

    static void writeThreadBehavior(TcpClient* client);

    static std::vector<Message> messagesToSend;
    static std::mutex msgOutMutex;
    static std::condition_variable msgCondVar;
    static void sendMessageToServer(Message msg);

    static void sendAudioMessageToServer(int sfxId, Vector3f* position);

    static float timeUntilRoundStarts;
    static GUIText* timeUntilRoundStartsText;
};
#endif
