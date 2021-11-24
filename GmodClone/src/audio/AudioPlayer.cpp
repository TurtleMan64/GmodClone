#include <AL/al.h>
#include <vector>
#include <cstring>
#include <fstream>

#include "audioplayer.hpp"
#include "audiomaster.hpp"
#include "source.hpp"
#include "../main/main.hpp"
#include "../toolbox/vector.hpp"
#include "../toolbox/split.hpp"
#include "../toolbox/getline.hpp"

float AudioPlayer::soundLevelSFX = 0.05f;
float AudioPlayer::soundLevelBGM = 0.05f;
std::vector<Source*> AudioPlayer::sources;
std::vector<ALuint> AudioPlayer::buffersSFX;
std::vector<ALuint> AudioPlayer::buffersBGM;
ALuint AudioPlayer::bgmIntro;
ALuint AudioPlayer::bgmLoop;
bool AudioPlayer::listenerIsUnderwater = false;

void AudioPlayer::loadSoundEffects()
{
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/jump.wav"));                    // 0
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/slide.wav"));                   // 1
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/swing_fast.wav"));              // 2 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/swing_slow.wav"));              // 3 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/water_splash.wav"));            // 4 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/bush_1.wav"));                  // 5 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/bush_2.wav"));                  // 6 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/bush_3.wav"));                  // 7 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_conc_1.wav"));    // 8 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_conc_2.wav"));    // 9 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_conc_3.wav"));    //10 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_conc_4.wav"));    //11
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_conc_5.wav"));    //12 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_dirt_1.wav"));    //13 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_dirt_2.wav"));    //14 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_dirt_3.wav"));    //15 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_dirt_4.wav"));    //16 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_gras_1.wav"));    //17 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_gras_2.wav"));    //18 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_gras_3.wav"));    //19 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_gras_4.wav"));    //20 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_mtlp_1.wav"));    //21 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_mtlp_2.wav"));    //22 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_mtlp_3.wav"));    //23 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_mtlp_4.wav"));    //24 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_mtlp_5.wav"));    //25 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_snow_1.wav"));    //26 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_snow_2.wav"));    //27 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_snow_3.wav"));    //28 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_snow_4.wav"));    //29 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_snow_5.wav"));    //30 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_watr_1.wav"));    //31 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_watr_2.wav"));    //32 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_watr_3.wav"));    //33 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_watr_4.wav"));    //34 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_watr_5.wav"));    //35 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_wood_1.wav"));    //36 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_wood_2.wav"));    //37 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_wood_3.wav"));    //38 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_wood_4.wav"));    //39 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_wood_1.wav"));    //40 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_wood_2.wav"));    //41 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/footstep/step_wood_3.wav"));    //42 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/land_conc.wav"));               //43 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/land_dirt.wav"));               //44 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/land_gras.wav"));               //45 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/land_mtlp.wav"));               //46 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/land_snow.wav"));               //47 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/land_watr.wav"));               //48 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/land_wood.wav"));               //49 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/Sonic/hit_wood.wav"));                //50 
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/HL2/player/fallpain3.wav"));          //51
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/HL2/weapon/crowbar_swing.wav"));      //52
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/HL2/weapon/explode3.wav"));           //53
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/HL2/weapon/crowbar_impact1.wav"));    //54
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/HL2/weapon/crowbar_impact2.wav"));    //55
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/HL2/misc/wpn_moveselect.wav"));       //56
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/SplatFruit/boing1.wav"));             //57
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/SplatFruit/boing2.wav"));             //58
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/SplatFruit/splat.wav"));              //59
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/HL2/footstep/hard_boot6_short.wav")); //60
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/HL2/misc/smallmedkit1.wav"));         //61
    AudioPlayer::buffersSFX.push_back(AudioMaster::loadWAV("res/Audio/SFX/HL2/physics/glass_largesheet_break1.wav")); //62
}

void AudioPlayer::loadBGM(char* fileName)
{
    AudioPlayer::buffersBGM.push_back(AudioMaster::loadOGG(fileName));
}

void AudioPlayer::deleteSources()
{
    for (Source* src : AudioPlayer::sources)
    {
        src->deleteMe();
        delete src; INCR_DEL("Source");
    }
    AudioPlayer::sources.clear();
    AudioPlayer::sources.shrink_to_fit();
}

void AudioPlayer::deleteBuffersSFX()
{
    for (ALuint buff : AudioPlayer::buffersSFX)
    {
        alDeleteBuffers(1, &buff);
    }
    AudioPlayer::buffersSFX.clear();
    AudioPlayer::buffersSFX.shrink_to_fit();
}

void AudioPlayer::deleteBuffersBGM()
{
    Source* src = AudioPlayer::sources[14];
    src->stop();

    for (ALuint buff : AudioPlayer::buffersBGM)
    {
        alDeleteBuffers(1, &buff);
    }
    AudioPlayer::buffersBGM.clear();
    AudioPlayer::buffersBGM.shrink_to_fit();
}

void AudioPlayer::createSources()
{
    //First 7 sources are for sound effects with relative effects (usually for sounds coming from the environment, other players)
    sources.push_back(new Source(1, 5, 40)); INCR_NEW("Source");
    sources.push_back(new Source(1, 5, 40)); INCR_NEW("Source");
    sources.push_back(new Source(1, 5, 40)); INCR_NEW("Source");
    sources.push_back(new Source(1, 5, 40)); INCR_NEW("Source");
    sources.push_back(new Source(1, 5, 40)); INCR_NEW("Source");
    sources.push_back(new Source(1, 5, 40)); INCR_NEW("Source");
    sources.push_back(new Source(1, 5, 40)); INCR_NEW("Source");

    //Next 7 sources are for sound effects without any relative effects (usually for noises made by the player)
    sources.push_back(new Source(0, 0, 0)); INCR_NEW("Source");
    sources.push_back(new Source(0, 0, 0)); INCR_NEW("Source");
    sources.push_back(new Source(0, 0, 0)); INCR_NEW("Source");
    sources.push_back(new Source(0, 0, 0)); INCR_NEW("Source");
    sources.push_back(new Source(0, 0, 0)); INCR_NEW("Source");
    sources.push_back(new Source(0, 0, 0)); INCR_NEW("Source");
    sources.push_back(new Source(0, 0, 0)); INCR_NEW("Source");

    //Last source is dedicated to background music
    sources.push_back(new Source(0, 0, 0)); INCR_NEW("Source");
}

//with position
Source* AudioPlayer::play(int buffer, Vector3f* pos)
{
    return AudioPlayer::play(buffer, pos, 1.0f, false, 0, 0, 0);
}

//with position and pitch
Source* AudioPlayer::play(int buffer, Vector3f* pos, float pitch)
{
    return AudioPlayer::play(buffer, pos, pitch, false, 0, 0, 0);
}

//with position and pitch and loop
Source* AudioPlayer::play(int buffer, Vector3f* pos, float pitch, bool loop)
{
    return AudioPlayer::play(buffer, pos, pitch, loop, 0, 0, 0);
}

//with everything
Source* AudioPlayer::play(int buffer, Vector3f* pos, float pitch, bool loop, float xVel, float yVel, float zVel)
{
    if (buffer >= (int)AudioPlayer::buffersSFX.size())
    {
        std::fprintf(stderr, "Error: Index out of bounds on SE buffers (%d / %d)\n", buffer, (int)AudioPlayer::buffersSFX.size());
        return nullptr;
    }

    int startIdx;
    float x, y, z;
    float velx, vely, velz;
    if (pos != nullptr)
    {
        startIdx = 0;
        x = pos->x;
        y = pos->y;
        z = pos->z;
        velx = xVel;
        vely = yVel;
        velz = zVel;
    }
    else
    {
        startIdx = 7;
        x = AudioMaster::spotInFrontOfListener.x;
        y = AudioMaster::spotInFrontOfListener.y;
        z = AudioMaster::spotInFrontOfListener.z;
        velx = AudioMaster::listenerVelCopy.x;
        vely = AudioMaster::listenerVelCopy.y;
        velz = AudioMaster::listenerVelCopy.z;
    }

    for (int i = startIdx; i < startIdx + 7; i++)
    {
        Source* src = AudioPlayer::sources[i];
        if (!src->isPlaying())
        {
            src->setVolume(soundLevelSFX);
            src->setLooping(loop);
            src->setPosition(x, y, z);
            src->setPitch(pitch);
            src->setVelocity(velx, vely, velz);
            src->play(AudioPlayer::buffersSFX[buffer]);
            return src;
        }
    }

    //no sources to play music
    return nullptr;
}

Source* AudioPlayer::playBGM(int bufferLoop)
{
    if (bufferLoop >= (int)AudioPlayer::buffersBGM.size() || bufferLoop < 0)
    {
        std::fprintf(stderr, "Error: Index out of bounds on BGM buffers (%d / %d)\n", bufferLoop, (int)AudioPlayer::buffersBGM.size());
        return nullptr;
    }

    return AudioPlayer::playBGM(AudioPlayer::buffersBGM[bufferLoop]);
}

Source* AudioPlayer::playBGM(ALuint bufferLoop)
{
    Source* src = AudioPlayer::sources[14];
    src->stop();
    src->setLooping(false);

    alSourcei(src->getSourceId(), AL_BUFFER, AL_NONE);

    src->setLooping(true);
    src->setVolume(AudioPlayer::soundLevelBGM);

    AudioPlayer::bgmIntro = AL_NONE;
    AudioPlayer::bgmLoop = bufferLoop;

    src->play(AudioPlayer::bgmLoop);

    return src;
}

Source* AudioPlayer::playBGMWithIntro(int bufferIntro, int bufferLoop)
{
    if (bufferIntro >= (int)AudioPlayer::buffersBGM.size() ||
        bufferLoop  >= (int)AudioPlayer::buffersBGM.size() ||
        bufferIntro < 0 ||
        bufferLoop  < 0)
    {
        std::fprintf(stderr, "Error: Index out of bounds on BGM buffers (%d / %d), (%d / %d)\n", bufferLoop, (int)AudioPlayer::buffersBGM.size(), bufferIntro, (int)AudioPlayer::buffersBGM.size());
        return nullptr;
    }

    return AudioPlayer::playBGMWithIntro(AudioPlayer::buffersBGM[bufferIntro], AudioPlayer::buffersBGM[bufferLoop]);
}

Source* AudioPlayer::playBGMWithIntro(ALuint bufferIntro, ALuint bufferLoop)
{
    Source* src = AudioPlayer::sources[14];
    src->stop();
    src->setVolume(AudioPlayer::soundLevelBGM);
    src->setLooping(false);


    alSourcei(src->getSourceId(), AL_BUFFER, AL_NONE); //Get rid of queued buffers 

    AudioPlayer::bgmIntro = bufferIntro;
    AudioPlayer::bgmLoop  = bufferLoop;

    alSourceQueueBuffers(src->getSourceId(), 1, &AudioPlayer::bgmIntro);
    alSourceQueueBuffers(src->getSourceId(), 1, &AudioPlayer::bgmLoop);
    alSourcePlay(src->getSourceId());

    //if (!src->isPlaying() || src->getLastPlayedBufferId() != AudioPlayer::buffersBGM[buffer])
    {
        //src->setLooping(true);
        //src->setVolume(AudioPlayer::soundLevelBGM);
        //src->play(AudioPlayer::buffersBGM[buffer]);
    }

    return src;
}

//Gets rid of the intro buffer, so that just the loop buffer loops
void AudioPlayer::refreshBGM()
{
    Source* src = AudioPlayer::sources[14];

    if (AudioPlayer::bgmIntro != AL_NONE)
    {
        ALint currentBufferIndex;
        alGetSourcei(src->getSourceId(), AL_BUFFERS_PROCESSED, &currentBufferIndex);
        if (currentBufferIndex == 1)
        {
            alSourceUnqueueBuffers(src->getSourceId(), 1, &AudioPlayer::bgmIntro);
            AudioPlayer::bgmIntro = AL_NONE;
            src->setLooping(true);
        }
    }
}

void AudioPlayer::stopBGM()
{
    Source* src = AudioPlayer::sources[14];
    src->stop();
    src->setLooping(false);

    alSourcei(src->getSourceId(), AL_BUFFER, AL_NONE); //Get rid of queued buffers 

    AudioPlayer::bgmIntro = AL_NONE;
    AudioPlayer::bgmLoop  = AL_NONE;
}

Source* AudioPlayer::getSource(int i)
{
    return AudioPlayer::sources[i];
}

ALuint AudioPlayer::getSFXBuffer(int i)
{
    return AudioPlayer::buffersSFX[i];
}

void AudioPlayer::setBGMVolume(float percent)
{
    Source* src = AudioPlayer::sources[14];
    src->setVolume(percent*soundLevelBGM);
}

void AudioPlayer::setListenerIsUnderwater(bool newIsUnderwater)
{
    if (AudioPlayer::listenerIsUnderwater != newIsUnderwater)
    {
        //update all sources to use the underwater filter

        for (int i = 0; i < 15; i++)
        {
            Source* src = AudioPlayer::sources[i];
            src->setIsUnderwater(newIsUnderwater);
        }
    }

    AudioPlayer::listenerIsUnderwater = newIsUnderwater;
}

void AudioPlayer::loadSettings()
{
    std::ifstream file(Global::pathToEXE+"Settings/AudioSettings.ini");
    if (!file.is_open())
    {
        std::fprintf(stderr, "Error: Cannot load file '%s'\n", (Global::pathToEXE+"Settings/AudioSettings.ini").c_str());
        file.close();
    }
    else
    {
        std::string line;

        while (!file.eof())
        {
            getlineSafe(file, line);

            char lineBuf[512];
            memcpy(lineBuf, line.c_str(), line.size()+1);

            int splitLength = 0;
            char** lineSplit = split(lineBuf, ' ', &splitLength);

            if (splitLength == 2)
            {
                if (strcmp(lineSplit[0], "SFX_Volume") == 0)
                {
                    AudioPlayer::soundLevelSFX = std::stof(lineSplit[1], nullptr);
                    AudioPlayer::soundLevelSFX = fmaxf(0.0f, fminf(AudioPlayer::soundLevelSFX, 1.0f));
                }
                else if (strcmp(lineSplit[0], "Music_Volume") == 0)
                {
                    AudioPlayer::soundLevelBGM = std::stof(lineSplit[1], nullptr);
                    AudioPlayer::soundLevelBGM = fmaxf(0.0f, fminf(AudioPlayer::soundLevelBGM, 1.0f));
                }
            }

            free(lineSplit);
        }
        file.close();
    }
}

void AudioPlayer::stopAllSFX()
{
    for (int i = 0; i < 14; i++)
    {
        if (AudioPlayer::sources[i]->isPlaying())
        {
            AudioPlayer::sources[i]->stop();
        }
    }
}
