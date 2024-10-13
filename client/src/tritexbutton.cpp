#include "colorf.hpp"
#include "sysconst.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "tritexbutton.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

void TritexButton::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    if(auto texPtr = g_progUseDB->retrieve(m_texIDList[getState()])){
        const int offX = m_offset[getState()][0];
        const int offY = m_offset[getState()][1];
        const auto modColor= [this]() -> uint32_t
        {
            if(!active()){
                return colorf::RGBA(128, 128, 128, 255);
            }
            else if(m_alterColor && (getState() != BEVENT_OFF)){
                return colorf::RGBA(255, 200, 255, 255);
            }
            else{
                return colorf::RGBA(255, 255, 255, 255);
            }
        }();

        SDLDeviceHelper::EnableTextureModColor enableColor(texPtr, modColor);
        SDLDeviceHelper::EnableTextureBlendMode enableBlendMode(texPtr, [this]()
        {
            if(m_blinkTime.has_value()){
                const auto offTime = std::get<0>(m_blinkTime.value());
                const auto  onTime = std::get<1>(m_blinkTime.value());

                if(offTime == 0){
                    return SDL_BLENDMODE_ADD;
                }
                else if(onTime == 0){
                    return SDL_BLENDMODE_BLEND;
                }
                else{
                    if(std::fmod(m_accuBlinkTime, offTime + onTime) < offTime){
                        return SDL_BLENDMODE_BLEND;
                    }
                    else{
                        return SDL_BLENDMODE_ADD;
                    }
                }
            }
            else{
                return SDL_BLENDMODE_BLEND;
            }
        }());
        g_sdlDevice->drawTexture(texPtr, dstX + offX, dstY + offY, srcX, srcY, srcW, srcH); // TODO: need to crop src region for offset
    }
}

void TritexButton::initButtonSize()
{
    int maxW = 0;
    int maxH = 0;
    for(const int state: {0, 1, 2}){
        if(m_texIDList[state] != SYS_U32NIL){
            if(auto texPtr = g_progUseDB->retrieve(m_texIDList[state])){
                const auto [texCurrW, texCurrH] = SDLDeviceHelper::getTextureSize(texPtr);
                maxW = std::max<int>(texCurrW, maxW);
                maxH = std::max<int>(texCurrH, maxH);
            }
        }
    }

    // we allow buttons without any valid texture, in that case some extra work
    // can be done for special drawing
    m_w = maxW;
    m_h = maxH;
}
