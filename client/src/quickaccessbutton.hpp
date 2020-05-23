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
#include "widget.hpp"
#include "buttonbase.hpp"

class QuickAccessButton: public 
{
    public:
        QuickAccessButton();

    public:
        void drawEx(int, int, int, int, int, int);
};
