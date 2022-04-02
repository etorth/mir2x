#include "soundeffecthandle.hpp"
#include "clientargparser.hpp"
#include "soundeffectdb.hpp"
#include "hexstr.hpp"

extern ClientArgParser *g_clientArgParser;
std::optional<std::tuple<SoundEffectElement, size_t>> SoundEffectDB::loadResource(uint32_t key)
{
    if(g_clientArgParser->disableAudio){
        return {};
    }

    char soundEffectIndexString[16];
    std::vector<uint8_t> soundEffectDataBuf;

    if(!m_zsdbPtr->decomp(hexstr::to_string<uint32_t, 4>(key, soundEffectIndexString, true), 8, &soundEffectDataBuf)){
        return {};
    }

    if(soundEffectDataBuf.empty()){
        return {};
    }

    Mix_Chunk *chunkPtr = nullptr;
    if(auto rwOpsPtr = SDL_RWFromConstMem(soundEffectDataBuf.data(), soundEffectDataBuf.size())){
        chunkPtr = Mix_LoadWAV_RW(rwOpsPtr, SDL_TRUE);
    }

    if(!chunkPtr){
        return {};
    }

    return std::make_tuple(SoundEffectElement
    {
        .handle = std::shared_ptr<SoundEffectHandle>(new SoundEffectHandle
        {
            .chunk = chunkPtr,
            .chunkFileData = std::move(soundEffectDataBuf),
        }),
    }, 1);
}

void SoundEffectDB::freeResource(SoundEffectElement &element)
{
    // check SDL_mixer docmument
    // Mix_FreeMusic() stops music if it's playing, this is blocking when music doing fading out

    if(g_clientArgParser->disableAudio){
        return;
    }

    element.handle.reset();
}
