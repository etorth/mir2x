/*
 * =====================================================================================
 *
 *       Filename: commandinput.cpp
 *        Created: 06/04/2017 13:01:35
 *  Last Modified: 06/04/2017 13:31:15
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

#include "commandinput.hpp"
#include "commandwindow.hpp"

int CommandInput::handle(int nEvent)
{
    switch(nEvent){
        case FL_KEYBOARD:
            {
                switch(Fl::event_key()){
                    case FL_Enter:
                        {
                            if(m_Window){
                                m_Window->AddLog(0, ">>", value());
                                return 1;
                            }
                            break;
                        }
                    default:
                        {
                            break;
                        }
                }
            }
        default:
            {
                break;
            }

    }
    return Fl_Multiline_Input::handle(nEvent);
}
