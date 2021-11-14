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

float triggerSensitivity = 2;

int CONTROLLER_Id = 0; //-1 = no controller. otherwise, controller id

int BUTTON_A      = 0;
int BUTTON_X      = 1;
int BUTTON_B      = 2;
int BUTTON_Y      = 3;
int BUTTON_LB     = 4;
int BUTTON_RB     = 5;
int BUTTON_SELECT = 6;
int BUTTON_START  = 7;
int BUTTON_DPADU  = 11;
int BUTTON_DPADD  = 13;

int   STICK_LX       = 0;
float STICK_LX_SCALE = 1;
int   STICK_LY       = 1;
float STICK_LY_SCALE = 1;
int   STICK_RX       = 2;
float STICK_RX_SCALE = 1;
int   STICK_RY       = 3;
float STICK_RY_SCALE = 1;

float STICK_LXDEADZONE = 0.1f;
float STICK_LYDEADZONE = 0.1f;
float STICK_RXDEADZONE = 0.1f;
float STICK_RYDEADZONE = 0.1f;

int   TRIGGER_L  =  4;
float LT_NEUTRAL = -1;
float LT_MAX     =  1;
float LT_RANGE   =  2;
int   TRIGGER_R  =  5;
float RT_NEUTRAL = -1;
float RT_MAX     =  1;
float RT_RANGE   =  2;

float TRIGGER_DEADZONE = 0.3f;

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

    bool joystickIsPresent = false;

    #if GLFW_VERSION_MAJOR == 3 && GLFW_VERSION_MINOR == 3
    if ((CONTROLLER_Id != (GLFW_JOYSTICK_1 - 1)) && glfwJoystickIsGamepad(CONTROLLER_Id)) //joystick is both present and has a pre defined mapping
    {
        GLFWgamepadstate state;
        if (glfwGetGamepadState(CONTROLLER_Id, &state))
        {
            joystickIsPresent = true;

            Input::inputs.INPUT_ACTION1 = state.buttons[BUTTON_A];
            Input::inputs.INPUT_ACTION2 = state.buttons[BUTTON_X];
            Input::inputs.INPUT_ACTION3 = state.buttons[BUTTON_B];
            Input::inputs.INPUT_ACTION4 = state.buttons[BUTTON_Y];

            Input::inputs.INPUT_LB      = state.buttons[BUTTON_LB];
            Input::inputs.INPUT_RB      = state.buttons[BUTTON_RB];
            Input::inputs.INPUT_SELECT  = state.buttons[BUTTON_SELECT];
            Input::inputs.INPUT_START   = state.buttons[BUTTON_START];

            Input::inputs.INPUT_DPADU   = state.buttons[BUTTON_DPADU];
            Input::inputs.INPUT_DPADD   = state.buttons[BUTTON_DPADD];

            Input::inputs.INPUT_X  = state.axes[STICK_LX];
            Input::inputs.INPUT_Y  = state.axes[STICK_LY];
            Input::inputs.INPUT_X2 = state.axes[STICK_RX];
            Input::inputs.INPUT_Y2 = state.axes[STICK_RY];
            Input::inputs.INPUT_L2 = state.axes[TRIGGER_L];
            Input::inputs.INPUT_R2 = state.axes[TRIGGER_R];
        }
    }
    #endif

    if (!joystickIsPresent && (CONTROLLER_Id != (GLFW_JOYSTICK_1 - 1)) && glfwJoystickPresent(CONTROLLER_Id))
    {
        joystickIsPresent = true;

        int buttonCount;
        const unsigned char *buttons = glfwGetJoystickButtons(CONTROLLER_Id, &buttonCount);

        Input::inputs.INPUT_ACTION1  = buttons[BUTTON_A];
        Input::inputs.INPUT_ACTION2  = buttons[BUTTON_B];
        Input::inputs.INPUT_ACTION3  = buttons[BUTTON_X];
        Input::inputs.INPUT_ACTION4  = buttons[BUTTON_Y];
        Input::inputs.INPUT_RB       = buttons[BUTTON_RB];
        Input::inputs.INPUT_LB       = buttons[BUTTON_LB];
        Input::inputs.INPUT_START    = buttons[BUTTON_START];
        Input::inputs.INPUT_DPADU    = buttons[BUTTON_DPADU];
        Input::inputs.INPUT_DPADD    = buttons[BUTTON_DPADD];

        int axesCount;
        const float *axes = glfwGetJoystickAxes(CONTROLLER_Id, &axesCount);

        Input::inputs.INPUT_X = axes[STICK_LX];
        Input::inputs.INPUT_Y = axes[STICK_LY];

        Input::inputs.INPUT_X2 = axes[STICK_RX];
        Input::inputs.INPUT_Y2 = axes[STICK_RY];

        Input::inputs.INPUT_L2 = axes[TRIGGER_L];
        Input::inputs.INPUT_R2 = axes[TRIGGER_R];
    }

    if (joystickIsPresent) //apply scaling and deadzones
    {
        if (abs(Input::inputs.INPUT_X)  < STICK_LXDEADZONE)
        { 
            Input::inputs.INPUT_X  = 0;
        }
        else
        {
            const float RANGE = 1.0f - STICK_LXDEADZONE;
            if (Input::inputs.INPUT_X >= 0)
            {
                Input::inputs.INPUT_X = (Input::inputs.INPUT_X - STICK_LXDEADZONE)/RANGE;
            }
            else
            {
                Input::inputs.INPUT_X = (Input::inputs.INPUT_X + STICK_LXDEADZONE)/RANGE;
            }
        }

        if (abs(Input::inputs.INPUT_Y)  < STICK_LYDEADZONE)
        {
            Input::inputs.INPUT_Y  = 0;
        }
        else
        {
            const float RANGE = 1.0f - STICK_LYDEADZONE;
            if (Input::inputs.INPUT_Y >= 0)
            {
                Input::inputs.INPUT_Y = (Input::inputs.INPUT_Y - STICK_LYDEADZONE)/RANGE;
            }
            else
            {
                Input::inputs.INPUT_Y = (Input::inputs.INPUT_Y + STICK_LYDEADZONE)/RANGE;
            }
        }

        if (abs(Input::inputs.INPUT_X2) < STICK_RXDEADZONE)
        {
            Input::inputs.INPUT_X2 = 0;
        }
        else
        {
            const float RANGE = 1.0f - STICK_RXDEADZONE;
            if (Input::inputs.INPUT_X2 >= 0)
            {
                Input::inputs.INPUT_X2 = (Input::inputs.INPUT_X2 - STICK_RXDEADZONE)/RANGE;
            }
            else
            {
                Input::inputs.INPUT_X2 = (Input::inputs.INPUT_X2 + STICK_RXDEADZONE)/RANGE;
            }
        }


        if (abs(Input::inputs.INPUT_Y2) < STICK_RYDEADZONE)
        {
            Input::inputs.INPUT_Y2 = 0;
        }
        else
        {
            const float RANGE = 1.0f - STICK_RYDEADZONE;
            if (Input::inputs.INPUT_Y2 >= 0)
            {
                Input::inputs.INPUT_Y2 = (Input::inputs.INPUT_Y2 - STICK_RYDEADZONE)/RANGE;
            }
            else
            {
                Input::inputs.INPUT_Y2 = (Input::inputs.INPUT_Y2 + STICK_RYDEADZONE)/RANGE;
            }
        }

        Input::inputs.INPUT_X  *= STICK_LX_SCALE;
        Input::inputs.INPUT_Y  *= STICK_LY_SCALE;
        Input::inputs.INPUT_X2 *= STICK_RX_SCALE;
        Input::inputs.INPUT_Y2 *= STICK_RY_SCALE;

        Input::inputs.INPUT_X2 *= stickSensitivityX;
        Input::inputs.INPUT_Y2 *= stickSensitivityY;


        float triggerLValue = 0;
        float triggerRValue = 0;


        float rawValue = (Input::inputs.INPUT_L2 - LT_NEUTRAL) / LT_RANGE;
        if (rawValue >= TRIGGER_DEADZONE) { triggerLValue = rawValue; }

        rawValue = (Input::inputs.INPUT_R2 - RT_NEUTRAL) / RT_RANGE;
        if (rawValue >= TRIGGER_DEADZONE) { triggerRValue = rawValue; }


        Input::inputs.INPUT_X2 += triggerSensitivity * (triggerLValue - triggerRValue);

        Input::inputs.INPUT_L2 = triggerLValue;
        Input::inputs.INPUT_R2 = triggerRValue;
    }

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
}

void Input::init()
{
    Input::inputs.uniqueVar = 1149650285; //Value that is very easy to find with a memory scan

    glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);

    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }

    glfwSetScrollCallback(window, Input::scrollCallback);

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
                else if (strcmp(lineSplit[0], "Stick_X") == 0)
                {
                    stickSensitivityX = std::stof(lineSplit[1], nullptr);
                }
                else if (strcmp(lineSplit[0], "Stick_Y") == 0)
                {
                    stickSensitivityY = std::stof(lineSplit[1], nullptr);
                }
                else if (strcmp(lineSplit[0], "Triggers") == 0)
                {
                    triggerSensitivity = std::stof(lineSplit[1], nullptr);
                }
            }

            free(lineSplit);
        }
        file.close();
    }


    std::ifstream file2(Global::pathToEXE + "Settings/ControllerConfig.ini");
    if (!file2.is_open())
    {
        std::fprintf(stdout, "Error: Cannot load file '%s'\n", (Global::pathToEXE + "Settings/ControllerConfig.ini").c_str());
        file2.close();
    }
    else
    {
        std::string line;

        while (!file2.eof())
        {
            getlineSafe(file2, line);

            char lineBuf[512];
            memcpy(lineBuf, line.c_str(), line.size()+1);

            int splitLength = 0;
            char** lineSplit = split(lineBuf, ' ', &splitLength);

            if (splitLength == 2)
            {
                if (strcmp(lineSplit[0], "A") == 0)
                {
                    BUTTON_A = std::stoi(lineSplit[1], nullptr, 10);
                }
                else if (strcmp(lineSplit[0], "B") == 0)
                {
                    BUTTON_B = std::stoi(lineSplit[1], nullptr, 10);
                }
                else if (strcmp(lineSplit[0], "X") == 0)
                {
                    BUTTON_X = std::stoi(lineSplit[1], nullptr, 10);
                }
                else if (strcmp(lineSplit[0], "Y") == 0)
                {
                    BUTTON_Y = std::stoi(lineSplit[1], nullptr, 10);
                }
                else if (strcmp(lineSplit[0], "LB") == 0)
                {
                    BUTTON_LB = std::stoi(lineSplit[1], nullptr, 10);
                }
                else if (strcmp(lineSplit[0], "RB") == 0)
                {
                    BUTTON_RB = std::stoi(lineSplit[1], nullptr, 10);
                }
                else if (strcmp(lineSplit[0], "Start") == 0)
                {
                    BUTTON_START = std::stoi(lineSplit[1], nullptr, 10);
                }
                else if (strcmp(lineSplit[0], "DpadU") == 0)
                {
                    BUTTON_DPADU = std::stoi(lineSplit[1], nullptr, 10);
                }
                else if (strcmp(lineSplit[0], "DpadD") == 0)
                {
                    BUTTON_DPADD = std::stoi(lineSplit[1], nullptr, 10);
                }
                else if (strcmp(lineSplit[0], "Controller_Id") == 0)
                {
                    int raw = std::stoi(lineSplit[1], nullptr, 10);
                    switch (raw)
                    {
                        case 0:  CONTROLLER_Id = GLFW_JOYSTICK_1;  break;
                        case 1:  CONTROLLER_Id = GLFW_JOYSTICK_2;  break;
                        case 2:  CONTROLLER_Id = GLFW_JOYSTICK_3;  break;
                        case 3:  CONTROLLER_Id = GLFW_JOYSTICK_4;  break;
                        case 4:  CONTROLLER_Id = GLFW_JOYSTICK_5;  break;
                        case 5:  CONTROLLER_Id = GLFW_JOYSTICK_6;  break;
                        case 6:  CONTROLLER_Id = GLFW_JOYSTICK_7;  break;
                        case 7:  CONTROLLER_Id = GLFW_JOYSTICK_8;  break;
                        case 8:  CONTROLLER_Id = GLFW_JOYSTICK_9;  break;
                        case 9:  CONTROLLER_Id = GLFW_JOYSTICK_10; break;
                        case 10: CONTROLLER_Id = GLFW_JOYSTICK_11; break;
                        case 11: CONTROLLER_Id = GLFW_JOYSTICK_12; break;
                        case 12: CONTROLLER_Id = GLFW_JOYSTICK_13; break;
                        case 13: CONTROLLER_Id = GLFW_JOYSTICK_14; break;
                        case 14: CONTROLLER_Id = GLFW_JOYSTICK_15; break;
                        case 15: CONTROLLER_Id = GLFW_JOYSTICK_16; break;
                        default: CONTROLLER_Id = GLFW_JOYSTICK_1-1;break;
                    }
                }
            }
            else if (splitLength == 4)
            {
                if (strcmp(lineSplit[0], "Stick_LX") == 0)
                {
                    STICK_LX = std::stoi(lineSplit[1], nullptr, 10);
                    STICK_LXDEADZONE = std::stof(lineSplit[2], nullptr);
                    STICK_LX_SCALE = std::stof(lineSplit[3], nullptr);
                }
                else if (strcmp(lineSplit[0], "Stick_LY") == 0)
                {
                    STICK_LY = std::stoi(lineSplit[1], nullptr, 10);
                    STICK_LYDEADZONE = std::stof(lineSplit[2], nullptr);
                    STICK_LY_SCALE = std::stof(lineSplit[3], nullptr);
                }
                else if (strcmp(lineSplit[0], "Stick_RX") == 0)
                {
                    STICK_RX = std::stoi(lineSplit[1], nullptr, 10);
                    STICK_RXDEADZONE = std::stof(lineSplit[2], nullptr);
                    STICK_RX_SCALE = std::stof(lineSplit[3], nullptr);
                }
                else if (strcmp(lineSplit[0], "Stick_RY") == 0)
                {
                    STICK_RY = std::stoi(lineSplit[1], nullptr, 10);
                    STICK_RYDEADZONE = std::stof(lineSplit[2], nullptr);
                    STICK_RY_SCALE = std::stof(lineSplit[3], nullptr);
                }
            }
            else if (splitLength == 5)
            {
                if (strcmp(lineSplit[0], "Trigger_L") == 0)
                {
                    TRIGGER_L = std::stoi(lineSplit[1], nullptr, 10);
                    LT_NEUTRAL = std::stof(lineSplit[2], nullptr);
                    LT_MAX = std::stof(lineSplit[3], nullptr);
                    TRIGGER_DEADZONE = std::stof(lineSplit[4], nullptr);
                    LT_RANGE = LT_MAX - LT_NEUTRAL;
                }
                else if (strcmp(lineSplit[0], "Trigger_R") == 0)
                {
                    TRIGGER_R = std::stoi(lineSplit[1], nullptr, 10);
                    RT_NEUTRAL = std::stof(lineSplit[2], nullptr);
                    RT_MAX = std::stof(lineSplit[3], nullptr);
                    TRIGGER_DEADZONE = std::stof(lineSplit[4], nullptr);
                    RT_RANGE = RT_MAX - RT_NEUTRAL;
                }
            }

            free(lineSplit);
        }
        file2.close();
    }

    glfwPollEvents();

    //make sure no index goes out of bounds
    if ((CONTROLLER_Id != (GLFW_JOYSTICK_1 - 1)) && glfwJoystickPresent(CONTROLLER_Id) == GLFW_TRUE)
    {
        int axesCount;
        glfwGetJoystickAxes(CONTROLLER_Id, &axesCount);
        STICK_LX  = std::min(STICK_LX,  axesCount - 1);
        STICK_LY  = std::min(STICK_LY,  axesCount - 1);
        STICK_RX  = std::min(STICK_RX,  axesCount - 1);
        STICK_RY  = std::min(STICK_RY,  axesCount - 1);
        TRIGGER_L = std::min(TRIGGER_L, axesCount - 1);
        TRIGGER_R = std::min(TRIGGER_R, axesCount - 1);

        int buttonCount;
        glfwGetJoystickButtons(CONTROLLER_Id, &buttonCount);
        BUTTON_A     = std::min(BUTTON_A,     buttonCount - 1);
        BUTTON_B     = std::min(BUTTON_B,     buttonCount - 1);
        BUTTON_X     = std::min(BUTTON_X,     buttonCount - 1);
        BUTTON_Y     = std::min(BUTTON_Y,     buttonCount - 1);
        BUTTON_LB    = std::min(BUTTON_LB,    buttonCount - 1);
        BUTTON_RB    = std::min(BUTTON_RB,    buttonCount - 1);
        BUTTON_START = std::min(BUTTON_START, buttonCount - 1);
        BUTTON_DPADU = std::min(BUTTON_DPADU, buttonCount - 1);
        BUTTON_DPADD = std::min(BUTTON_DPADD, buttonCount - 1);
    }

    //log the controllers we see for humans if needed
    std::ofstream joyLog;
    joyLog.open((Global::pathToEXE + "Settings/ControllerLog.txt").c_str(), std::ios::out | std::ios::trunc);

    if (!joyLog.is_open())
    {
        std::fprintf(stderr, "Error: Failed to create/access '%s'\n", (Global::pathToEXE + "Settings/ControllerLog.txt").c_str());
        joyLog.close();
    }
    else
    {
        for (int i = GLFW_JOYSTICK_1; i < GLFW_JOYSTICK_LAST; i++)
        {
            joyLog << "Controller " + std::to_string(i) + " Name: ";
            const char* name = glfwGetJoystickName(i);
            if (name != nullptr)
            {
                joyLog << name;
            }
            joyLog << "\n";
        }

        joyLog.close();
    }
}

std::string Input::getControllerName()
{
    if (CONTROLLER_Id == (GLFW_JOYSTICK_1 - 1))
    {
        return "None";    
    }

    #if GLFW_VERSION_MAJOR == 3 && GLFW_VERSION_MINOR == 3
    const char* nameGamepad = glfwGetGamepadName(CONTROLLER_Id);
    if (nameGamepad != nullptr)
    {
        return nameGamepad;
    }
    #endif

    const char* nameJoystick = glfwGetJoystickName(CONTROLLER_Id);
    if (nameJoystick != nullptr)
    {
        return nameJoystick;
    }

    return "None";
}

bool Input::changeController(int direction)
{
    int originalControllerId = CONTROLLER_Id;
    int maxAttempts = (GLFW_JOYSTICK_LAST - GLFW_JOYSTICK_1) + 1;
    int currentAttempt = 0;
    if (direction >= 0)
    {
        direction = 1;
    }
    else
    {
        direction = -1;
    }

    while (currentAttempt < maxAttempts)
    {
        CONTROLLER_Id = (CONTROLLER_Id + direction);
        if (CONTROLLER_Id < GLFW_JOYSTICK_1 - 1)
        {
            CONTROLLER_Id = GLFW_JOYSTICK_LAST;
        }
        else if (CONTROLLER_Id > GLFW_JOYSTICK_LAST)
        {
            CONTROLLER_Id = GLFW_JOYSTICK_1 - 1;
        }

        if (CONTROLLER_Id == GLFW_JOYSTICK_1 - 1 || glfwJoystickPresent(CONTROLLER_Id))
        {
            return true;
        }

        currentAttempt++;
    }

    CONTROLLER_Id = originalControllerId;
    return false;
}

void Input::scrollCallback(GLFWwindow* /*w*/, double /*xOff*/, double yOff)
{
    mouseScroll = yOff;
}
