#include "sdldevice.hpp"
#include "pngtexoffdb.hpp"

extern SDLDevice *g_sdlDevice;
std::tuple<SDL_Texture *, int, int> PNGTexOffDB::retrieve(uint32_t key, int frame)
{
    if(const auto p = innLoad(key); p && frame >= 0 && frame < to_d(p->frameList.size())){
        return {p->frameList[frame], p->dx, p->dy};
    }
    return {nullptr, 0, 0};
}

std::optional<std::tuple<PNGTexOffElement, size_t>> PNGTexOffDB::loadResource(uint32_t key)
{
    char keyString[16];
    std::vector<uint8_t> dataBuf;

    if(const auto fileName = m_zsdbPtr->decomp(hexstr::to_string<uint32_t, 4>(key, keyString, true), 8, &dataBuf); fileName && (std::strlen(fileName) >= 14)){
        //
        // [0 ~ 7] [8 ~ 10] [11 ~ 13]
        //  <KEY>    <DX>     <DY>
        //
        //   KEY: 4 bytes
        //   DX : 12-bit two's complement
        //   DY : 12-bit two's complement

        if(auto anim = g_sdlDevice->loadPNGAnimation(dataBuf.data(), dataBuf.size()); !anim.empty()){
            PNGTexOffElement element
            {
                .dx = hexstr::to_2comp(fileName +  8, 3),
                .dy = hexstr::to_2comp(fileName + 11, 3),
            };

            element.frameList.reserve(anim.size());
            for(const auto &[texPtr, _]: anim){
                element.frameList.push_back(texPtr);
            }

            const auto weight = element.frameList.size();
            return std::make_tuple(std::move(element), weight);
        }
    }
    return {};
}

void PNGTexOffDB::freeResource(PNGTexOffElement &element)
{
    for(auto texPtr: element.frameList){
        SDL_DestroyTexture(texPtr);
    }
    element.frameList.clear();
}
