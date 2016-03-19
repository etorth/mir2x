/*
 * =====================================================================================
 *
 *       Filename: pinyinime.cpp
 *        Created: 03/13/2016 19:37:04
 *  Last Modified: 03/13/2016 20:22:48
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

#include "pinyinime.hpp"





bool PinyinIME::ProcessEvent(const SDL_Event &rstEvent)
{
    switch(rstEvent.type){
        case SDL_MOUSEMOTION:
            {
                if(In(rstEvent.motion.x, rstEvent.motion.y)){
                    for(int nIndex = 1; nIndex <= OptionCount(); ++nIndex){
                        if(InOptionBox(nIndex, rstEvent.motion.x, rstEvent.motion.y)){
                            //
                            m_HotOption = nIndex;
                            return true;
                        }
                    }else{
                        if(m_DrawOwnSystemCursor){
                            m_ShowSystemCursorCount++;
                        }
                        m_DrawOwnSystemCursor = false;
                    }
                    break;
                }
                case SDL_MOUSEBUTTONDOWN:
                {
                    if(In(rstEvent.button.x, rstEvent.button.y)){
                        m_Focus = true;
                        BindCursorTokenBox(rstEvent.button.x, rstEvent.button.y);
                        ResetShowStartX();
                        return true;
                    }else{
                        m_Focus = false;
                    }
                    break;
                }
                case SDL_KEYDOWN:
                {
                    if(m_Focus){
                        // clear the count
                        m_Ticks = 0;


                        if(m_IME){
                            return m_IME->ProcessEvent(rstEvent);
                        }

                        char chKeyName = SDLKeyEventChar(rstEvent);


                        if(chKeyName != '\0'){
                            m_BindTokenBoxIndex++;
                            m_Content.insert((std::size_t)(m_BindTokenBoxIndex), (std::size_t)1, chKeyName);
                            Compile();
                            ResetShowStartX();
                            return true;
                        }

                        if(rstEvent.key.keysym.sym == SDLK_BACKSPACE){
                            if(m_BindTokenBoxIndex >= 0){
                                m_Content.erase(m_BindTokenBoxIndex, 1);
                                m_BindTokenBoxIndex--;
                                Compile();
                                ResetShowStartX();
                            }
                            return true;
                        }

                        if(rstEvent.key.keysym.sym == SDLK_LEFT){
                            if(m_BindTokenBoxIndex >= 0){
                                m_BindTokenBoxIndex--;
                                ResetShowStartX();
                            }
                            return true;
                        }

                        if(rstEvent.key.keysym.sym == SDLK_RIGHT){
                            if((size_t)(m_BindTokenBoxIndex + 1) < m_Content.size()){
                                m_BindTokenBoxIndex++;
                                ResetShowStartX();
                            }
                            return true;
                        }
                    }
                    break;
                }
                default:
                break;
            }
            return false;
    }

    void PinyinIME::Draw(int nX, int nY1, int nY2, std::function)
    {
        // input method engine is special widget
        // when it's unfocused, it can't be visiable
        //
        if(m_Focus){
        }
    }


    PinyinIME::InsertInfo()
    {
        // insert a utf-8 string to the input widget
        // the input widget should parse the string

        m_InputWidget->InsertInfo(m_CurrentString.c_str());
    }
