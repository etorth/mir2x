#include <cstring>
#include "hexstr.hpp"
#include "magicdb.hpp"
#include "sdldevice.hpp"

extern SDLDevice *g_sdlDevice;
std::tuple<SDL_Texture *, int, int> MagicDB::retrieve(uint32_t key, int frame)
{
    if(const auto p = innLoad(key); p && frame >= 0 && frame < to_d(p->frameList.size())){
        return {p->frameList[frame], p->dx, p->dy};
    }
    return {nullptr, 0, 0};
}

int MagicDB::frameCount(uint32_t key)
{
    if(const auto p = innLoad(key)){
        return to_d(p->frameList.size());
    }
    return 0;
}

int MagicDB::frameCount(const MagicGfxEntry *gfxEntry, int dir)
{
    fflassert(gfxEntry);
    fflassert(gfxEntry->frameCount >= 0, gfxEntry->frameCount);
    fflassert(dir >= 0 && dir < gfxEntry->gfxDirType, dir, gfxEntry->gfxDirType);

    if(gfxEntry->gfxID == SYS_U32NIL){
        return gfxEntry->frameCount;
    }

    // some directions have no APNG, they are not drawn
    // then the entry must give frameCount explicitly
    const auto seqFrameCount = frameCount(seqID(gfxEntry->gfxID, dir));
    if(gfxEntry->frameCount > 0){
        fflassert(seqFrameCount == 0 || gfxEntry->frameCount <= seqFrameCount, gfxEntry->gfxID, dir, gfxEntry->frameCount, seqFrameCount);
        return gfxEntry->frameCount;
    }

    fflassert(seqFrameCount > 0, gfxEntry->gfxID, dir);
    return seqFrameCount;
}

std::tuple<int, int> MagicDB::head(uint32_t key)
{
    if(const auto p = innLoad(key)){
        return {p->headX, p->headY};
    }
    return {0, 0};
}

std::optional<std::tuple<_MagicStoreElement, size_t>> MagicDB::loadResource(uint32_t key)
{
    char keyString[16];
    std::vector<uint8_t> dataBuf;

    if(const auto fileName = m_zsdbPtr->decomp(hexstr::to_string<uint32_t, 4>(key, keyString, true), 8, &dataBuf); fileName && (std::strlen(fileName) >= 26)){
        if(auto anim = g_sdlDevice->loadPNGAnimation(dataBuf.data(), dataBuf.size()); !anim.empty()){
            _MagicStoreElement element
            {
                .dx = hexstr::to_2comp(fileName +  8, 3),
                .dy = hexstr::to_2comp(fileName + 11, 3),

                .headX = hexstr::to_2comp(fileName + 14, 3),
                .headY = hexstr::to_2comp(fileName + 17, 3),

                .tailX = hexstr::to_2comp(fileName + 20, 3),
                .tailY = hexstr::to_2comp(fileName + 23, 3),
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

void MagicDB::freeResource(_MagicStoreElement &element)
{
    for(auto texPtr: element.frameList){
        SDL_DestroyTexture(texPtr);
    }
    element.frameList.clear();
}
