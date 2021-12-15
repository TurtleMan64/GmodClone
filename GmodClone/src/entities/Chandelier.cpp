#include <list>

#include "../toolbox/vector.hpp"
#include "chandelier.hpp"
#include "../main/main.hpp"
#include "../toolbox/maths.hpp"
#include "../loader/objloader.hpp"
#include "entity.hpp"
#include "light.hpp"

std::list<TexturedModel*> Chandelier::modelsChandelier;
std::list<TexturedModel*> Chandelier::modelsWallLamp;

extern float dt;

Chandelier::Chandelier(std::string name, Vector3f pos, int type, float rotY, float attenuation2, float attenuation3, int lightIdx)
{
    this->name = name;
    this->type = type;
    this->lightIdx = lightIdx;
    this->rotY = rotY;
    this->attenuation2 = attenuation2;
    this->attenuation3 = attenuation3;

    scale = 1.5f;

    position = pos;
    visible = true;

    entitiesToRender.push_back(this);

    if (type == 0)
    {
        Global::lights[lightIdx]->position = position;
        Global::lights[lightIdx]->position.y -= 0.691178f*scale;
        Global::lights[lightIdx]->attenuation.set(1, attenuation2, attenuation3);
        Global::lights[lightIdx]->color.set(1, 1, 1);
    }
    else
    {
        Vector3f off(0.511914f*scale, 0, 0);
        Vector3f yAxis(0, 1, 0);
        off = Maths::rotatePoint(&off, &yAxis, Maths::toRadians(rotY));
        Global::lights[lightIdx]->position = position + off;
        Global::lights[lightIdx]->attenuation.set(1, attenuation2, attenuation3);
        Global::lights[lightIdx]->color.set(0.5f, 0.5f, 0.5f);
    }

    updateTransformationMatrix();
}

Chandelier::~Chandelier()
{

}

void Chandelier::step()
{
    flickerTimer += dt;

    if (flickerTimer > 0.02f)
    {
        float ran = Maths::nextGaussian();
        if (ran < 0.0f)
        {
            ran = 0.0f;
        }
        Global::lights[lightIdx]->attenuation.y = attenuation2 + ran*0.01f;

        flickerTimer -= 0.02f;
    }
}

std::vector<Entity*>* Chandelier::getEntitiesToRender()
{
    return &entitiesToRender;
}

std::list<TexturedModel*>* Chandelier::getModels()
{
    if (type == 0)
    {
        return &Chandelier::modelsChandelier;
    }
    else
    {
        return &Chandelier::modelsWallLamp;
    }
}

int Chandelier::getEntityType()
{
    return ENTITY_CHANDELIER;
}

void Chandelier::loadModels()
{
    if (Chandelier::modelsChandelier.size() == 0)
    {
        ObjLoader::loadModel(&Chandelier::modelsChandelier, "res/Models/Chandelier/", "Chandelier");
        ObjLoader::loadModel(&Chandelier::modelsWallLamp,   "res/Models/Chandelier/", "Lamp");
    }
}
