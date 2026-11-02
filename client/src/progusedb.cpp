#include "progusedb.hpp"
#include "sdldevice.hpp"

extern SDLDevice *g_sdlDevice;
std::optional<std::tuple<ProgUseElement, size_t>> ProgUseDB::loadResource(uint32_t key)
{
    char keyString[16];
    if(std::vector<uint8_t> dataBuf; m_zsdbPtr->decomp(hexstr::to_string<uint32_t, 4>(key, keyString, true), 8, &dataBuf)){
        if(auto anim = g_sdlDevice->loadPNGAnimation(dataBuf.data(), dataBuf.size()); !anim.empty()){
            const auto weight = anim.size();
            return std::make_tuple(ProgUseElement
            {
                .frameList = std::move(anim),
            }, weight);
        }
    }
    return {};
}

void ProgUseDB::freeResource(ProgUseElement &element)
{
    for(auto &[texPtr, _]: element.frameList){
        if(texPtr){
            SDL_DestroyTexture(texPtr);
        }
    }
    element.frameList.clear();
}
