/*
 * =====================================================================================
 *
 *       Filename: podmonitorwindow.hpp
 *        Created: 11/20/2020 21:49:14
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
#include <FL/Fl_Double_Window.H>

class PodMonitorWindow: public Fl_Double_Window
{
    public:
        PodMonitorWindow()
            : Fl_Double_Window(0, 0, 100, 100)
        {}
};
