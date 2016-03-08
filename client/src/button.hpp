/*
 * =====================================================================================
 *
 *       Filename: button.hpp
 *        Created: 08/21/2015 04:12:57
 *  Last Modified: 03/07/2016 23:14:51
 *
 *    Description: Button, texture id should be baseID + [0, 1, 2]
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
#include "widget.hpp"

class Button: public Widget
{
    public:
        Button(int, int, std::function<void()>,
                std::function<bool(uint32_t, int &, int &)>);
       ~Button();

    public:
        void Draw(std::function<void(uint32_t, int, int)>);
        void Update(uint32_t);
        bool ProcessEvent(SDL_Event &);

    private:
        // 0: normal
        // 1: on
        // 2: pressed
        uint32_t               m_TextureID;
        int                    m_State;
        std::function<void()>  m_OnClick;
        int                    m_MS;
};
