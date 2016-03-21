/*
 * =====================================================================================
 *
 *       Filename: inputwidget.hpp
 *        Created: 03/17/2016 00:16:52
 *  Last Modified: 03/20/2016 23:53:55
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
        InputWidget() = default;
        virtual ~InputWidget() = default;

    public:
        virtual void InsertInfo(const char *) = 0;
};
