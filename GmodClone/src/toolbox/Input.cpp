#include <glad/glad.h>

#ifdef _WIN32
#include <GLFW/glfw3.h>
#include <Windows.h>
#else
#include "/usr/include/GLFW/glfw3.h"
#endif

#include <iostream>
#include <cmath>
#include <fstream>
#include <string>
#include <cstring>
#include <algorithm>

#include "input.hpp"
#include "../main/main.hpp"
#include "../entities/camera.hpp"
#include "maths.hpp"
#include "../toolbox/split.hpp"
#include "../toolbox/getline.hpp"
#include <random>
#include <chrono>
#include "../renderEngine/renderEngine.hpp"
#include "../entities/player.hpp"
#include "split.hpp"

#ifdef DEV_MODE
#include <iostream>
#include <fstream>
#endif

InputStruct Input::inputs{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//vars for use by us
double mousePreviousX = 0;
double mousePreviousY = 0;

double mouseScroll = 0.0;

//settings

bool freeMouse = true;

float mouseSensitivityX = 0.25f;
float mouseSensitivityY = 0.25f;

float stickSensitivityX = 2.5f;
float stickSensitivityY = 2.5f;

extern GLFWwindow* window;

void Input::pollInputs()
{
    glfwPollEvents();

    Input::inputs.INPUT_PREVIOUS_ACTION1     = Input::inputs.INPUT_ACTION1;
    Input::inputs.INPUT_PREVIOUS_ACTION2     = Input::inputs.INPUT_ACTION2;
    Input::inputs.INPUT_PREVIOUS_ACTION3     = Input::inputs.INPUT_ACTION3;
    Input::inputs.INPUT_PREVIOUS_ACTION4     = Input::inputs.INPUT_ACTION4;
    Input::inputs.INPUT_PREVIOUS_LEFT_CLICK  = Input::inputs.INPUT_LEFT_CLICK;
    Input::inputs.INPUT_PREVIOUS_RIGHT_CLICK = Input::inputs.INPUT_RIGHT_CLICK;
    Input::inputs.INPUT_PREVIOUS_LB          = Input::inputs.INPUT_LB;
    Input::inputs.INPUT_PREVIOUS_RB          = Input::inputs.INPUT_RB;
    Input::inputs.INPUT_PREVIOUS_START       = Input::inputs.INPUT_START;
    Input::inputs.INPUT_PREVIOUS_TAB         = Input::inputs.INPUT_TAB;

    Input::inputs.INPUT_PREVIOUS_X  = Input::inputs.INPUT_X;
    Input::inputs.INPUT_PREVIOUS_Y  = Input::inputs.INPUT_Y;
    Input::inputs.INPUT_PREVIOUS_X2 = Input::inputs.INPUT_X2;
    Input::inputs.INPUT_PREVIOUS_Y2 = Input::inputs.INPUT_Y2;
    Input::inputs.INPUT_PREVIOUS_R2 = Input::inputs.INPUT_L2;
    Input::inputs.INPUT_PREVIOUS_L2 = Input::inputs.INPUT_R2;


    Input::inputs.INPUT_ACTION1     = false;
    Input::inputs.INPUT_ACTION2     = false;
    Input::inputs.INPUT_ACTION3     = false;
    Input::inputs.INPUT_ACTION4     = false;
    Input::inputs.INPUT_LEFT_CLICK  = false;
    Input::inputs.INPUT_RIGHT_CLICK = false;
    Input::inputs.INPUT_LB          = false;
    Input::inputs.INPUT_RB          = false;
    Input::inputs.INPUT_SELECT      = false;
    Input::inputs.INPUT_START       = false;
    Input::inputs.INPUT_TAB         = false;

    Input::inputs.INPUT_X  = 0;
    Input::inputs.INPUT_Y  = 0;
    Input::inputs.INPUT_X2 = 0;
    Input::inputs.INPUT_Y2 = 0;
    Input::inputs.INPUT_L2 = 0;
    Input::inputs.INPUT_R2 = 0;

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    if (freeMouse == false)
    {
        float mouseDiffX = (float)(mouseSensitivityX*(xpos - mousePreviousX))*0.004f;
        float mouseDiffY = (float)(mouseSensitivityY*(ypos - mousePreviousY))*0.004f;

        Input::inputs.INPUT_X2 += mouseDiffX;
        Input::inputs.INPUT_Y2 += mouseDiffY;
    }
    mousePreviousX = xpos;
    mousePreviousY = ypos;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        Input::inputs.INPUT_LEFT_CLICK = true;
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        Input::inputs.INPUT_RIGHT_CLICK = true;
    }

    if (mouseScroll > 0.0)
    {
        Input::inputs.INPUT_SCROLL = 1;
    }
    else if (mouseScroll < 0.0)
    {
        Input::inputs.INPUT_SCROLL = -1;
    }
    else
    {
        Input::inputs.INPUT_SCROLL = 0;
    }

    mouseScroll = 0;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        Input::inputs.INPUT_ACTION1 = true;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        Input::inputs.INPUT_ACTION2 = true;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        Input::inputs.INPUT_ACTION3 = true;
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    {
        Input::inputs.INPUT_ACTION4 = true;
    }
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
    {
        Input::inputs.INPUT_START = true;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        Input::inputs.INPUT_LB = true;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        Input::inputs.INPUT_RB = true;
        extern float dt;
        Global::player->vel.y += 60*dt;
    }
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS)
    {
        Input::inputs.INPUT_TAB = true;
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        Input::inputs.INPUT_Y = -1;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        Input::inputs.INPUT_Y = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        Input::inputs.INPUT_X = -1;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        Input::inputs.INPUT_X = 1;
    }

    if ((GetKeyState(VK_CAPITAL) & 1) != 0)
    {
        Input::inputs.INPUT_ACTION3 = true;
    }

    /*
    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
    {
        SkyManager::increaseTimeOfDay(0.5f);
    }
    if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS)
    {
        SkyManager::increaseTimeOfDay(-0.5f);
    }

    ifdef DEV_MODE
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
    {
        if (Global::gameMainPlayer != nullptr)
        {
            extern float dt;
            //Global::gameMainVehicle->getPosition()->y = Global::gameMainVehicle->getY()+600*dt;
            Vector3f* v = &Global::gameMainPlayer->vel;
            Global::gameMainPlayer->vel.set(v->x, v->y+600*dt, v->z);
        }
    }
    endif
    */
    #ifdef DEV_MODE
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        printf("%f %f %f\n", Global::player->position.x, Global::player->position.y, Global::player->position.z);
    }
    #endif

    #ifdef DEV_MODE
    if (Input::inputs.INPUT_LB && !Input::inputs.INPUT_PREVIOUS_LB)
    {
        /*
        if (Global::gameMainPlayer != nullptr)
        {
            std::fprintf(stdout, "Time of day: %f\n", SkyManager::getTimeOfDay());
            std::fprintf(stdout, "position = [%f, %f, %f]\n", Global::gameMainPlayer->position.x, Global::gameMainPlayer->position.y, Global::gameMainPlayer->position.z);
            std::fprintf(stdout, "velocity = [%f, %f, %f]\n", Global::gameMainPlayer->vel.x, Global::gameMainPlayer->vel.y, Global::gameMainPlayer->vel.z);
            std::fprintf(stdout, "normal   = [%f, %f, %f]\n", Global::gameMainPlayer->relativeUp.x, Global::gameMainPlayer->relativeUp.y, Global::gameMainPlayer->relativeUp.z);
            //std::fprintf(stdout, "player rot = %f\n", Global::gamePlayer->getRotY());
            //std::fprintf(stdout, "cam yaw: %f,   cam pitch: %f\n", Global::gameCamera->getYaw(), Global::gameCamera->getPitch());
            std::fprintf(stdout, "cam pos = [%f, %f, %f]\n", Global::gameCamera->eye.x, Global::gameCamera->eye.y, Global::gameCamera->eye.z);
            std::fprintf(stdout, "cam dir = [%f, %f, %f]\n", Global::gameMainPlayer->camDir.x, Global::gameMainPlayer->camDir.y, Global::gameMainPlayer->camDir.z);
            std::fprintf(stdout, "\n");

            //std::fprintf(stdout, "106 1  %f %f %f  %f %f %f   %f %f %f\n", 
            //    Global::gameKart->position.x, Global::gameKart->position.y, Global::gameKart->position.z,
            //    Global::gameKart->vel.x, Global::gameKart->vel.y, Global::gameKart->vel.z, 
            //    Global::gameKart->currNorm.x, Global::gameKart->currNorm.y, Global::gameKart->currNorm.z);
        }
        */

        //std::fprintf(stdout, "diff = %d\n", Global::countNew-Global::countDelete);
        //extern std::unordered_map<std::string, int> heapObjects;
        //std::unordered_map<std::string, int>::iterator it = heapObjects.begin();
        //while (it != heapObjects.end())
        {
            //std::fprintf(stdout, "'%s' count: %d\n", it->first.c_str(), it->second);
            //it++;
        }

        //Loader::printInfo();

        //if (Global::raceLogSize > 0)
        {
            //std::ofstream raceLogFile;
            //raceLogFile.open("RaceLog.txt", std::ios::out | std::ios::trunc);
            //for (int i = 0; i < Global::raceLogSize; i++)
            //{
            //    std::string line = Global::raceLog[i].toString();
            //    raceLogFile << line << "\n";
            //}
            //raceLogFile.close();
            //Global::raceLogSize = 0;
        }
    }
    if (Input::inputs.INPUT_RB && !Input::inputs.INPUT_PREVIOUS_RB)
    {
        //Global::shouldLogRace = !Global::shouldLogRace;
    }
    #endif


    if (Input::inputs.INPUT_TAB && !Input::inputs.INPUT_PREVIOUS_TAB)
    {
        if (freeMouse)
        {
            freeMouse = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else
        {
            freeMouse = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }


    float mag = sqrtf(Input::inputs.INPUT_X*Input::inputs.INPUT_X + Input::inputs.INPUT_Y*Input::inputs.INPUT_Y);
    if (mag > 1)
    {
        Input::inputs.INPUT_X = Input::inputs.INPUT_X / mag;
        Input::inputs.INPUT_Y = Input::inputs.INPUT_Y / mag;
    }

    Input::inputs.approxXLeftPrevious = Input::inputs.approxXLeft;
    Input::inputs.approxXLeft = (int)round(Input::inputs.INPUT_X);
    Input::inputs.approxYLeftPrevious = Input::inputs.approxYLeft;
    Input::inputs.approxYLeft = (int)round(Input::inputs.INPUT_Y);

    if (Input::inputs.approxXLeft != 0)
    {
        Input::inputs.MENU_X = Input::inputs.approxXLeft - Input::inputs.approxXLeftPrevious;
    }

    if (Input::inputs.approxYLeft != 0)
    {
        Input::inputs.MENU_Y = Input::inputs.approxYLeft - Input::inputs.approxYLeftPrevious;
    }

    if (Input::isTypingInChat)
    {
        Input::inputs.INPUT_ACTION1     = false;
        Input::inputs.INPUT_ACTION2     = false;
        Input::inputs.INPUT_ACTION3     = false;
        Input::inputs.INPUT_ACTION4     = false;
        Input::inputs.INPUT_LEFT_CLICK  = false;
        Input::inputs.INPUT_RIGHT_CLICK = false;
        Input::inputs.INPUT_LB          = false;
        Input::inputs.INPUT_RB          = false;
        Input::inputs.INPUT_SELECT      = false;
        Input::inputs.INPUT_START       = false;
        Input::inputs.INPUT_TAB         = false;

        Input::inputs.INPUT_X  = 0;
        Input::inputs.INPUT_Y  = 0;
        Input::inputs.INPUT_X2 = 0;
        Input::inputs.INPUT_Y2 = 0;
        Input::inputs.INPUT_L2 = 0;
        Input::inputs.INPUT_R2 = 0;
    }

    if (Global::timeUntilRoundStarts > 0.0f)
    {
        Input::inputs.INPUT_ACTION1     = false;
        Input::inputs.INPUT_ACTION2     = false;
        Input::inputs.INPUT_ACTION3     = false;
        Input::inputs.INPUT_ACTION4     = false;
        Input::inputs.INPUT_LEFT_CLICK  = false;
        Input::inputs.INPUT_RIGHT_CLICK = false;
        Input::inputs.INPUT_LB          = false;
        Input::inputs.INPUT_RB          = false;
        Input::inputs.INPUT_SELECT      = false;
        Input::inputs.INPUT_START       = false;

        Input::inputs.INPUT_X  = 0;
        Input::inputs.INPUT_Y  = 0;
        Input::inputs.INPUT_L2 = 0;
        Input::inputs.INPUT_R2 = 0;
    }
}

void Input::init()
{
    Input::inputs.uniqueVar = 1149650285; //Value that is very easy to find with a memory scan

    glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
    glfwSetInputMode(window, GLFW_LOCK_KEY_MODS, GLFW_TRUE);

    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }

    Input::chatInput = new char[100]; INCR_NEW("CHAR ARRAY");
    for (int i = 0; i < 100; i++)
    {
        Input::chatInput[i] = 0;
    }

    glfwSetScrollCallback(window, Input::scrollCallback);
    glfwSetKeyCallback(window, Input::keyboardCallback);

    //load sensitivity and button mappings from external file

    std::ifstream file(Global::pathToEXE + "Settings/CameraSensitivity.ini");
    if (!file.is_open())
    {
        std::fprintf(stdout, "Error: Cannot load file '%s'\n", (Global::pathToEXE + "Settings/CameraSensitivity.ini").c_str());
        file.close();
    }
    else
    {
        std::string line;

        while (!file.eof())
        {
            getlineSafe(file, line);

            char lineBuf[512];
            memcpy(lineBuf, line.c_str(), line.size()+1);

            int splitLength = 0;
            char** lineSplit = split(lineBuf, ' ', &splitLength);

            if (splitLength == 2)
            {
                if (strcmp(lineSplit[0], "Mouse_X") == 0)
                {
                    mouseSensitivityX = std::stof(lineSplit[1], nullptr);
                }
                else if (strcmp(lineSplit[0], "Mouse_Y") == 0)
                {
                    mouseSensitivityY = std::stof(lineSplit[1], nullptr);
                }
            }

            free(lineSplit);
        }
        file.close();
    }

    glfwPollEvents();
}

void Input::scrollCallback(GLFWwindow* /*w*/, double /*xOff*/, double yOff)
{
    mouseScroll = yOff;
}

char* Input::chatInput = nullptr;
int Input::chatLength = 0;
bool Input::isTypingInChat = false;
bool Input::localChatHasBeenUpdated = false;

void Input::keyboardCallback(GLFWwindow* /**/, int key, int /**/, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        switch (key)
        {
            case GLFW_KEY_GRAVE_ACCENT:
            {
                Input::isTypingInChat = !Input::isTypingInChat;
                Input::localChatHasBeenUpdated = true;
                break;
            }

            case GLFW_KEY_ENTER:
            {
                if (Input::isTypingInChat && Input::chatLength > 0)
                {
                    try
                    {
                        if (Input::chatLength >= 5 && strncmp("fov ", Input::chatInput, 4) == 0 ||
                            Input::chatLength >= 5 && strncmp("FOV ", Input::chatInput, 4) == 0)
                        {
                            std::vector<std::string> tokens = split(Input::chatInput, ' ');
                            if (tokens.size() > 1)
                            {
                                extern float VFOV_BASE;
                                VFOV_BASE = Maths::clamp(30.0f, std::stof(tokens[1]), 120.0f);
                                Master_makeProjectionMatrix();
                            }
                        }
                        else if (Input::chatLength >= 9 && strncmp("fps_cap ", Input::chatInput, 8) == 0 ||
                                 Input::chatLength >= 9 && strncmp("FPS_CAP ", Input::chatInput, 8) == 0 ||
                                 Input::chatLength >= 9 && strncmp("fps-cap ", Input::chatInput, 8) == 0 ||
                                 Input::chatLength >= 9 && strncmp("FPS-CAP ", Input::chatInput, 8) == 0)
                        {
                            std::vector<std::string> tokens = split(Input::chatInput, ' ');
                            if (tokens.size() > 1)
                            {
                                float newFps = std::stof(tokens[1]);
                                if (newFps <= 0.0f)
                                {
                                    Global::fpsLimit = -1.0f;
                                }
                                else
                                {
                                    Global::fpsLimit = Maths::clamp(30.0f, newFps, 99999999999.0f);
                                }
                            }
                        }
                        else if (Input::chatLength >= 7 && strncmp("vsync ", Input::chatInput, 6) == 0 ||
                                 Input::chatLength >= 7 && strncmp("VSYNC ", Input::chatInput, 6) == 0)
                        {
                            std::vector<std::string> tokens = split(Input::chatInput, ' ');
                            if (tokens.size() > 1)
                            {
                                float fpsUnlock = std::stof(tokens[1]);
                                if (fpsUnlock > 0.5f)
                                {
                                    Global::framerateUnlock = false;
                                }
                                else
                                {
                                    Global::framerateUnlock = true;
                                }
                            }
                        }
                        else if (Input::chatLength >= 10 && strncmp("fps-show ", Input::chatInput, 9) == 0 ||
                                 Input::chatLength >= 10 && strncmp("FPS-SHOW ", Input::chatInput, 9) == 0 ||
                                 Input::chatLength >= 10 && strncmp("fps_show ", Input::chatInput, 9) == 0 ||
                                 Input::chatLength >= 10 && strncmp("FPS_SHOW ", Input::chatInput, 9) == 0)
                        {
                            std::vector<std::string> tokens = split(Input::chatInput, ' ');
                            if (tokens.size() > 1)
                            {
                                float fpsShow = std::stof(tokens[1]);
                                if (fpsShow > 0.5f)
                                {
                                    Global::displayFPS = true;
                                }
                                else
                                {
                                    Global::displayFPS = false;
                                }
                            }
                        }
                        else if (Input::chatLength >= 10 && strncmp("load-map ", Input::chatInput, 9) == 0 ||
                                 Input::chatLength >= 10 && strncmp("LOAD-MAP ", Input::chatInput, 9) == 0 ||
                                 Input::chatLength >= 10 && strncmp("load_map ", Input::chatInput, 9) == 0 ||
                                 Input::chatLength >= 10 && strncmp("LOAD_MAP ", Input::chatInput, 9) == 0)
                        {
                            std::vector<std::string> tokens = split(Input::chatInput, ' ');
                            if (tokens.size() > 1)
                            {
                                Global::levelToLoad = tokens[1];
                            }
                        }
                    }
                    catch (std::exception e)
                    {
                        
                    }

                    Global::addChatMessage(Input::chatInput, Vector3f(1, 1, 1));
                    Input::chatLength = 0;
                    Input::chatInput[0] = 0;
                    Input::localChatHasBeenUpdated = true;
                    Input::isTypingInChat = false;
                }
                break;
            }

            case GLFW_KEY_BACKSPACE:
            {
                if (Input::isTypingInChat)
                {
                    if (Input::chatLength > 0)
                    {
                        Input::chatInput[Input::chatLength] = 0;
                        Input::chatInput[Input::chatLength-1] = 0;
                        Input::chatLength--;
                        Input::localChatHasBeenUpdated = true;
                    }
                }
                break;
            }

            default:
            {
                if (Input::isTypingInChat && key >= 32 && key <= 126)
                {
                    if (Input::chatLength < 44)
                    {
                        char ascii = (char)key;
                        bool shift = mods & GLFW_MOD_SHIFT;
                        bool caps = mods & GLFW_MOD_CAPS_LOCK;

                        if (key >= 65 && key <= 90)
                        {
                            if (shift == caps)
                            {
                                ascii += 32;
                            }
                        }
                        else if (shift)
                        {
                            switch (key)
                            {
                                case GLFW_KEY_1:             ascii = '!'; break;
                                case GLFW_KEY_2:             ascii = '@'; break;
                                case GLFW_KEY_3:             ascii = '#'; break;
                                case GLFW_KEY_4:             ascii = '$'; break;
                                case GLFW_KEY_5:             ascii = '%'; break;
                                case GLFW_KEY_6:             ascii = '^'; break;
                                case GLFW_KEY_7:             ascii = '&'; break;
                                case GLFW_KEY_8:             ascii = '*'; break;
                                case GLFW_KEY_9:             ascii = '('; break;
                                case GLFW_KEY_0:             ascii = ')'; break;
                                case GLFW_KEY_MINUS:         ascii = '_'; break;
                                case GLFW_KEY_EQUAL:         ascii = '+'; break;
                                case GLFW_KEY_LEFT_BRACKET:  ascii = '{'; break;
                                case GLFW_KEY_RIGHT_BRACKET: ascii = '}'; break;
                                case GLFW_KEY_BACKSLASH:     ascii = '|'; break;
                                case GLFW_KEY_SEMICOLON:     ascii = ':'; break;
                                case GLFW_KEY_APOSTROPHE:    ascii = '"'; break;
                                case GLFW_KEY_COMMA:         ascii = '<'; break;
                                case GLFW_KEY_PERIOD:        ascii = '>'; break;
                                case GLFW_KEY_SLASH:         ascii = '?'; break;
                                default: break;
                            }
                        }

                        Input::chatInput[Input::chatLength] = ascii;
                        Input::chatInput[Input::chatLength+1] = 0;
                        Input::chatLength++;
                        Input::localChatHasBeenUpdated = true;
                    }
                }
                break;   
            }
        }
    }
}
