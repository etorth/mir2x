/*
 * =====================================================================================
 *
 *       Filename: animation.cpp
 *        Created: 06/20/2016 19:56:07
 *  Last Modified: 06/20/2016 20:17:23
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

void Animation::Draw()
{
    if(m_Action >= m_AnimationFrameV2D.size()){
        fl_alert("internal error in Animation::Draw(): invalid action code");
        exit(0);
    }

    if(m_AnimationFrameV2D[m_Action].size() == 0 || m_Frame >= m_AnimationFrameV2D[m_Action].size()){
        fl_alert("internal error in Animation::Draw(): invalid frame indexing");
        exit(0);
    }

    if(m_AnimationFrameV2D[m_Action][m_Frame].Image == nullptr){

    }
    if(true
            && m_AnimationFrameV2D[m_Action][m_Frame].Image == nullptr
            && m_AnimationFrameV2D[m_Action][m_Frame].ImageName == ""){
        fl_alert("internal error in Animation::Draw(): invalid frame indexing");
        exit(0);
    }








}
