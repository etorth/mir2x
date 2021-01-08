/*
 * =====================================================================================
 *
 *       Filename: quickaccessbutton.hpp
 *        Created: 03/28/2020 05:43:45
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
#include <cstdint>
#include <functional>
#include "widget.hpp"
#include "buttonbase.hpp"

class QuickAccessButton: public ButtonBase
{
    private:
        constexpr static uint32_t m_texID = 0X00000045;

    public:
        QuickAccessButton(const std::function<void()> &, Widget * pwidget = nullptr, bool autoDelete = false);

    public:
        void drawEx(int, int, int, int, int, int) const override;
};
