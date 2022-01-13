#include <string>
#include <vector>

#include "skinloader.hpp"
#include "../toolbox/vector.hpp"
#include "../toolbox/maths.hpp"
#include "../toolbox/matrix.hpp"

SkinLoader::SkinLoader(
    std::vector<std::string> jointNamesRaw,
    std::vector<std::string> weightsRaw,
    std::vector<std::string> vcountsRaw,
    std::vector<std::string> vRaw,
    int maxWeights)
{
    this->jointNamesRaw = jointNamesRaw;
    this->weightsRaw    = weightsRaw;
    this->vcountsRaw    = vcountsRaw;
    this->vRaw          = vRaw;
    this->maxWeights    = maxWeights;

    this->jointNamesRaw.erase(this->jointNamesRaw.begin());
    this->weightsRaw   .erase(this->weightsRaw.begin());
    this->vcountsRaw   .erase(this->vcountsRaw.begin());
    this->vRaw         .erase(this->vRaw.begin());
}

SkinningData SkinLoader::extractSkinData()
{
    std::vector<std::string> jointsNames = loadJointsList();
    std::vector<float> weights = loadWeights();
    std::vector<int> effectorJointCounts = getEffectiveJointsCounts();
    std::vector<VertexSkinData> vertexWeights = getSkinData(effectorJointCounts, weights);
    return SkinningData(jointsNames, vertexWeights);
}

std::vector<std::string> SkinLoader::loadJointsList()
{
    std::vector<std::string> j;
    for (int i = 0; i < (int)jointNamesRaw.size(); i++)
    {
        j.push_back(jointNamesRaw[i]);
    }
    return j;
}

std::vector<float> SkinLoader::loadWeights()
{
    std::vector<float> w;
    for (int i = 0; i < (int)weightsRaw.size(); i++)
    {
        w.push_back(std::stof(weightsRaw[i]));
    }
    return w;
}

std::vector<int> SkinLoader::getEffectiveJointsCounts()
{
    std::vector<int> v2;
    for (int i = 0; i < (int)vcountsRaw.size(); i++)
    {
        v2.push_back(std::stoi(vcountsRaw[i]));
    }
    return v2;
}

std::vector<VertexSkinData> SkinLoader::getSkinData(std::vector<int> counts, std::vector<float> weights)
{
    std::vector<VertexSkinData> skinningData;
    int pointer = 0;
    for (int count : counts)
    {
        VertexSkinData skinData;
        for (int i = 0; i < count; i++)
        {
            int jointId  = std::stoi(vRaw[pointer]); pointer++;
            int weightId = std::stoi(vRaw[pointer]); pointer++;
            skinData.addJointEffect(jointId, weights[weightId]);
        }
        skinData.limitJointNumber(maxWeights);
        skinningData.push_back(skinData);
    }
    return skinningData;
}
