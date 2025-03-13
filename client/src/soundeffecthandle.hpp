#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <vector>

struct SoundEffectHandle
{
    Mix_Chunk *chunk = nullptr;
    std::vector<uint8_t> chunkFileData;

    ~SoundEffectHandle();
};
