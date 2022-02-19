#ifndef INPUT_H
#define INPUT_H

#include <GLFW/glfw3.h>
#include <string>

struct InputStruct
{
    volatile int uniqueVar;

    volatile float INPUT_X;
    volatile float INPUT_Y;
    volatile float INPUT_X2;
    volatile float INPUT_Y2;
    volatile float INPUT_R2;
    volatile float INPUT_L2;

    volatile bool INPUT_ACTION1; //A button
    volatile bool INPUT_ACTION2; //B button
    volatile bool INPUT_ACTION3; //X button
    volatile bool INPUT_ACTION4; //Y button
    volatile bool INPUT_LEFT_CLICK;
    volatile bool INPUT_RIGHT_CLICK;
    volatile bool INPUT_RB;
    volatile bool INPUT_LB;
    volatile bool INPUT_SELECT;
    volatile bool INPUT_START;
    volatile bool INPUT_ESC;
    volatile bool INPUT_F5;
    volatile bool INPUT_DPADU;
    volatile bool INPUT_DPADD;

    volatile float INPUT_PREVIOUS_X;
    volatile float INPUT_PREVIOUS_Y;
    volatile float INPUT_PREVIOUS_X2;
    volatile float INPUT_PREVIOUS_Y2;
    volatile float INPUT_PREVIOUS_R2;
    volatile float INPUT_PREVIOUS_L2;

    volatile bool INPUT_PREVIOUS_ACTION1;
    volatile bool INPUT_PREVIOUS_ACTION2;
    volatile bool INPUT_PREVIOUS_ACTION3;
    volatile bool INPUT_PREVIOUS_ACTION4;
    volatile bool INPUT_PREVIOUS_LEFT_CLICK;
    volatile bool INPUT_PREVIOUS_RIGHT_CLICK;
    volatile bool INPUT_PREVIOUS_RB;
    volatile bool INPUT_PREVIOUS_LB;
    volatile bool INPUT_PREVIOUS_START;
    volatile bool INPUT_PREVIOUS_ESC;
    volatile bool INPUT_PREVIOUS_F5;

    volatile int MENU_X;
    volatile int MENU_Y;

    volatile int INPUT_SCROLL;

    volatile int approxXLeft;
    volatile int approxXLeftPrevious;
    volatile int approxYLeft;
    volatile int approxYLeftPrevious;
};

class Input
{
private:
    static void scrollCallback(GLFWwindow* window, double xOff, double yOff);

    static void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

public:
    static InputStruct inputs;

    static char* chatInput;
    static int chatLength;
    static bool isTypingInChat;
    static bool localChatHasBeenUpdated;

    static void init();

    static void pollInputs();
};

#endif
