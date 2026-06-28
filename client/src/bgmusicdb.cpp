#include "clientargparser.hpp"
#include "bgmusicdb.hpp"
#include "sdldevice.hpp"
#include "hexstr.hpp"

extern ClientArgParser *g_clientArgParser;
extern SDLDevice *g_sdlDevice;

std::optional<std::tuple<BGMusicElement, size_t>> BGMusicDB::loadResource(uint32_t key)
{
    if(g_clientArgParser->disableAudio){
        return {};
    }

    char bgmIndexString[16];
    std::vector<uint8_t> bgmDataBuf;

    if(!m_zsdbPtr->decomp(hexstr::to_string<uint32_t, 4>(key, bgmIndexString, true), 8, &bgmDataBuf)){
        return {};
    }

    if(bgmDataBuf.empty()){
        return {};
    }

    // stream from in-memory buffer; do not predecode so large music files don't fully load
    // closeio=true so MIX_DestroyAudio() will close the iostream
    // the underlying bgmDataBuf must outlive the MIX_Audio (kept in musicFileData)
    MIX_Audio *musicPtr = nullptr;
    if(auto ioStream = SDL_IOFromConstMem(bgmDataBuf.data(), bgmDataBuf.size())){
        musicPtr = MIX_LoadAudio_IO(g_sdlDevice->getMixer(), ioStream, false, true);
    }

    if(!musicPtr){
        return {};
    }

    return std::make_tuple(BGMusicElement
    {
        .music = musicPtr,
        .musicFileData = std::move(bgmDataBuf), // vector class guarantees .data() get preserved
    }, 1);
}

void BGMusicDB::freeResource(BGMusicElement &element)
{
    if(g_clientArgParser->disableAudio){
        return;
    }

    if(element.music){
        MIX_DestroyAudio(element.music);
        element.music = nullptr;
        std::vector<uint8_t>().swap(element.musicFileData);
    }
}
