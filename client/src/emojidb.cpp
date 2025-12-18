#include "emojidb.hpp"
#include "sdldevice.hpp"

extern SDLDevice *g_sdlDevice;
SDL_Texture *EmojiDB::retrieve(uint32_t key, int *srcXPtr, int *srcYPtr, int *srcWPtr, int *srcHPtr, int *frameH1Ptr, int *fpsPtr, int *frameCountPtr)
{
    if(auto p = innLoad(key & 0XFFFFFF00)){
        const int texW = SDLDeviceHelper::getTextureWidth(p->texture);
        const int gridXCount = texW / p->frameW;
        const int frameIndex = to_d(key & 0X000000FF);

        if(      srcXPtr){       *srcXPtr = (frameIndex % gridXCount) * p->frameW;     }
        if(      srcYPtr){       *srcYPtr = (frameIndex / gridXCount) * p->frameH;     }
        if(      srcWPtr){       *srcWPtr =                             p->frameW;     }
        if(      srcHPtr){       *srcHPtr =                             p->frameH;     }
        if(   frameH1Ptr){    *frameH1Ptr =                             p->frameH1;    }
        if(       fpsPtr){        *fpsPtr =                             p->fps;        }
        if(frameCountPtr){ *frameCountPtr =                             p->frameCount; }

        return p->texture;
    }
    return nullptr;
}

SDL_Texture *EmojiDB::retrieve(uint8_t emojiSet, uint16_t emojiSubset, uint8_t emojiIndex, int *srcXPtr, int *srcYPtr, int *srcWPtr, int *srcHPtr, int *fpsPtr, int *frameH1Ptr, int *frameCountPtr)
{
    return retrieve((to_u32(emojiSet) << 24) + to_u32(emojiSubset << 8) + to_u32(emojiIndex), srcXPtr, srcYPtr, srcWPtr, srcHPtr, frameH1Ptr, fpsPtr, frameCountPtr);
}

std::optional<std::tuple<EmojiElement, size_t>> EmojiDB::loadResource(uint32_t key)
{
    char keyString[16];
    std::vector<uint8_t> dataBuf;

    if(auto fileName = m_zsdbPtr->decomp(hexstr::to_string<uint32_t, 4>(key, keyString, true), 8, &dataBuf); fileName && (std::strlen(fileName) >= 22)){
        if(auto texPtr = g_sdlDevice->loadPNGTexture(dataBuf.data(), dataBuf.size())){
            return std::make_tuple(EmojiElement
            {
                .frameW     = to_d(hexstr::to_hex<uint16_t, 2>(fileName + 12)),
                .frameH     = to_d(hexstr::to_hex<uint16_t, 2>(fileName + 16)),
                .frameH1    = to_d(hexstr::to_hex<uint16_t, 2>(fileName + 20)),
                .fps        = to_d(hexstr::to_hex< uint8_t, 1>(fileName + 10)),
                .frameCount = to_d(hexstr::to_hex< uint8_t, 1>(fileName +  8)),
                .texture    = texPtr,
            }, 1);
        }
    }
    return {};
}

void EmojiDB::freeResource(EmojiElement &element)
{
    if(element.texture){
        SDL_DestroyTexture(element.texture);
        element.texture = nullptr;
    }
}
