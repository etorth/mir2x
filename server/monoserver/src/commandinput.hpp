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
        CommandWindow *m_Window;

    private:
        // to support history scan
        // int m_InputListPos;
        // std::vector<std::string> m_InputList;

    public:
        CommandInput(int nX, int nY, int nW, int nH, const char *pLabel = nullptr)
            : Fl_Multiline_Input(nX, nY, nW, nH, pLabel)
            , m_Window(nullptr)
        {}

    public:
        void Bind(CommandWindow *pWindow)
        {
            m_Window = pWindow;
        }

    public:
        int handle(int);
};
