#ifndef SKIN_LOADER_H
#define SKIN_LOADER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <vector>

#include "../toolbox/maths.hpp"
#include "../toolbox/matrix.hpp"
#include "skinningdata.hpp"

class SkinLoader
{
public:
    int maxWeights;
    std::vector<std::string> jointNamesRaw;
    std::vector<std::string> weightsRaw;
    std::vector<std::string> vcountsRaw;
    std::vector<std::string> vRaw;

    SkinLoader(std::vector<std::string> jointNamesRaw,
        std::vector<std::string> weightsRaw,
        std::vector<std::string> vcountsRaw,
        std::vector<std::string> vRaw,
        int maxWeights);

    SkinningData extractSkinData();

private:
    std::vector<std::string> loadJointsList();

    std::vector<float> loadWeights();

    std::vector<int> getEffectiveJointsCounts();

    std::vector<VertexSkinData> getSkinData(std::vector<int> counts, std::vector<float> weights);
};
#endif
