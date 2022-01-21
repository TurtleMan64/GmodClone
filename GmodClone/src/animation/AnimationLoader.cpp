#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <unordered_map>

#include "keyframe.hpp"
#include "animationloader.hpp"
#include "animation.hpp"
#include "jointtransform.hpp"
#include "../toolbox/matrix.hpp"
#include "../toolbox/vector.hpp"
#include "../toolbox/quaternion.hpp"
#include "../toolbox/split.hpp"
#include "../main/main.hpp"

Animation* AnimationLoader::loadAnimation(const char* filename)
{
    return AnimationLoader::loadAnimation((char*)filename);
}

Animation* AnimationLoader::loadAnimation(char* filename)
{
    std::string line;
    std::ifstream myfile((Global::pathToEXE + filename).c_str());
    if (myfile.is_open())
    {
        std::string rootBoneName;
        int numKeyframes;
        int numBones;

        getline(myfile, line);
        std::vector<std::string> tokens = split(line, ' ');
        rootBoneName = tokens[1];

        getline(myfile, line);
        tokens = split(line, ' ');
        numKeyframes = std::stoi(tokens[1]);

        getline(myfile, line);
        tokens = split(line, ' ');
        numBones = std::stoi(tokens[1]);

        Animation* animation = new Animation; INCR_NEW("Animation");

        getline(myfile, line);
        tokens = split(line, ' ');
        if (tokens[1] == "LOOP")
        {
            animation->timeWrappingType = 0;
        }
        else
        {
            animation->timeWrappingType = 1;
        }

        for (int currKey = 0; currKey < numKeyframes; currKey++)
        {
            getline(myfile, line);
            tokens = split(line, ' ');
            animation->length = std::stof(tokens[1]); //assume that keyframes are ordered by time

            Keyframe keyframe;
            keyframe.timeStamp = animation->length;

            for (int currBone = 0; currBone < numBones; currBone++)
            {
                getline(myfile, line);
                tokens = split(line, ' ');

                float localTransMat[16];
                for (int j = 0; j < 16; j++)
                {
                    localTransMat[j] = std::stof(tokens[j + 1]);
                }

                Matrix4f localTransform;
                localTransform.loadColumnFirst(localTransMat);

                if (rootBoneName == tokens[0])
                {
                    Matrix4f CORRECTION;
                    Vector3f xAxis(1, 0, 0);
                    CORRECTION.rotate(Maths::toRadians(-90), &xAxis);

                    CORRECTION.multiply(&localTransform, &localTransform);
                }

                Vector3f translation(localTransform.m30, localTransform.m31, localTransform.m32);
                Quaternion rotation = Quaternion::fromMatrix(&localTransform);

                if (rootBoneName == tokens[0])
                {
                    //for some reason, translations dont seem to work correctly on the root bone during rotations.
                    // setting these to 0 so that doesnt happen
                    translation.x = 0;
                    translation.z = 0;
                }

                JointTransform joint(translation, rotation);

                keyframe.pose[tokens[0]] = joint;
            }

            animation->keyframes.push_back(keyframe);
        }

        return animation;
    }
    else
    {
        printf("Error: Could not load animation file '%s'\n", (Global::pathToEXE + filename).c_str());
        return nullptr;
    }
}
