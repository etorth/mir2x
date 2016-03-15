/*
 * =====================================================================================
 *
 *       Filename: imebase.cpp
 *        Created: 03/13/2016 19:37:04
 *  Last Modified: 03/15/2016 00:45:06
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

#include "imebase.hpp"

//  +------------------------------------- input string
//  |
//  |                +-------------------- page up
//  |                |
//  |                |   +---------------- page down
//  |                |   |
//  |                |   |   +------------ configure button
//  |                |   |   |
//  v                v   v   v
// +---------------+---+---+---+
// |wo|            | < | > | S |
// +---------------+---+---+---+
// |x. wo                      | <-------- default, selected by space key
// |1. wo                      | <---+
// |2. wo                      |     |
// |3. wo                      |     |
// |4. wo                      |     |
// |5. wo                      |     |
// |6. wo                      |     |
// |7. wo                      |     |
// |8. wo                      |     |
// |9. wo                      | <---+---- options, selecetd by key 1 ~ 9
// +---------------------------+

bool IMEBase::ProcessEvent(const SDL_Event &rstEvent)
{
    switch(rstEvent.type){
        case SDL_MOUSEMOTION:
            {
                if(In(rstEvent.motion.x, rstEvent.motion.y)){
                    if(InHeadBox(rstEvent.motion.x, rstEvent.motion.y)){
                    }
                    for(int nIndex = 1; nIndex <= OptionCount(); ++nIndex){
                        if(InOptionBox(nIndex, rstEvent.motion.x, rstEvent.motion.y)){
                            //
                            m_HotOption = nIndex;
                            return true;
                        }
                    }
                }

                break;
            }
        case SDL_MOUSEBUTTONUP:
            {
                if(In(rstEvent.motion.x, rstEvent.motion.y)){
                    for(int nIndex = 1; nIndex <= OptionCount(); ++nIndex){
                        if(InOptionBox(nIndex, rstEvent.motion.x, rstEvent.motion.y)){
                            //
                            m_HotOption = nIndex;
                            return true;
                        }
                    }
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
