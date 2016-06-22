/*
 * =====================================================================================
 *
 *       Filename: animation.cpp
 *        Created: 06/20/2016 19:56:07
 *  Last Modified: 06/21/2016 11:16:03
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

Animation::Animation()
{
}

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

bool Animation::FrameValid()
{
    // 1. check action
    if(!ActionValid()){ return false; }

    // 2. ok check frame
    if(m_Frame >= m_AnimationFrameV2D[m_Action].size()){
        fl_alert("internal error in Animation::Draw(): invalid frame indexing");
        return false;
    }

    auto &rstFrame = m_AnimationFrameV2D[m_Action][m_Frame];
    for(size_t nIndex = 0; nIndex < 2; ++nIndex){
        if(rstFrame[nIndex].Image == nullptr){
            if(rstFrame[nIndex].ImageName == ""){
                fl_alert("internal error in Animation::Draw(): invalid image name");
                return false;
            }

            rstFrame[nIndex].Image = Fl_Share_Image::get(rstFrame[nIndex].ImageName.c_str());
        }

        if(rstFrame[nIndex].Image == nullptr){
            fl_alert("internal error in Animation::Draw(): invalid image file name: %s", rstFrame[nIndex].ImageName.c_str());
            return false;
        }
    }
    return true;
}

void Animation::Draw()
{
    if(FrameValid()){
        auto &rstFrame = m_AnimationFrameV2D[m_Action][m_Frame];
        rstFrame[1].Image->draw(m_X + rstFrame[1].DX, m_Y + rstFrame[1].DY);    // shadow
        rstFrame[0].Image->draw(m_X + rstFrame[0].DX, m_Y + rstFrame[0].DY);    // body
    }
}

void Animation::Update()
{
    if(FrameValid()){
        m_Frame = ((m_Frame + 1) % m_AnimationFrameV2D[m_Action][m_Frame].size());
    }
}
