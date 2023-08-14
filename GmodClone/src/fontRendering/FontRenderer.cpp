#include <glad/glad.h>
#include <list>
#include "fontrenderer.hpp"
#include "../fontMeshCreator/fonttype.hpp"
#include "../fontMeshCreator/guitext.hpp"
#include "../fontMeshCreator/guinumber.hpp"
#include "fontshader.hpp"
#include "../main/main.hpp"

FontRenderer::FontRenderer()
{
    shader = new FontShader("res/Shaders/font/FontVert.glsl", "res/Shaders/font/FontFrag.glsl"); INCR_NEW("FontShader");
}

void FontRenderer::render(
    std::unordered_map<FontType*, std::list<GUIText*>>* texts,
    std::unordered_map<FontType*, std::list<GUINumber*>>* numbers)
{
    prepare();
    for (auto kv : (*texts))
    {
        FontType* font = kv.first;
        std::list<GUIText*> listOfTexts = kv.second;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, font->getTextureAtlas());
        for (GUIText* text : listOfTexts)
        {
            if (text->isVisible())
            {
                renderText(text);
            }
        }
    }
    for (auto kv : (*numbers))
    {
        FontType* font = kv.first;
        std::list<GUINumber*> listOfNumbers = kv.second;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, font->getTextureAtlas());
        for (GUINumber* number : listOfNumbers)
        {
            if (number->visible)
            {
                renderNumber(number);
            }
        }
    }
    endRendering();
}

void FontRenderer::cleanUp()
{
    shader->cleanUp();
}

void FontRenderer::prepare()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    shader->start();
}

void FontRenderer::renderText(GUIText* text)
{
    glBindVertexArray(text->getMesh());
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    extern unsigned int SCR_WIDTH;
    extern unsigned int SCR_HEIGHT;

    float ratio = ((float)SCR_HEIGHT)/SCR_WIDTH;

    shader->loadScreenRatio(ratio);
    shader->loadScreenHeight(SCR_HEIGHT);
    shader->loadFontHeight(text->fontSize);
    shader->loadColor(text->getColor());

    float offX = 0.0f;
    float offY = 0.0f;

    if (text->alignment == 3 ||
        text->alignment == 4 ||
        text->alignment == 5)
    {
        offY = -((text->fontSize)/2);
    }
    else if (text->alignment == 6 ||
             text->alignment == 7 ||
             text->alignment == 8)
    {
        offY = -(text->fontSize);
    }

    if (text->alignment == 1 ||
        text->alignment == 4 ||
        text->alignment == 7)
    {
        offX = (-(text->maxLineWidth/2))*ratio*text->fontSize;
    }
    else if (text->alignment == 2 ||
             text->alignment == 5 ||
             text->alignment == 8)
    {
        offX = -(text->maxLineWidth)*ratio*text->fontSize;
    }

    Vector2f off(offX + text->position.x, offY + text->position.y);
    shader->loadTranslation(&off);
    shader->loadAlpha(text->alpha);

    glDrawArrays(GL_TRIANGLES, 0, text->getVertexCount());
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindVertexArray(0);
}

void FontRenderer::renderNumber(GUINumber* number)
{
    int numChars = (int)number->meshIds.size();
    shader->loadFontHeight(number->size);
    for (int i = 0; i < numChars; i++)
    {
        glBindVertexArray(number->meshIds[i]);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        shader->loadColor(&number->colors[i]);
        shader->loadTranslation(&number->meshPositions[i]);
        glDrawArrays(GL_TRIANGLES, 0, number->meshVertexCounts[i]);
    }

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindVertexArray(0);
}

void FontRenderer::endRendering()
{
    shader->stop();
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}
