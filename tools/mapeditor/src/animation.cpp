/*
 * =====================================================================================
 *
 *       Filename: animation.cpp
 *        Created: 06/20/2016 19:56:07
 *  Last Modified: 06/20/2016 21:59:06
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

    for(size_t nIndex = 0; nIndex < 2; ++nIndex){
        if(m_AnimationFrameV2D[m_Action][m_Frame][nIndex].Image == nullptr){
            if(m_AnimationFrameV2D[m_Action][m_Frame][nIndex].ImageName == ""){
                fl_alert("internal error in Animation::Draw(): invalid image name");
                return false;
            }

            m_AnimationFrameV2D[m_Action][m_Frame][nIndex].Image = Fl_Share_Image::get(m_AnimationFrameV2D[m_Action][m_Frame][nIndex].ImageName.c_str());
        }

        if(m_AnimationFrameV2D[m_Action][m_Frame][nIndex].Image == nullptr){
            fl_alert("internal error in Animation::Draw(): invalid image file name: %s", m_AnimationFrameV2D[m_Action][m_Frame][nIndex].ImageName.c_str());
            return false;
        }

        if(m_AnimationFrameV2D[m_Action][m_Frame].Image == nullptr){
            if(m_AnimationFrameV2D[m_Action][m_Frame].ImageName == ""){
                fl_alert("internal error in Animation::Draw(): invalid image name");
                return false;
            }

            m_AnimationFrameV2D[m_Action][m_Frame].Image = Fl_Share_Image::get(m_AnimationFrameV2D[m_Action][m_Frame].ImageName.c_str());
        }

        if(m_AnimationFrameV2D[m_Action][m_Frame].Image == nullptr){
            fl_alert("internal error in Animation::Draw(): invalid image file name: %s", m_AnimationFrameV2D[m_Action][m_Frame].ImageName.c_str());
            return false;
        }


    }
    return true;
}

void Animation::Draw()
{
    // ok now we are legal to draw it
    m_AnimationFrameV2D[m_Action][m_Frame].Image->draw(m_X + m_AnimationFrameV2D[m_Action][m_Frame].DX, m_Y + m_AnimationFrameV2D[m_Action][m_Frame].DY);
}
