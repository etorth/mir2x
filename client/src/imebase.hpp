/*
 * =====================================================================================
 *
 *       Filename: imebase.hpp
 *        Created: 03/13/2016 19:17:48
 *  Last Modified: 07/26/2017 15:18:09
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
#include <string>
#include "widget.hpp"
#include "inputwidget.hpp"

class IMEBase: public Widget
{
    private:
        int m_HotOption;
        std::string m_CurrentString;

    protected:
        InputWidget *m_InputWidget;

    public:
        IMEBase() = default;

    public:
        ~IMEBase() = default;

    protected:
        bool InHeadBox(int, int);
        bool InOptionBox(int, int, int);

        int  OptionCount();

        void Draw(int, int, int, int);
        void InsertInfo();

    public:
        bool ProcessEvent(const SDL_Event &);
};
