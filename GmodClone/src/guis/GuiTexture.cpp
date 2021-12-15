#include "guitexture.hpp"

#include <glad/glad.h>
#include "../toolbox/vector.hpp"

GuiTexture::GuiTexture()
{

}

GuiTexture::GuiTexture(GLuint textureId, Vector2f* position, Vector2f* size, int alignment)
{
    this->textureId = textureId;
    this->position.x = position->x;
    this->position.y = position->y;
    this->size.x = size->x;
    this->size.y = size->y;
    this->alignment = alignment;
}

GuiTexture::GuiTexture(GLuint textureId, float posX, float posY, float sizeX, float sizeY, int alignment)
{
    this->textureId = textureId;
    this->position.x = posX;
    this->position.y = posY;
    this->size.x = sizeX;
    this->size.y = sizeY;
    this->alignment = alignment;
}
