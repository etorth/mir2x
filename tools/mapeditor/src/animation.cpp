/*
 * =====================================================================================
 *
 *       Filename: animation.cpp
 *        Created: 06/20/2016 19:56:07
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
#include <limits>
#include <FL/fl_ask.H>
#include "animation.hpp"

bool Animation::ActionValid(uint32_t nAction)
{
    if(nAction >= 16){ return false; }
    if(nAction >= m_AnimationFrameV2D.size()){ return false; }
    if(m_AnimationFrameV2D[nAction].size() == 0){ return false; }

    return true;
}

bool Animation::DirectionValid(uint32_t nAction, uint32_t nDirection)
{
    if(nAction >= 16){ return false; }
    if(nDirection >= 8){ return false; }

    if(!ActionValid(nAction)){ return false; }

    if(nDirection >= m_AnimationFrameV2D[nAction].size()){ return false; }
    if(m_AnimationFrameV2D[nAction][nDirection].size() == 0){ return false; }

    return true;
}

bool Animation::FrameValid(uint32_t nAction, uint32_t nDirection, uint32_t nFrame, bool bShadow)
{
    if(nAction >= 16){ return false; }
    if(nDirection >= 8){ return false; }
    if(nFrame >= 32){ return false; }

    if(!ActionValid(nAction)){ return false; }
    if(!DirectionValid(nAction, nDirection)){ return false; }

    if(nFrame >= m_AnimationFrameV2D[nAction][nDirection].size()){ return false; }

    // then we will check the image pointer, here we always try to load shadow and body
    auto &rstFrame = m_AnimationFrameV2D[nAction][nDirection][nFrame];
    for(size_t nIndex = 0; nIndex < 2; ++nIndex){
        // try to load it if not loaded
        if(!rstFrame[nIndex].Image){
            // 1. can't load, skip
            if(rstFrame[nIndex].BadImageName){ continue; }

            // 2. bad name
            if(rstFrame[nIndex].ImageName == ""){
                rstFrame[nIndex].BadImageName = true;
                continue;
            }

            // 3. try to load it
            //    TODO: problem is if we have a bad ImageName, then we'll do it everytime
            rstFrame[nIndex].Image = Fl_Shared_Image::get(rstFrame[nIndex].ImageName.c_str());

            // 4. if load failed, this is a bad name either
            if(!rstFrame[nIndex].Image){
                rstFrame[nIndex].BadImageName = true;
                continue;
            }
        }
    }

    return rstFrame[bShadow ? 1 : 0].Image != nullptr;
}

void Animation::Draw(int nX, int nY)
{
    // shadow
    if(FrameValid(m_Action, m_Direction, m_Frame, true)){
        auto &rstFrame = m_AnimationFrameV2D[m_Action][m_Direction][m_Frame];
        rstFrame[1].Image->draw(nX + rstFrame[1].DX, nY + rstFrame[1].DY);
    }

    // body
    if(FrameValid(m_Action, m_Direction, m_Frame, false)){
        auto &rstFrame = m_AnimationFrameV2D[m_Action][m_Direction][m_Frame];
        rstFrame[0].Image->draw(nX + rstFrame[0].DX, nY + rstFrame[0].DY);
    }
}

void Animation::Draw(int nX, int nY, std::function<void(Fl_Shared_Image *, int, int)> fnDraw)
{
    // shadow
    if(FrameValid(m_Action, m_Direction, m_Frame, true)){
        auto &rstFrame = m_AnimationFrameV2D[m_Action][m_Direction][m_Frame];
        fnDraw(rstFrame[1].Image, nX + rstFrame[1].DX, nY + rstFrame[1].DY);
    }

    // body
    if(FrameValid(m_Action, m_Direction, m_Frame, false)){
        auto &rstFrame = m_AnimationFrameV2D[m_Action][m_Direction][m_Frame];
        fnDraw(rstFrame[0].Image, nX + rstFrame[0].DX, nY + rstFrame[0].DY);
    }
}

void Animation::Update()
{
    if(FrameValid(false) || FrameValid(true)){
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
    if(ActionValid(nAction) && DirectionValid(nAction, nDirection)){
        int nMinX, nMaxX;
        nMinX = std::numeric_limits<int>::max();
        nMaxX = std::numeric_limits<int>::min();

        auto &rstFrameV = m_AnimationFrameV2D[nAction][nDirection];
        for(auto &rstFrame: rstFrameV){
            for(int nIndex = 0; nIndex < 2; ++nIndex){
                if(rstFrame[nIndex].Valid()){
                    nMinX = (std::min<int>)(nMinX, rstFrame[nIndex].DX);
                    nMaxX = (std::max<int>)(nMaxX, rstFrame[nIndex].DX + rstFrame[nIndex].Image->w());
                }
            }
        }

        return nMaxX - nMinX;
    }

    return 0;
}

int Animation::AnimationH(uint32_t nAction, uint32_t nDirection)
{
    if(ActionValid(nAction) && DirectionValid(nAction, nDirection)){
        int nMinY, nMaxY;
        nMinY = std::numeric_limits<int>::max();
        nMaxY = std::numeric_limits<int>::min();

        auto &rstFrameV = m_AnimationFrameV2D[nAction][nDirection];
        for(auto &rstFrame: rstFrameV){
            for(int nIndex = 0; nIndex < 2; ++nIndex){
                if(rstFrame[nIndex].Valid()){
                    nMinY = (std::min<int>)(nMinY, rstFrame[nIndex].DY);
                    nMaxY = (std::max<int>)(nMaxY, rstFrame[nIndex].DY + rstFrame[nIndex].Image->h());
                }
            }
        }

        return nMaxY - nMinY;
    }

    return 0;
}

bool Animation::ResetFrame(uint32_t nAction, uint32_t nDirection, uint32_t nFrame)
{
    if(FrameValid(nAction, nDirection, nFrame, true) && FrameValid(nAction, nDirection, nFrame, false)){
        m_Action = nAction;
        m_Direction = nDirection;
        m_Frame = nFrame;

        return true;
    }

    return false;
}
