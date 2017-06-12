/*
 * =====================================================================================
 *
 *       Filename: commandinput.cpp
 *        Created: 06/04/2017 13:01:35
 *  Last Modified: 06/11/2017 18:35:22
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

#include "monoserver.hpp"
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
                            if(true
                                    && m_Window
                                    && m_Window->GetTaskHub()
                                    && m_Window->GetLuaModule()){

                                // 1. print current command for echo
                                //    should give method to disable the echo

                                // won't use CommandWindow::AddLog() directly
                                // use MonoServer::AddCWLog() for thread-safe access
                                int nCWID = m_Window->GetLuaModule()->CWID();

                                extern MonoServer *g_MonoServer;
                                g_MonoServer->AddCWLog(nCWID, 0, "> ", value());

                                // 2. put a task in the LuaModule::TaskHub
                                //    and return immediately for current thread

                                std::string szCommandStr = value();
                                m_Window->GetTaskHub()->Add([this, nCWID, szCommandStr](){
                                    auto stCallResult = m_Window->GetLuaModule()->script(szCommandStr.c_str(), [](lua_State *, sol::protected_function_result stResult){
                                        // default handler
                                        // do nothing and let the call site handle the errors
                                        return stResult;
                                    });

                                    if(stCallResult.valid()){
                                        // default nothing printed
                                        // we can put information here to show call succeeds
                                        // or we can unlock the input widget to allow next command
                                    }else{
                                        sol::error stError = stCallResult;

                                        extern MonoServer *g_MonoServer;
                                        g_MonoServer->AddCWLog(nCWID, 2, ">>> ", stError.what());
                                    }
                                });

                                value("");

                                // to inform fltk that we have handled this event
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
