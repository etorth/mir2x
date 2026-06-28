#pragma once
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <vector>

struct SoundEffectHandle
{
    MIX_Audio *audio = nullptr;
    std::vector<uint8_t> chunkFileData;

    ~SoundEffectHandle();
};
