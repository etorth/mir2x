#pragma once
#include <variant>
#include <functional>
#include <SDL2/SDL.h>
#include "widget.hpp"
#include "colorf.hpp"

// check drawTextureEx comments for how image flip/rotation works
//
//      ---H-->
//
//    x--y | y--x
//    |  | | |  |    |    Vflip = Hflip + 180
//    +--+ | +--+    |    Hflip = Vflip + 180
// --------+-------  V    Hflip + Vflip = 180
//    +--+ | +--+    |
//    |  | | |  |    v
//    x--y | y--x

// top-left corners of image and widget are aligned
// widget uses original image width/height if widget size is given by {}, otherwise rescaling applied
class ImageBoard: public Widget
{
    protected:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarSizeOpt w = std::nullopt; // {} means image width , otherwise rescale the image
            Widget::VarSizeOpt h = std::nullopt; // {} means image height, otherwise rescale the image

            Widget::VarTexLoadFunc texLoadFunc = nullptr;

            bool hflip  = false;
            bool vflip  = false;
            int  rotate = 0;

            Widget::VarU32 modColor = colorf::WHITE_A255;
            Widget::VarBlendMode blendMode = SDL_BLENDMODE_BLEND;

            Widget::WADPair parent {};
        };

    private:
        Widget::VarSizeOpt m_varW;
        Widget::VarSizeOpt m_varH;

    private:
        Widget::VarU32 m_varColor;
        Widget::VarBlendMode m_varBlendMode;

    private:
        Widget::VarTexLoadFunc m_loadFunc;

    private:
        std::pair<bool, int> m_xformPair;

    private:
        bool &m_hflip  = m_xformPair.first;
        int  &m_rotate = m_xformPair.second;

    public:
        ImageBoard(ImageBoard::InitArgs);

    public:
        void drawDefault(Widget::ROIMap) const override;

    public:
        void setColor(Widget::VarU32 color)
        {
            m_varColor = std::move(color);
        }

        void setLoadFunc(VarTexLoadFunc func)
        {
            m_loadFunc = std::move(func);
        }

    public:
        void setFlipRotate(bool hflip, bool vflip, int rotate)
        {
            m_xformPair = getHFlipRotatePair(hflip, vflip, rotate);
        }

    private:
        static std::pair<bool, int> getHFlipRotatePair(bool hflip, bool vflip, int rotate)
        {
            if     (hflip && vflip) return {false, (((rotate + 2) % 4) + 4) % 4};
            else if(hflip         ) return { true, (((rotate    ) % 4) + 4) % 4};
            else if(         vflip) return { true, (((rotate + 2) % 4) + 4) % 4};
            else                    return {false, (((rotate    ) % 4) + 4) % 4};
        }

    public:
        SDL_Texture *getTexture() const
        {
            return Widget::evalTexLoadFunc(m_loadFunc, this);
        }
};
