/*
 * =====================================================================================
 *
 *       Filename: animation.cpp
 *        Created: 06/20/2016 19:56:07
 *  Last Modified: 06/23/2016 01:04:07
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

#include "animation.hpp"
#include <FL/fl_ask.H>

bool Animation::ActionValid()
{
    if(m_Action >= m_AnimationFrameV2D.size()){
        fl_alert("internal error in Animation::Draw(): invalid action code");
        return false;
    }

    if(m_AnimationFrameV2D[m_Action].size() == 0){
        fl_alert("internal error in Animation::Draw(): current action is invalid");
        return false;
    }

    return true;
}

bool Animation::DirectionValid()
{
    if(!ActionValid()){ return false; }

    if(m_Direction >= m_AnimationFrameV2D[m_Action].size()){
        fl_alert("internal error in Animation::Draw(): invalid direction code");
        return false;
    }

    if(m_AnimationFrameV2D[m_Action][m_Direction].size() == 0){
        fl_alert("internal error in Animation::Draw(): current direction is invalid");
        return false;
    }

    return true;
}

bool Animation::FrameValid()
{
    // 1. check action
    if(!(ActionValid() && DirectionValid())){ return false; }

    // 2. ok check frame
    if(m_Frame >= m_AnimationFrameV2D[m_Action][m_Direction].size()){
        fl_alert("internal error in Animation::Draw(): invalid frame index");
        return false;
    }

    auto &rstFrame = m_AnimationFrameV2D[m_Action][m_Direction][m_Frame];
    for(size_t nIndex = 0; nIndex < 2; ++nIndex){
        if(rstFrame[nIndex].Image == nullptr){
            if(rstFrame[nIndex].ImageName == ""){
                fl_alert("internal error in Animation::Draw(): invalid image name");
                return false;
            }

            rstFrame[nIndex].Image = Fl_Shared_Image::get(rstFrame[nIndex].ImageName.c_str());
        }

        if(rstFrame[nIndex].Image == nullptr){
            fl_alert("internal error in Animation::Draw(): invalid image file name: %s", rstFrame[nIndex].ImageName.c_str());
            return false;
        }
    }
    return true;
}

void Animation::Draw(int nX, int nY)
{
    if(FrameValid()){
        auto &rstFrame = m_AnimationFrameV2D[m_Action][m_Direction][m_Frame];
        rstFrame[1].Image->draw(nX + rstFrame[1].DX, nY + rstFrame[1].DY);    // shadow
        rstFrame[0].Image->draw(nX + rstFrame[0].DX, nY + rstFrame[0].DY);    // body
    }
}

void Animation::Update()
{
    if(FrameValid()){
        m_Frame = ((m_Frame + 1) % m_AnimationFrameV2D[m_Action][m_Direction][m_Frame].size());
    }
}

bool Animation::ResetAction(uint32_t nAction)
{
    if(nAction < 16){
        m_Frame = 0;
        m_Action = nAction;
    }

    return true;
}

bool Animation::ResetDirection(uint32_t nDirection)
{
    if(nDirection < 8){
        m_Frame = 0;
        m_Direction = nDirection;
    }

    return true;
}

int Animation::AnimationW(uint32_t nAction, uint32_t nDirection)
{
    if(!(nAction < 16 && nDirection < 8)){ return 0; }


    xxxxxxx
}
