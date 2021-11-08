#ifndef AUDIOMASTER_H
#define AUDIOMASTER_H

class Vector3f;

#include <AL/al.h>
#include <AL/alc.h>

#include "../toolbox/vector.hpp"

class AudioMaster
{
private:
    static ALCdevice* device;
    static ALCcontext* context;

public:
    static Vector3f spotInFrontOfListener;
    static Vector3f listenerVelCopy;

    static void init();

    static void updateListenerData(Vector3f* eye, Vector3f* target, Vector3f* up, Vector3f* vel);

    static ALuint loadOGG(const char* fileName);

    static ALuint loadWAV(const char* fileName);
    
    static void cleanUp();
};
#endif
