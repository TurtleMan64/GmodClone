#include <list>

#include "fonttype.hpp"
#include "textmeshcreator.hpp"
#include "textmeshdata.hpp"

#include "../renderEngine/renderEngine.hpp"

#include "../main/main.hpp"

FontType::FontType(int textureAtlas, std::string fontFilename)
{
    this->textureAtlas = textureAtlas;
    this->loader = new TextMeshCreator(fontFilename); INCR_NEW("TextMeshCreator");
}

int FontType::getTextureAtlas()
{
    return textureAtlas;
}

void FontType::deleteMe()
{
    Loader::deleteTexture(textureAtlas);
    textureAtlas = -1;
}

TextMeshData* FontType::loadText(GUIText* text)
{
    return loader->createTextMesh(text);
}
