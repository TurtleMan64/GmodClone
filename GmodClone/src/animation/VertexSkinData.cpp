#include <vector>
#include <math.h>

#include "vertexskindata.hpp"

VertexSkinData::VertexSkinData()
{

}

void VertexSkinData::addJointEffect(int jointId, float weight)
{
    for (int i = 0; i < (int)weights.size(); i++)
    {
        if (weight > weights[i])
        {
            jointIds.insert(jointIds.begin() + i, jointId);
            weights.insert(weights.begin() + i, weight);
            return;
        }
    }
    jointIds.push_back(jointId);
    weights.push_back(weight);
}

void VertexSkinData::limitJointNumber(int max)
{
    if (jointIds.size() > max)
    {
        std::vector<float> topWeights;
        for (int i = 0; i < max; i++)
        {
            topWeights.push_back(0.0f);
        }
        float total = saveTopWeights(&topWeights);
        refillWeightList(&topWeights, total);
        removeExcessJointIds(max);
    }
    else if (jointIds.size() < max)
    {
        fillEmptyWeights(max);
    }
}

void VertexSkinData::fillEmptyWeights(int max)
{
    while (jointIds.size() < max)
    {
        jointIds.push_back(0);
        weights.push_back(0);
    }
}

float VertexSkinData::saveTopWeights(std::vector<float>* topWeightsArray)
{
    float total = 0;
    for (int i = 0; i < (int)topWeightsArray->size(); i++)
    {
        topWeightsArray->at(i) = weights[i];
        total += weights[i];
    }
    return total;
}

void VertexSkinData::refillWeightList(std::vector<float>* topWeights, float total)
{
    weights.clear();
    for (int i = 0; i < (int)topWeights->size(); i++)
    {
        weights.push_back(fminf(topWeights->at(i)/total, 1.0f));
    }
}

void VertexSkinData::removeExcessJointIds(int max)
{
    while (jointIds.size() > max)
    {
        jointIds.erase(jointIds.begin() + ((int)jointIds.size() - 1));
    }
}
