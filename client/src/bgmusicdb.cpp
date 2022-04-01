#include "clientargparser.hpp"
#include "bgmusicdb.hpp"
#include "hexstr.hpp"

extern ClientArgParser *g_clientArgParser;
std::optional<std::tuple<BGMusicElement, size_t>> BGMusicDB::loadResource(uint32_t key)
{
    if(g_clientArgParser->disableAudio){
        return {};
    }

    char bgmIndexString[8];
    std::vector<uint8_t> bgmDataBuf;

    if(!m_zsdbPtr->decomp(hexstr::to_string<uint32_t, 4>(key, bgmIndexString, true), 8, &bgmDataBuf)){
        return {};
    }

    if(bgmDataBuf.empty()){
        return {};
    }

    Mix_Music *musicPtr = nullptr;
    if(auto rwOpsPtr = SDL_RWFromConstMem(bgmDataBuf.data(), bgmDataBuf.size())){
        musicPtr = Mix_LoadMUS_RW(rwOpsPtr, SDL_TRUE);
    }

    if(!musicPtr){
        return {};
    }

    return std::make_tuple(BGMusicElement
    {
        .music = musicPtr,
    }, 1);
}
