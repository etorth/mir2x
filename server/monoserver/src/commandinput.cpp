/*
 * =====================================================================================
 *
 *       Filename: commandinput.cpp
 *        Created: 06/04/2017 13:01:35
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
                            // if last char is escape as ``\"
                            // don't commit the command for execution
                            std::string szCommandStr = value() ? value() : "";
                            if(true
                                    && !szCommandStr.empty()
                                    &&  szCommandStr.back() == '\\'){
                                return Fl_Multiline_Input::handle(nEvent);
                            }

                            if(true
                                    && m_Window
                                    && m_Window->GetTaskHub()
                                    && m_Window->GetLuaModule()){

                                // 1. print current command for echo
                                //    should give method to disable the echo

                                // won't use CommandWindow::AddLog() directly
                                // use MonoServer::AddCWLog() for thread-safe access
                                int nCWID = m_Window->GetLuaModule()->CWID();

                                // we echo the command to the command window
                                // enter in command string will be commit to lua machine
                                // but for echo we need to remove it
                                size_t nCurrLoc = 0;
                                while(nCurrLoc < szCommandStr.size()){
                                    auto nEnterLoc = szCommandStr.find_first_of('\n', nCurrLoc);
                                    if(true
                                            && (nEnterLoc >= nCurrLoc)
                                            && (nEnterLoc != std::string::npos)){

                                        // we do find an enter
                                        // remove the enter and print it

                                        extern MonoServer *g_MonoServer;
                                        g_MonoServer->addCWLog(nCWID, 0, "> ", szCommandStr.substr(nCurrLoc, nEnterLoc - nCurrLoc).c_str());

                                        nCurrLoc = nEnterLoc + 1;
                                    }else{
                                        // can't find a enter
                                        // we done here for the whole string

                                        extern MonoServer *g_MonoServer;
                                        g_MonoServer->addCWLog(nCWID, 0, "> ", szCommandStr.substr(nCurrLoc).c_str());
                                        break;
                                    }
                                }

                                // 2. put a task in the LuaModule::TaskHub
                                //    and return immediately for current thread
                                m_Window->GetTaskHub()->Add([this, nCWID, szCommandStr]()
                                {
                                    auto stCallResult = m_Window->GetLuaModule()->GetLuaState().script(szCommandStr.c_str(),
                                    [](lua_State *, sol::protected_function_result stResult)
                                    {
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
                                        std::stringstream stErrorStream(stError.what());

                                        // need to handle here
                                        // error message could be more than one line

                                        std::string szErrorLine;
                                        while(std::getline(stErrorStream, szErrorLine, '\n')){
                                            extern MonoServer *g_MonoServer;
                                            g_MonoServer->addCWLog(nCWID, 2, ">>> ", szErrorLine.c_str());
                                        }
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
