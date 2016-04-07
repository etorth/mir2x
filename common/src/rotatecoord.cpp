/*
 * =====================================================================================
 *
 *       Filename: rotatecoord.cpp
 *        Created: 08/15/2015 04:01:57 PM
 *  Last Modified: 04/06/2016 23:08:27
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

#include <algorithm>

#include "rotatecoord.hpp"
#include "mathfunc.hpp"

bool RotateCoord::Reset(
        int nCenterX, int nCenterY, int nStartX, int nStartY, int nW, int nH)
{
    if(nW > 0 && nH > 0){
        m_CenterX = nCenterX;
        m_CenterY = nCenterY;
        m_StartX  = nStartX;
        m_StartY  = nStartY;
        m_StopX   = nStartX + nW - 1;
        m_StopY   = nStartY + nH - 1;

        if(PointInRectangle(m_CenterX, m_CenterY, m_StartX, m_StartY, nW, nH)){
            m_Distance  = 0;
            m_Direction = 0;
            m_CurrentX  = m_CenterX;
            m_CurrentY  = m_CenterY;
        }else{
            if(true
                    && (m_CenterX - m_CenterY) >= (m_StartX - m_StartY)
                    && (m_CenterX + m_CenterY) <  (m_StopX  + m_StartY)
					&& m_CenterY < m_StartY
              ){
                // 0
                m_Distance  = m_StartY - m_CenterY;
                m_Direction = 0;
                m_CurrentX  = (std::max)(m_StartX, m_CenterX - m_Distance);
                m_CurrentY  = m_StartY;
            }
            if(true
                    && (m_CenterX + m_CenterY) >= (m_StopX + m_StartY)
                    && (m_CenterX - m_CenterY) >  (m_StopX - m_StopY)
					&& m_CenterX > m_StopX
              ){
                // 1
                //
                // hard here
                // when set direction to 0 1 2 it's ok
                // but for 3, need to set the the smallest valid direction
                // since for d 3, there is jumping

                m_Distance = m_CenterX - m_StopX;
                m_CurrentX = m_StopX;
                if(m_StopY >= m_CenterY + m_Distance){
                    // capable for set to d 0
                    m_Direction = 0;
                    m_CurrentY  = m_CenterY + m_Distance;
                }else{
                    // set to d 3
                    m_Direction = 3;
                    m_CurrentY  = (std::max)(m_StartY, m_CenterY - m_Distance);
                }
            }
            if(true
                    && (m_CenterX - m_CenterY) <= (m_StopX  - m_StopY)
                    && (m_CenterX + m_CenterY) >  (m_StartX + m_StopY)
					&& m_CenterY > m_StopY
              ){
                // 2
                m_Distance  = m_CenterY - m_StopY;
                m_Direction = 2;
                m_CurrentX  = (std::min)(m_StopX, m_CenterX + m_Distance);
                m_CurrentY  = m_StopY;
            }
            if(true
                    && (m_CenterX + m_CenterY) <= (m_StartX + m_StopY)
                    && (m_CenterX - m_CenterY) <  (m_StartX - m_StartY)
					&& m_CenterX < m_StartX
              ){
                // 3
                m_Distance  = m_StartX - m_CenterX;
                m_Direction = 1;
				m_CurrentX  = m_StartX;
                m_CurrentY  = (std::min)(m_StopY, m_CenterY + m_Distance);
            }
        }
        CheckOverlap();
        return true;
    }
    return false;
}

void RotateCoord::CheckOverlap()
{
    m_Overlap[0] = PointInSegment(m_CenterY + m_Distance, m_StartY, m_StopY - m_StartY + 1);
    m_Overlap[1] = PointInSegment(m_CenterX + m_Distance, m_StartX, m_StopX - m_StartX + 1);
    m_Overlap[2] = PointInSegment(m_CenterY - m_Distance, m_StartY, m_StopY - m_StartY + 1);
    m_Overlap[3] = PointInSegment(m_CenterX - m_Distance, m_StartX, m_StopX - m_StartX + 1);
}

bool RotateCoord::MoveToNextRound()
{
    // always pick the first valid position following current direction
    if(m_Overlap[0]){
        m_Direction = 0;
        m_CurrentX  = (std::max)(m_StartX, m_CenterX - m_Distance);
        m_CurrentY  = m_CenterY + m_Distance;
        return true;
    }else if(m_Overlap[1]){
        m_Direction = 1;
        m_CurrentX  = m_CenterX + m_Distance;
        m_CurrentY  = (std::min)(m_StopY, m_CenterY + m_Distance);
        return true;
    }else if(m_Overlap[2]){
        m_Direction = 2;
        m_CurrentX  = (std::min)(m_StopX, m_CenterX + m_Distance);
        m_CurrentY  = m_CenterY - m_Distance;
        return true;
    }else if(m_Overlap[3]){
        m_Direction = 3;
		m_CurrentX  = m_CenterX - m_Distance;
        m_CurrentY  = (std::max)(m_StartY, m_CenterY - m_Distance);
        return true;
    }else{
        return false;
    }
}

bool RotateCoord::Forward()
{
    // based on boundary but not current position
    // this is easiest idea
    // every time when we arrival the boundary, check next boundary
    if(m_Distance != 0){
        switch(m_Direction){
            case 0:
                {
                    if(m_CurrentX == (std::min)(m_StopX, m_CenterX + m_Distance)){
                        if(m_Overlap[1]){
                            // 0 and 1 are neighbor, so need to check something
                            int nNewX = m_CenterX + m_Distance;
                            int nNewY = (std::min)(m_StopY, m_CenterY + m_Distance);
                            if(nNewX == m_CurrentX && nNewY == m_CurrentY){
                                nNewY--;
                            }
                            if(PointInRectangle(nNewX, nNewY, m_StartX, m_StartY,
                                        m_StopX - m_StartX + 1, m_StopY - m_StartY + 1)
                              ){
                                m_Direction = 1;
                                m_CurrentX  = nNewX;
                                m_CurrentY  = nNewY;
                                return true;
                            }
                        }
                        if(m_Overlap[2]){
                            // but if we need to jump from 0 to 2
                            // actually nothing needs to be check
                            // just jump to the first valid grid

                            m_CurrentX  = (std::min)(m_StopX, m_CenterX + m_Distance);
                            m_CurrentY  = m_CenterY - m_Distance;
                            m_Direction = 2;
                            return true;

                            // int nNewX = (std::min)(m_StopX, m_CenterX + m_Distance);
                            // int nNewY = m_CenterY - m_Distance;
                            // if(nNewX == m_CurrentX && nNewY == m_CurrentY){
                            //     nNewX--;
                            // }
                            // if(PointInRectangle(nNewX, nNewY, m_StartX, m_StartY,
                            //             m_StopX - m_StartX + 1, m_StopY - m_StartY + 1)
                            //   ){
                            //     m_Direction = 2;
                            //     m_CurrentX  = nNewX;
                            //     m_CurrentY  = nNewY;
                            //     return true;
                            // }
                        }
                        // when jump to 3, need to check whether it's overlapping with
                        // the start point of direction 0
                        if(m_Overlap[3]){
                            int nNewX = m_CenterX - m_Distance;
                            int nNewY = (std::max)(m_StartY, m_CenterY - m_Distance);
                            int nD0StartX = (std::max)(m_StartX, m_CenterX - m_Distance);
                            int nD0StartY = m_CenterY + m_Distance;

                            if(!(nNewX == nD0StartX && nNewY == nD0StartY)){
                                m_Direction = 3;
                                m_CurrentX  = nNewX;
                                m_CurrentY  = nNewY;
                                return true;
                            }

                            // int nNewX = m_CenterX - m_Distance;
                            // int nNewY = (std::max)(m_StartY, m_CenterY - m_Distance);
                            // if(nNewX == m_CurrentX && nNewY == m_CurrentY){
                            //     nNewY++;
                            // }
                            // if(PointInRectangle(nNewX, nNewY, m_StartX, m_StartY,
                            //             m_StopX - m_StartX + 1, m_StopY - m_StartY + 1)
                            //   ){
                            //     m_Direction = 3;
                            //     m_CurrentX  = nNewX;
                            //     m_CurrentY  = nNewY;
                            //     return true;
                            // }
                        }

                        m_Distance++;
                        CheckOverlap();
                        return MoveToNextRound();
                    }else{
                        m_CurrentX++;
                        return true;
                    }
                    break;
                }
            case 1:
                {
                    if(m_CurrentY == (std::max)(m_StartY, m_CenterY - m_Distance)){
                        if(m_Overlap[2]){
                            int nNewX = (std::min)(m_StopX, m_CenterX + m_Distance);
                            int nNewY = m_CenterY - m_Distance;
                            if(nNewX == m_CurrentX && nNewY == m_CurrentY){
                                nNewX--;
                            }
                            if(PointInRectangle(nNewX, nNewY, m_StartX, m_StartY,
                                        m_StopX - m_StartX + 1, m_StopY - m_StartY + 1)
                              ){
                                m_Direction = 2;
                                m_CurrentX  = nNewX;
                                m_CurrentY  = nNewY;
                                return true;
                            }
                        }
                        if(m_Overlap[3]){
                            int nNewX = m_CenterX - m_Distance;
                            int nNewY = (std::max)(m_StartY, m_CenterY - m_Distance);
                            int nD0StartX = (std::max)(m_StartX, m_CenterX - m_Distance);
                            int nD0StartY = m_CenterY + m_Distance;

                            if(!(nNewX == nD0StartX && nNewY == nD0StartY)){
                                m_Direction = 3;
                                m_CurrentX  = nNewX;
                                m_CurrentY  = nNewY;
                                return true;
                            }

                            // int nNewX = m_CenterX - m_Distance;
                            // int nNewY = (std::max)(m_StartY, m_CenterY - m_Distance);
                            // if(nNewX == m_CurrentX && nNewY == m_CurrentY){
                            //     nNewY++;
                            // }
                            // if(PointInRectangle(nNewX, nNewY, m_StartX, m_StartY,
                            //             m_StopX - m_StartX + 1, m_StopY - m_StartY + 1)
                            //   ){
                            //     m_Direction = 3;
                            //     m_CurrentX  = nNewX;
                            //     m_CurrentY  = nNewY;
                            //     return true;
                            // }
                        }
                        m_Distance++;
                        CheckOverlap();
                        return MoveToNextRound();
                    }else{
                        m_CurrentY--;
                        return true;
                    }
                    break;
                }
            case 2:
                {
                    if(m_CurrentX == (std::max)(m_StartX, m_CenterX - m_Distance)){
                        // hard
                        // first we need to skip the first point
                        // and we also need to check whether the first is on D0
                        if(m_Overlap[3]){
                            int nNewX = m_CenterX - m_Distance;
                            int nNewY = (std::max)(m_StartY, m_CenterY - m_Distance);
                            if(nNewX == m_CurrentX && nNewY == m_CurrentY){
                                nNewY++;
                            }
                            if(PointInRectangle(nNewX, nNewY, m_StartX, m_StartY,
                                        m_StopX - m_StartX + 1, m_StopY - m_StartY + 1)
                              ){
                                int nD0StartX = (std::max)(m_StartX, m_CenterX - m_Distance);
                                int nD0StartY = m_CenterY + m_Distance;
                                if(!(nNewX == nD0StartX && nNewY == nD0StartY)){
                                    m_Direction = 3;
                                    m_CurrentX  = nNewX;
                                    m_CurrentY  = nNewY;
                                    return true;
                                }
                            }
                        }
                        m_Distance++;
                        CheckOverlap();
                        return MoveToNextRound();
                    }else{
                        m_CurrentX--;
                        return true;
                    }
                    break;
                }
            case 3:
                {
                    if(m_StopY >= m_CenterY + m_Distance){
						// may jump to the end of D3 when invoking Reset()
                        // I have updated Reset(), for d 3 to get the smallest direction
                        // then this is impossible
                        // but keep it here as it is
                        // if(m_CurrentY == m_CenterY + m_Distance - 1){
                        if(m_CurrentY >= m_CenterY + m_Distance - 1){
                            m_Distance++;
                            CheckOverlap();
                            return MoveToNextRound();
                        }else{
                            m_CurrentY++;
                            return true;
                        }
                    }else{
                        if(m_CurrentY == m_StopY){
                            m_Distance++;
                            CheckOverlap();
                            return MoveToNextRound();
                        }else{
                            m_CurrentY++;
                            return true;
                        }
                    }
                    break;
                }
            default:
                {
                    return false;
                }
        }
    }else{
        m_Distance++;
        CheckOverlap();
        return MoveToNextRound();
    }
}
