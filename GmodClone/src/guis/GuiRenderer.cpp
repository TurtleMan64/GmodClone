#include "guirenderer.hpp"
#include "guishader.hpp"
#include "guitexture.hpp"

#include <glad/glad.h>
#include "../main/main.hpp"
#include "../renderEngine/renderEngine.hpp"
#include "../models/models.hpp"
#include "../toolbox/maths.hpp"
#include "../toolbox/matrix.hpp"

#include <vector>

RawModel GuiRenderer::quadModel;
GuiShader* GuiRenderer::shader = nullptr;

void GuiRenderer::init()
{
    std::vector<float> positions;
    positions.push_back(0);
    positions.push_back(0);
    positions.push_back(0);
    positions.push_back(1);
    positions.push_back(1);
    positions.push_back(0);
    positions.push_back(1);
    positions.push_back(1);

    GuiRenderer::quadModel = Loader::loadToVAO(&positions, 2);
    GuiRenderer::shader = new GuiShader("res/Shaders/gui/GuiVert.glsl", "res/Shaders/gui/GuiFrag.glsl"); INCR_NEW("GuiShader");
}

void GuiRenderer::render(std::list<GuiTexture*>* guis)
{
    GuiRenderer::shader->start();
    glBindVertexArray(GuiRenderer::quadModel.getVaoId());
    glEnableVertexAttribArray(0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    extern unsigned int SCR_WIDTH;
    extern unsigned int SCR_HEIGHT;

    float ratio = ((float)SCR_HEIGHT)/SCR_WIDTH;

    for (GuiTexture* gui: (*guis))
    {
        if (gui->visible)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gui->textureId);

            Matrix4f matrix;
            Vector2f scale = gui->size;
            scale.x = scale.x*ratio;

            float offX = 0.0f;
            float offY = 0.0f;

            if (gui->alignment == 3 ||
                gui->alignment == 4 ||
                gui->alignment == 5)
            {
                offY = -((scale.y)/2);
            }
            else if (gui->alignment == 6 ||
                     gui->alignment == 7 ||
                     gui->alignment == 8)
            {
                offY = -(scale.y);
            }

            if (gui->alignment == 1 ||
                gui->alignment == 4 ||
                gui->alignment == 7)
            {
                offX = (-(scale.x/2));
            }
            else if (gui->alignment == 2 ||
                     gui->alignment == 5 ||
                     gui->alignment == 8)
            {
                offX = -(scale.x);
            }

            Vector2f off(offX + gui->position.x, offY + gui->position.y);

            Maths::createTransformationMatrix(&matrix, &off, 0, &scale);

            GuiRenderer::shader->loadTransformation(&matrix);
            GuiRenderer::shader->loadAlpha(gui->alpha);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, GuiRenderer::quadModel.getVertexCount());
        }
    }
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisableVertexAttribArray(0);
    glBindVertexArray(0);
    GuiRenderer::shader->stop();
}

void GuiRenderer::cleanUp()
{
    GuiRenderer::shader->cleanUp();
    delete GuiRenderer::shader; INCR_DEL("GuiShader");
    GuiRenderer::shader = nullptr;
}
