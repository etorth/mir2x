#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include "clientargparser.hpp"
#include "soundeffecthandle.hpp"

extern ClientArgParser *g_clientArgParser;
SoundEffectHandle::~SoundEffectHandle()
{
    if(g_clientArgParser->disableAudio){
        return;
    }

    if(this->audio){
        MIX_DestroyAudio(this->audio);
        this->audio = nullptr;
    }
}
