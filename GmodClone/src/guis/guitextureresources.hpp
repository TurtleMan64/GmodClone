#ifndef GUITEXTURERESOURCES_H
#define GUITEXTURERESOURCES_H

class GuiTexture;

class GuiTextureResources
{
public:
    static GuiTexture* textureChatBG;
    static GuiTexture* textureLocalChatBG;
    static GuiTexture* textureCrosshair;
    static GuiTexture* textureHealthbar;
    static GuiTexture* textureHealthbarBG;
    static GuiTexture* texturePlayersBG;
    static GuiTexture* textureMap1;
    static GuiTexture* textureMap2;
    static GuiTexture* textureMap4;
    static GuiTexture* textureMap5;
    static GuiTexture* textureMap6;
    static GuiTexture* textureMap7;
    static GuiTexture* textureMapBG;

    static void loadGuiTextures();
};
#endif
