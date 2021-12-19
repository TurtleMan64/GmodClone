#include "guitextureresources.hpp"
#include "guitexture.hpp"
#include "../renderEngine/renderEngine.hpp"
#include "../main/main.hpp"

GuiTexture* GuiTextureResources::textureChatBG      = nullptr;
GuiTexture* GuiTextureResources::textureLocalChatBG = nullptr;
GuiTexture* GuiTextureResources::textureCrosshair   = nullptr;
GuiTexture* GuiTextureResources::textureHealthbar   = nullptr;
GuiTexture* GuiTextureResources::textureHealthbarBG = nullptr;
GuiTexture* GuiTextureResources::texturePlayersBG   = nullptr;
GuiTexture* GuiTextureResources::textureMap1        = nullptr;
GuiTexture* GuiTextureResources::textureMap2        = nullptr;
GuiTexture* GuiTextureResources::textureMap4        = nullptr;
GuiTexture* GuiTextureResources::textureMap5        = nullptr;
GuiTexture* GuiTextureResources::textureMapBG       = nullptr;

void GuiTextureResources::loadGuiTextures()
{
    INCR_NEW("GuiTexture"); textureChatBG      = new GuiTexture(Loader::loadTexture("res/Images/Black.png"),     0.007f, 0.99f-0.025f, 0.425f, 0.025f, 6); textureChatBG     ->alpha = 0.0f;
    INCR_NEW("GuiTexture"); textureLocalChatBG = new GuiTexture(Loader::loadTexture("res/Images/Black.png"),     0.007f, 0.99f,        0.425f, 0.025f, 6); textureLocalChatBG->alpha = 0.7f;
    INCR_NEW("GuiTexture"); textureCrosshair   = new GuiTexture(Loader::loadTexture("res/Images/Crosshair.png"), 0.500f, 0.50f,        0.025f, 0.025f, 4); textureCrosshair  ->alpha = 0.7f;
    INCR_NEW("GuiTexture"); textureHealthbar   = new GuiTexture(Loader::loadTexture("res/Images/Health.png"),    0.993f, 0.965f,       0.425f, 0.050f, 8); textureHealthbar  ->alpha = 0.9f;
    INCR_NEW("GuiTexture"); textureHealthbarBG = new GuiTexture(Loader::loadTexture("res/Images/Black.png"),     0.993f, 0.965f,       0.425f, 0.050f, 8); textureHealthbarBG->alpha = 0.9f;
    INCR_NEW("GuiTexture"); texturePlayersBG   = new GuiTexture(Loader::loadTexture("res/Images/Black.png"),     0.500f, 0.09f,        0.590f, 0.050f, 1); texturePlayersBG  ->alpha = 0.5f;
    INCR_NEW("GuiTexture"); textureMap1        = new GuiTexture(Loader::loadTexture("res/Images/Map1.jpg"),      0.500f, 0.50f,        1.333f, 1.000f, 4); textureMap1       ->alpha = 1.0f;
    INCR_NEW("GuiTexture"); textureMap2        = new GuiTexture(Loader::loadTexture("res/Images/Map2.jpg"),      0.500f, 0.50f,        1.333f, 1.000f, 4); textureMap2       ->alpha = 1.0f;
    INCR_NEW("GuiTexture"); textureMap4        = new GuiTexture(Loader::loadTexture("res/Images/Map4.jpg"),      0.500f, 0.50f,        1.333f, 1.000f, 4); textureMap4       ->alpha = 1.0f;
    INCR_NEW("GuiTexture"); textureMap5        = new GuiTexture(Loader::loadTexture("res/Images/Map5.jpg"),      0.500f, 0.50f,        1.333f, 1.000f, 4); textureMap5       ->alpha = 1.0f;
    INCR_NEW("GuiTexture"); textureMapBG       = new GuiTexture(Loader::loadTexture("res/Images/Black.png"),     0.500f, 0.50f,        5.000f, 5.000f, 4); textureMapBG      ->alpha = 1.0f;
}
