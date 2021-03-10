/*
 * =====================================================================================
 *
 *       Filename: imageboard.hpp
 *        Created: 03/25/2020 22:27:45
 *    Description:
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#pragma once
#include <functional>
#include <SDL2/SDL.h>
#include "widget.hpp"
#include "colorf.hpp"

class ImageBoard: public Widget
{
    private:
        std::function<SDL_Texture *(const ImageBoard *)> m_loadFunc;

    private:
        uint32_t m_color;

    public:
        ImageBoard(
                dir8_t dir,
                int x,
                int y,
                int w,
                int h,

                std::function<SDL_Texture *(const ImageBoard *)> loadFunc,

                uint32_t color     = colorf::WHITE + 0XFF,
                Widget *parent     = nullptr,
                bool    autoDelete = false)
            : Widget(dir, x, y, w, h, parent, autoDelete)
            , m_loadFunc(std::move(loadFunc))
            , m_color(color)
        {
            if(!m_loadFunc){
                throw fflerror("invalid texture load function");
            }

            if(auto texPtr = m_loadFunc(this)){
                const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
                if(m_w <= 0){
                    m_w = texW;
                }

                if(m_h <= 0){
                    m_h = texH;
                }
            }
        }

    public:
        ImageBoard(
                dir8_t dir,
                int x,
                int y,

                std::function<SDL_Texture *(const ImageBoard *)> loadFunc,

                uint32_t color     = colorf::WHITE + 0XFF,
                Widget *parent     = nullptr,
                bool    autoDelete = false)
            : ImageBoard(dir, x, y, 0, 0, std::move(loadFunc), color, parent, autoDelete)
        {}

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        void setColor(uint32_t color)
        {
            m_color = color;
        }

    public:
        void setSize     (std::optional<int  >, std::optional<int  >);
        void setSizeRatio(std::optional<float>, std::optional<float>);
};
