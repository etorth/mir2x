#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "clientargparser.hpp"
#include "soundeffecthandle.hpp"

extern ClientArgParser *g_clientArgParser;
SoundEffectHandle::~SoundEffectHandle()
{
    if(g_clientArgParser->disableAudio){
        return;
    }

    if(this->chunk){
        Mix_FreeChunk(this->chunk);
        this->chunk = nullptr;
    }
}
