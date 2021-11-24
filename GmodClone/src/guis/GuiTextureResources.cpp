#include "guitextureresources.hpp"
#include "guitexture.hpp"
#include "../renderEngine/renderEngine.hpp"
#include "../main/main.hpp"

GuiTexture* GuiTextureResources::textureChatBG      = nullptr;
GuiTexture* GuiTextureResources::textureLocalChatBG = nullptr;
GuiTexture* GuiTextureResources::textureCrosshair   = nullptr;
GuiTexture* GuiTextureResources::textureHealthbar   = nullptr;
GuiTexture* GuiTextureResources::textureHealthbarBG = nullptr;

void GuiTextureResources::loadGuiTextures()
{
    INCR_NEW("GuiTexture"); textureChatBG      = new GuiTexture(Loader::loadTexture("res/Images/Black.png"),     0.007f, 0.99f-0.025f, 0.425f, 0.025f, 6); textureChatBG     ->alpha = 0.0f;
    INCR_NEW("GuiTexture"); textureLocalChatBG = new GuiTexture(Loader::loadTexture("res/Images/Black.png"),     0.007f, 0.99f,        0.425f, 0.025f, 6); textureLocalChatBG->alpha = 0.7f;
    INCR_NEW("GuiTexture"); textureCrosshair   = new GuiTexture(Loader::loadTexture("res/Images/Crosshair.png"), 0.500f, 0.50f,        0.025f, 0.025f, 4); textureCrosshair  ->alpha = 0.7f;
    INCR_NEW("GuiTexture"); textureHealthbar   = new GuiTexture(Loader::loadTexture("res/Images/Health.png"),    0.993f, 0.965f,       0.425f, 0.050f, 8); textureHealthbar  ->alpha = 0.9f;
    INCR_NEW("GuiTexture"); textureHealthbarBG = new GuiTexture(Loader::loadTexture("res/Images/Black.png"),     0.993f, 0.965f,       0.425f, 0.050f, 8); textureHealthbarBG->alpha = 0.9f;
}
