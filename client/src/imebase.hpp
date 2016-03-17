/*
 * =====================================================================================
 *
 *       Filename: imebase.hpp
 *        Created: 03/13/2016 19:17:48
 *  Last Modified: 03/17/2016 00:18:39
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
#include "inputwidget.hpp"
#include <string>

class IMEBase: public Widget
{
    public:
        IMEBase();
        virtual ~IMEBase();

    protected:
        bool InHeadBox(int, int);
        bool InOptionBox(int, int, int);

        int  OptionCount();

        void Draw(int, int, int, int);
        void InsertInfo();

    public:
        bool ProcessEvent(const SDL_Event &);

    private:
        int m_HotOption;

        std::string m_CurrentString;

    protected:
        InputWidget *m_InputWidget;
};
