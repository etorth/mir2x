/*
 * =====================================================================================
 *
 *       Filename: flinc.hpp
 *        Created: 12/06/2018 00:29:57
 *    Description: include all needed FLTK headers
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
#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>
#include <Fl/Fl_Table_Row.H>
#include <Fl/Fl_Text_Buffer.H>
#include <Fl/Fl_Multiline_Input.H>
#include <FL/Fl_Native_File_Chooser.H>

class DisableFlWidget final
{
    private:
        Fl_Widget *m_widget;

    public:
        DisableFlWidget(Fl_Widget *widget)
            : m_widget(widget)
        {
            if(m_widget){
                m_widget->deactivate();
            }
        }

    public:
        ~DisableFlWidget()
        {
            if(m_widget){
                m_widget->activate();
            }
        }
};
