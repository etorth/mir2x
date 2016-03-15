/*
 * =====================================================================================
 *
 *       Filename: imebase.hpp
 *        Created: 03/13/2016 19:17:48
 *  Last Modified: 03/15/2016 00:45:23
 *
 *    Description: input method engine
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

class IMEBase: public Widget
{
    public:
        IMEBase();
        virtual ~IMEBase();

    protected:
        bool InHeadBox(int, int);
        bool InOptionBox(int, int, int);

        int OptionCount();
        void BindCursorTokenBox(int, int);

    public:
        bool ProcessEvent(const SDL_Event &);

    private:
        int m_HotOption;
};
