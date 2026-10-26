#include <algorithm>
#include "fflerror.hpp"
#include "emojidb.hpp"
#include "sdldevice.hpp"

extern SDLDevice *g_sdlDevice;
EmojiElement EmojiDB::retrieveElement(uint32_t key, size_t frame)
{
    const auto p = innLoad(key, [this]{ return createFallback(); }); // never fail
    EmojiElement element
    {
        .h1 = p->h1,
        .frameCount = p->animation.size(),
    };

    if(frame < p->animation.size()){
        element.delay = to_uz(std::max<int>(1, p->animation[frame].second));
        element.texture = p->animation[frame].first;
    }
    return element;
}

std::optional<std::tuple<_EmojiStoreElement, size_t>> EmojiDB::loadResource(uint32_t key)
{
    char keyString[16];
    std::vector<uint8_t> dataBuf;

    if(auto fileName = m_zsdbPtr->decomp(hexstr::to_string<uint32_t, 4>(key, keyString, true), 8, &dataBuf); fileName && (std::strlen(fileName) >= 11)){
        if(auto anim = g_sdlDevice->loadPNGAnimation(dataBuf.data(), dataBuf.size()); !anim.empty()){
            return std::make_tuple(_EmojiStoreElement
            {
                .h1 = hexstr::to_2comp(fileName + 8, 3),
                .animation = std::move(anim),
            }, 1);
        }
    }
    return {};
}

void EmojiDB::freeResource(_EmojiStoreElement &element)
{
    for(auto &[tex, _]: element.animation){
        SDL_DestroyTexture(tex);
    }
    element.animation.clear();
}

_EmojiStoreElement *EmojiDB::createFallback()
{
    if(m_fallback.animation.empty()){
        constexpr uint8_t emojiDefData[]
        {
            #embed "emojidef.png"
        };

        if(auto anim = g_sdlDevice->loadPNGAnimation(std::data(emojiDefData), std::size(emojiDefData)); !anim.empty()){
            m_fallback = _EmojiStoreElement
            {
                .h1 = std::max<int>(1, anim.front().first->h * 4 / 5),
                .animation = std::move(anim),
            };
        }
        else{
            throw fflpanic("failed to load emojidef.png");
        }
    }
    return &m_fallback;
}
