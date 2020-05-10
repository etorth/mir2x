/*
 * =====================================================================================
 *
 *       Filename: commandinput.hpp
 *        Created: 06/03/2017 00:14:31
 *    Description: widget to do command line input
 *                 based on Fl_Multiline_Input
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
#include <Fl/Fl_Multiline_Input.H>

class CommandWindow;
class CommandInput : public Fl_Multiline_Input
{
    private:
        CommandWindow *m_window;

    private:
        // to support history scan
        // int m_inputListPos;
        // std::vector<std::string> m_inputList;

    public:
        CommandInput(int nX, int nY, int nW, int nH, const char *pLabel = nullptr)
            : Fl_Multiline_Input(nX, nY, nW, nH, pLabel)
            , m_window(nullptr)
        {}

    public:
        void Bind(CommandWindow *pWindow)
        {
            m_window = pWindow;
        }

    public:
        int handle(int);
};
