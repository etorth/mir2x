/*
 * =====================================================================================
 *
 *       Filename: inputwidget.hpp
 *        Created: 03/17/2016 00:16:52
 *  Last Modified: 03/31/2016 22:45:14
 *
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

class InputWidget: public Widget
{
    public:
        InputWidget(
                int     nX,
                int     nY,
                int     nW          = 0,
                int     nH          = 0,
                Widget *pWidget     = nullptr,
                bool    bFreeWidget = false) :
            Widget(nX, nY, nW, nH, pWidget, bFreeWidget){}
            
        virtual ~InputWidget() = default;

    public:
        virtual void InsertInfo(const char *) = 0;
};
