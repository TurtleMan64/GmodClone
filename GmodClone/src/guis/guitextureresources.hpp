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

    static void loadGuiTextures();
};
#endif
