#pragma once
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

class ImageBoard: public Widget
{
    private:
        std::function<SDL_Texture *(const ImageBoard *)> m_loadFunc;

    private:
        std::pair<bool, int> m_xformPair;

    private:
        bool &m_hflip  = m_xformPair.first;
        int  &m_rotate = m_xformPair.second;

    private:
        uint32_t m_color;

    public:
        ImageBoard(dir8_t,
                int,
                int,

                Widget::VarSize,
                Widget::VarSize,

                std::function<SDL_Texture *(const ImageBoard *)>,

                bool = false,
                bool = false,
                int  = 0,

                uint32_t = colorf::WHITE + colorf::A_SHF(0XFF),

                Widget * = nullptr,
                bool     = false);

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        void setColor(uint32_t color)
        {
            m_color = color;
        }

        void setLoadFunc(std::function<SDL_Texture *(const ImageBoard *)> func)
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
};
