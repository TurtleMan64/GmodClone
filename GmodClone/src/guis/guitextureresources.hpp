#ifndef GUITEXTURERESOURCES_H
#define GUITEXTURERESOURCES_H

class GuiTexture;

class GuiTextureResources
{
public:
    static GuiTexture* textureChatBG;
    static GuiTexture* textureLocalChatBG;

    static void loadGuiTextures();
};
#endif
