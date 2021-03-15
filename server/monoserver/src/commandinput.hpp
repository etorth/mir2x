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
#include <vector>
#include "flinc.hpp"
#include "threadpool.hpp"

class CommandWindow;
class CommandInput : public Fl_Multiline_Input
{
    private:
        CommandWindow *m_window = nullptr;
        std::unique_ptr<threadPool> m_worker;

    private:
        int m_inputListPos = 0;
        std::vector<std::string> m_inputList;

    public:
        CommandInput(int argX, int argY, int argW, int argH, const char *labelCPtr = nullptr)
            : Fl_Multiline_Input(argX, argY, argW, argH, labelCPtr)
        {}

    public:
        void bind(CommandWindow *winPtr)
        {
            m_window = winPtr;
            m_worker = std::make_unique<threadPool>(1);
        }

    public:
        int handle(int);
};
