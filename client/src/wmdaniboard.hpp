/*
 * =====================================================================================
 *
 *       Filename: wmdaniboard.hpp
 *        Created: 07/20/2017 00:34:13
 *    Description:
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *
 * =====================================================================================
 */

#pragma once
#include "texaniboard.hpp"
class WMDAniBoard: public TexAniBoard
{
    public:
        WMDAniBoard(int x, int y, Widget * pwidget = nullptr, bool autoDelete = false)
            : TexAniBoard(x, y, 0X04000010, 10, 8, true, true, pwidget, autoDelete)
        {}
};
