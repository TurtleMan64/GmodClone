//To get rid of fopen error
//#ifdef _WIN32
//#define _CRT_SECURE_NO_DEPRECATE
//#endif

#define AL_ALEXT_PROTOTYPES

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/efx.h>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <stdio.h>
#include <errno.h>
#include <cstdio>
#endif

#include <vorbis/vorbisfile.h>

#include "audiomaster.hpp"
#include "../toolbox/vector.hpp"
#include "source.hpp"
#include "audioplayer.hpp"
#include "../toolbox/maths.hpp"
#include "../main/main.hpp"

ALCdevice*  AudioMaster::device = nullptr;
ALCcontext* AudioMaster::context = nullptr;
Vector3f AudioMaster::spotInFrontOfListener;
Vector3f AudioMaster::listenerVelCopy;

void AudioMaster::init()
{
    AudioMaster::device = alcOpenDevice(nullptr);
    if (AudioMaster::device == nullptr)
    {
        fprintf(stderr, "no sound device\n");
        return;
    }

    AudioMaster::context = alcCreateContext(AudioMaster::device, nullptr);
    alcMakeContextCurrent(AudioMaster::context);
    if (AudioMaster::context == nullptr)
    {
        fprintf(stderr, "no sound context\n");
        return;
    }

    printf("OpenAL version: %s\n", alGetString(AL_VERSION));
    printf("OpenAL device:  %s\n", alcGetString(AudioMaster::device, ALC_CAPTURE_DEVICE_SPECIFIER));

    /* Try to create a Filter */
    ALuint filterLowpass;
    ALenum erral = alGetError();
    if (erral != AL_NO_ERROR)
    {
        std::fprintf(stderr, "########  AL ERROR  ########\n");
        std::fprintf(stderr, "%d\n", erral);
    }

    alGenFilters(1, &filterLowpass);
    erral = alGetError();
    if (erral != AL_NO_ERROR)
    {
        printf("Could not generate a filter. error code = %d\n", erral);
    }
    else if (alIsFilter(filterLowpass))
    {
        // Set Filter type to Low-Pass and set parameters
        alFilteri(filterLowpass, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
        if (alGetError() != AL_NO_ERROR)
        {
            printf("Low Pass Filter not supported\n");
        }
        else
        {
            alFilterf(filterLowpass, AL_LOWPASS_GAIN, 1.0f); //1.0
            alFilterf(filterLowpass, AL_LOWPASS_GAINHF, 0.015f); //0.004
        }
    }

    Source::filterLowpass = filterLowpass;

    alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
    alListenerf(AL_GAIN, 1.0f);

    AudioPlayer::loadSettings();
    AudioPlayer::createSources();
    AudioPlayer::loadSoundEffects();
}

void AudioMaster::updateListenerData(Vector3f* eye, Vector3f* target, Vector3f* up, Vector3f* vel)
{
    Vector3f at(target);
    at = at - eye;
    at.normalize();

    Vector3f perpen = at.cross(up);

    float u3 = perpen.x;
    float v3 = perpen.y;
    float w3 = perpen.z;
    float x3 = at.x;
    float y3 = at.y;
    float z3 = at.z;
    float buf[3];
    Maths::rotatePoint(buf, 0, 0, 0, u3, v3, w3, x3, y3, z3, Maths::toRadians(90));

    Vector3f up2(buf[0], buf[1], buf[2]);
    up2.normalize();

    ALfloat listenerPos[] = { eye->x, eye->y, eye->z };
    ALfloat listenerVel[] = { vel->x/3.0f, vel->y/3.0f, vel->z/3.0f }; // over 3 to scale down doppler effect
    ALfloat listenerOri[] = { at.x, at.y, at.z, up2.x, up2.y, up2.z };

    alListenerfv(AL_POSITION,    listenerPos);
    alListenerfv(AL_VELOCITY,    listenerVel);
    alListenerfv(AL_ORIENTATION, listenerOri);

    AudioMaster::spotInFrontOfListener.set(eye->x + at.x, eye->y + at.y, eye->z + at.z);
    AudioMaster::listenerVelCopy.set(listenerVel[0], listenerVel[1], listenerVel[2]);

    for (int i = 7; i < 14; i++)
    {
        Source* src = AudioPlayer::getSource(i);
        src->setPosition(spotInFrontOfListener.x, spotInFrontOfListener.y, spotInFrontOfListener.z);
        src->setVelocity(listenerVel[0], listenerVel[1], listenerVel[2]);
    }
}

ALuint AudioMaster::loadOGG(const char* fileName)
{
    FILE* fp = nullptr;

    #ifdef _WIN32
    int er = fopen_s(&fp, (Global::pathToEXE+fileName).c_str(), "rb");
    if (fp == nullptr || er != 0)
    {
        fprintf(stderr, "Error when trying to open '%s'\n", (Global::pathToEXE+fileName).c_str());
        if (er != 0)
        {
            fprintf(stderr, "fopen_s return value: %d\n", er);
        }
        return AL_NONE;
    }
    #else
    fp = fopen(fileName, "rb");
    if (fp == nullptr)
    {
        fprintf(stderr, "Error when trying to open '%s'\n", (Global::pathToEXE+fileName).c_str());
        perror("error");
        return AL_NONE;
    }
    #endif

    int endian = 0; //0 = little
    int bitStream;
    long bytes;
    char array[32768];

    std::vector<char> buf;

    ALuint frequency;
    ALenum format;

    vorbis_info* pInfo;
    OggVorbis_File oggFile;

    ov_open(fp, &oggFile, NULL, 0);

    pInfo = ov_info(&oggFile, -1);

    if (pInfo->channels == 1)
    {
        format = AL_FORMAT_MONO16;
    }
    else
    {
        format = AL_FORMAT_STEREO16;
    }

    frequency = pInfo->rate;

    do
    {
        bytes = ov_read(&oggFile, array, 32768, endian, 2, 1, &bitStream);
        buf.insert(buf.end(), array, array + bytes);
    }
    while (bytes > 0);

    ov_clear(&oggFile);

    ALuint buffer;
    alGenBuffers(1, &buffer);
    alBufferData(buffer, format, &buf[0], (ALsizei)buf.size(), frequency);

    buf.clear();
    buf.shrink_to_fit();

    return buffer;
}

ALuint AudioMaster::loadWAV(const char* fileName)
{
    #ifdef _WIN32
	FILE* fp = nullptr;
	errno_t e = fopen_s(&fp, fileName, "rb");

	if (fp == nullptr || e != 0)
	{
		fprintf(stdout, "Error when trying to open '%s'\n", fileName);
		return AL_NONE;
	}

	char type[4];
	int size, chunkSize;
	short formatType, channels;
	int sampleRate, avgBytesPerSec;
	short bytesPerSample, bitsPerSample;
	int dataSize;

	fread(type, sizeof(char), 4, fp);
	if (type[0] != 'R' ||
		type[1] != 'I' ||
		type[2] != 'F' ||
		type[3] != 'F')
	{
		fprintf(stderr, "No RIFF\n");
		fclose(fp);
		return 0;
	}

	fread(&size, sizeof(int), 1, fp);
	fread(type, sizeof(char), 4, fp);
	if (type[0] != 'W' ||
		type[1] != 'A' ||
		type[2] != 'V' ||
		type[3] != 'E')
	{
		fprintf(stderr, "not WAVE\n");
		fclose(fp);
		return 0;
	}

	fread(type, sizeof(char), 4, fp);
	if (type[0] != 'f' ||
		type[1] != 'm' ||
		type[2] != 't' ||
		type[3] != ' ')
	{
		fprintf(stderr, "not fmt\n");
		fclose(fp);
		return 0;
	}

	fread(&chunkSize,      sizeof(int  ), 1, fp);
	fread(&formatType,     sizeof(short), 1, fp);
	fread(&channels,       sizeof(short), 1, fp);
	fread(&sampleRate,     sizeof(int  ), 1, fp);
	fread(&avgBytesPerSec, sizeof(int  ), 1, fp);
	fread(&bytesPerSample, sizeof(short), 1, fp);
	fread(&bitsPerSample,  sizeof(short), 1, fp);

	fread(type, sizeof(char), 4, fp);
	if (type[0] != 'd' ||
		type[1] != 'a' ||
		type[2] != 't' ||
		type[3] != 'a')
	{
		fprintf(stderr, "Missing DATA\n");
		fclose(fp);
		return 0;
	}

	fread(&dataSize, sizeof(int), 1, fp);

	unsigned char* buf = new unsigned char[dataSize]; INCR_NEW("UNSIGNED_CHAR_ARRARY");
	fread(buf, sizeof(char), dataSize, fp);



	ALuint buffer;
	ALuint frequency = sampleRate;
	ALenum format = 0;

	alGenBuffers(1, &buffer);

	switch (bitsPerSample)
	{
	case 8:
		switch (channels)
		{
		case 1:
			format = AL_FORMAT_MONO8;
            //printf("MONO 8\n");
			break;

		case 2:
			format = AL_FORMAT_STEREO8;
            //printf("STER 8\n");
			break;

		default:
			fprintf(stderr, "unknown sound format\n");
		}
		break;

	case 16:
		switch (channels)
		{
		case 1:
			format = AL_FORMAT_MONO16;
            //printf("MONO 16\n");
			break;

		case 2:
			format = AL_FORMAT_STEREO16;
            //printf("STER 16\n");
			break;

		default:
			fprintf(stderr, "unknown sound format\n");
		}
		break;

	default:
		fprintf(stderr, "unknown sound format\n");
		break;
	}

	alBufferData(buffer, format, buf, dataSize, frequency);

	delete[] buf; INCR_DEL("UNSIGNED_CHAR_ARRARY");

	fclose(fp);

	return buffer;
    #else
    return AL_NONE;
    #endif
}

void AudioMaster::cleanUp()
{
    AudioPlayer::deleteSources();
    AudioPlayer::deleteBuffersSFX();

    alcMakeContextCurrent(nullptr);
    alcDestroyContext(AudioMaster::context);
    alcCloseDevice(AudioMaster::device);
}
