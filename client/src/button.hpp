/*
 * =====================================================================================
 *
 *       Filename: button.hpp
 *        Created: 08/21/2015 04:12:57
 *  Last Modified: 03/20/2016 20:24:56
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
#include "widget.hpp"

#include <cstdint>
#include <functional>

class Button: public Widget
{
    public:
        Button(uint8_t, uint16_t, int, int, const std::function<void()> &);
        virtual ~Button() = default;

    public:
        void Draw();
        bool ProcessEvent(const SDL_Event &);

    private:
        // 0: normal
        // 1: on
        // 2: pressed
        uint32_t               m_BaseID;
        int                    m_State;
        std::function<void()>  m_OnClick;
};
