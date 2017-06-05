/*
 * =====================================================================================
 *
 *       Filename: commandinput.cpp
 *        Created: 06/04/2017 13:01:35
 *  Last Modified: 06/04/2017 17:21:17
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
                    case FL_Up:
                        {
                            // to check history
                            break;
                        }
                    case FL_Enter:
                        {
                            if(m_Window){
                                m_Window->AddLog(0, "> ", value());

                                auto stCallResult = m_Window->LuaModule()->script(value(), [](lua_State *, sol::protected_function_result stResult){
                                    // default handler
                                    // do nothing and let the call site handle the errors
                                    return stResult;
                                });

                                if(stCallResult.valid()){
                                    // default nothing printed
                                    // we can put information here to show call succeeds
                                }else{
                                    sol::error stError = stCallResult;
                                    m_Window->AddLog(2, ">>> ", stError.what());
                                }

                                value("");

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
