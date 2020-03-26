/*
 * =====================================================================================
 *
 *       Filename: editboard.hpp
 *        Created: 03/26/2020 16:15:15
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
#include "widget.hpp"

class editBoard: public Widget
{
    private:
        int m_cursorX;
        int m_cursorY;
};
