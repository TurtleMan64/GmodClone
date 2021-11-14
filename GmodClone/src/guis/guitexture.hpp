#ifndef GUITEXTURE_H
#define GUITEXTURE_H

#include <glad/glad.h>
#include "../toolbox/vector.hpp"

class GuiTexture
{
public:
    GLuint textureId = GL_NONE;
    Vector2f position;
    Vector2f size; //x = 1: full screen height (NOT width). y = 1: full screen height
    bool visible = true;
    float alpha = 1.0f;
    int alignment = 0;

    GuiTexture();

    //x, y = (0, 0) is top left, (1, 1) is bottom right
    //size = 1.0 = full screen height
    //alignment chart:
    //  0 1 2
    //  3 4 5
    //  6 7 8
    GuiTexture(GLuint textureId, Vector2f* position, Vector2f* size, int alignment);

    //x, y = (0, 0) is top left, (1, 1) is bottom right
    //size = 1.0 = full screen height
    //alignment chart:
    //  0 1 2
    //  3 4 5
    //  6 7 8
    GuiTexture(GLuint textureId, float posX, float posY, float sizeX, float sizeY, int alignment);
};

#endif
