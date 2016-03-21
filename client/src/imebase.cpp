/*
 * =====================================================================================
 *
 *       Filename: imebase.cpp
 *        Created: 03/13/2016 19:37:04
 *  Last Modified: 03/20/2016 23:58:17
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
#include "supwarning.hpp"

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
                    return true;
                }else{
                    m_Focus = false;
                }
                break;
            }
        case SDL_KEYDOWN:
            {
                break;
            }
        default:
            break;
    }
    return false;
}

void IMEBase::Draw(int nX, int nW, int nY1, int nY2)
{
    // input method engine is special widget
    // when it's unfocused, it can't be visiable
    //
    if(!m_Focus){ return; }
    UNUSED(nX);
    UNUSED(nW);
    UNUSED(nY1);
    UNUSED(nY2);
}


void IMEBase::InsertInfo()
{
    // insert a utf-8 string to the input widget
    // the input widget should parse the string

    m_InputWidget->InsertInfo(m_CurrentString.c_str());
}

bool IMEBase::InHeadBox(int, int)
{
    return false;
}

bool IMEBase::InOptionBox(int, int, int)
{
    return false;
}

int IMEBase::OptionCount()
{
    return 0;
}
