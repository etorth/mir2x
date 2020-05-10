/*
 * =====================================================================================
 *
 *       Filename: rotatecoord.cpp
 *        Created: 08/15/2015 04:01:57
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
#include "mathf.hpp"
#include "rotatecoord.hpp"

RotateCoord::RotateCoord(int nCenterX, int nCenterY, int nStartX, int nStartY, int nW, int nH)
    : m_centerX(nCenterX)
    , m_centerY(nCenterY)
    , m_startX(nStartX)
    , m_startY(nStartY)
    , m_stopX(nStartX + nW - 1)
    , m_stopY(nStartY + nH - 1)
{
    if(nW <= 0 || nH <= 0){
        throw fflerror("invalid argument: W = %d, H = %d", nW, nH);
    }

    // simpler case
    // center point is inside the rectangle

    if(mathf::pointInRectangle(m_centerX, m_centerY, m_startX, m_startY, nW, nH)){
        m_distance  = 0;
        m_direction = DirType::DIR_0;
        m_currentX  = m_centerX;
        m_currentY  = m_centerY;

        CheckOverlap();
        return;
    }

    // center point is not inside the rectangle
    // we should firstly decide the rotation direction for initialization
    //
    //         (1)
    //      *-------*
    //      |*     *|
    //      | *   * |
    //      |  * *  |  (2)
    //  (4) |   * <-- CenterX, CenterY
    //      |  * *  |
    //      | *   * |
    //      |*     *|
    //      *-------*
    //         (3)

    // case-(1)
    // center pointer is at the top of the rectangle
    if(true
            && ((m_centerX - m_centerY) >= (m_startX - m_startY))
            && ((m_centerX + m_centerY) <  (m_stopX  + m_startY))
            && ((m_centerY < m_startY))){

        m_distance  = m_startY - m_centerY;
        m_direction = DirType::DIR_0;
        m_currentX  = (std::max<int>)(m_startX, m_centerX - m_distance);
        m_currentY  = m_startY;

        CheckOverlap();
        return;
    }

    // case-(2)
    // complicated part here
    // center pointer is at the right side of the rectangle
    // when set direction to 0 1 2 it's ok
    // but for 3, need to set the the smallest valid direction
    // because for direction = 3, there is jumping when forwarding
    if(true
            && ((m_centerX + m_centerY) >= (m_stopX + m_startY))
            && ((m_centerX - m_centerY) >  (m_stopX - m_stopY ))
            && ((m_centerX > m_stopX))){

        m_distance = m_centerX - m_stopX;
        m_currentX = m_stopX;

        if(m_stopY >= m_centerY + m_distance){
            // the vertical edge is long enough
            // on it we can get one point as start point with direction as 0
            m_direction = DirType::DIR_0;
            m_currentY  = m_centerY + m_distance;
        }else{
            // set start point with direction = 3
            m_direction = DirType::DIR_3;
            m_currentY  = (std::max<int>)(m_startY, m_centerY - m_distance);
        }

        CheckOverlap();
        return;
    }

    // case-(3)
    // center pointer is at the bottom of the rectangle
    if(true
            && ((m_centerX - m_centerY) <= (m_stopX  - m_stopY))
            && ((m_centerX + m_centerY) >  (m_startX + m_stopY))
            && ((m_centerY > m_stopY))){

        m_distance  = m_centerY - m_stopY;
        m_direction = DirType::DIR_2;
        m_currentX  = (std::min<int>)(m_stopX, m_centerX + m_distance);
        m_currentY  = m_stopY;

        CheckOverlap();
        return;
    }

    // case-(4)
    // center pointer is at the left side of the rectangle
    if(true
            && ((m_centerX + m_centerY) <= (m_startX + m_stopY ))
            && ((m_centerX - m_centerY) <  (m_startX - m_startY))
            && ((m_centerX < m_startX))){

        m_distance  = m_startX - m_centerX;
        m_direction = DirType::DIR_1;
        m_currentX  = m_startX;
        m_currentY  = (std::min<int>)(m_stopY, m_centerY + m_distance);

        CheckOverlap();
        return;
    }

    // report bug
    // there is case doesn't cover
    throw fflerror("missed logic");
}

void RotateCoord::CheckOverlap()
{
    m_overlap[0] = mathf::pointInSegment(m_centerY + m_distance, m_startY, m_stopY - m_startY + 1);
    m_overlap[1] = mathf::pointInSegment(m_centerX + m_distance, m_startX, m_stopX - m_startX + 1);
    m_overlap[2] = mathf::pointInSegment(m_centerY - m_distance, m_startY, m_stopY - m_startY + 1);
    m_overlap[3] = mathf::pointInSegment(m_centerX - m_distance, m_startX, m_stopX - m_startX + 1);
}

bool RotateCoord::MoveToNextRound()
{
    // always pick the first valid position following current direction
    if(m_overlap[0]){
        m_direction = DirType::DIR_0;
        m_currentX  = (std::max<int>)(m_startX, m_centerX - m_distance);
        m_currentY  = m_centerY + m_distance;
        return true;
    }else if(m_overlap[1]){
        m_direction = DirType::DIR_1;
        m_currentX  = m_centerX + m_distance;
        m_currentY  = (std::min<int>)(m_stopY, m_centerY + m_distance);
        return true;
    }else if(m_overlap[2]){
        m_direction = DirType::DIR_2;
        m_currentX  = (std::min<int>)(m_stopX, m_centerX + m_distance);
        m_currentY  = m_centerY - m_distance;
        return true;
    }else if(m_overlap[3]){
        m_direction = DirType::DIR_3;
		m_currentX  = m_centerX - m_distance;
        m_currentY  = (std::max<int>)(m_startY, m_centerY - m_distance);
        return true;
    }else{
        return false;
    }
}

bool RotateCoord::forward()
{
    // based on boundary but not current position
    // every time when we reach the boundary, check next boundary

    if(m_distance < 0){
        throw fflerror("invalid distance %d", m_distance);
    }

    // center point is inside the rectangle and we are immediately after calling ctor
    // this is the only case that current X and Y are the center point

    if(m_distance == 0){
        m_distance++;
        CheckOverlap();
        return MoveToNextRound();
    }

    // distance greater than zero
    // check direction

    switch(m_direction){
        case DirType::DIR_0:
            {
                if(m_currentX == (std::min<int>)(m_stopX, m_centerX + m_distance)){
                    if(m_overlap[1]){

                        // 0 and 1 are neighbor
                        // and (NewX, NewY) is the point of next edge direction 1
                        // so need to check if overlap at the endpoint of direction 0

                        int nNewX = m_centerX + m_distance;
                        int nNewY = (std::min<int>)(m_stopY, m_centerY + m_distance);

                        if(true
                                && (nNewX == m_currentX)
                                && (nNewY == m_currentY)){ nNewY--; }

                        if(mathf::pointInRectangle(nNewX, nNewY, m_startX, m_startY, m_stopX - m_startX + 1, m_stopY - m_startY + 1)){
                            m_direction = DirType::DIR_1;
                            m_currentX  = nNewX;
                            m_currentY  = nNewY;
                            return true;
                        }
                    }

                    if(m_overlap[2]){
                        // if we need to jump from 0 to 2
                        // because they are not neighbor nothing needs to be check
                        // just jump to the first valid point

                        m_currentX  = (std::min<int>)(m_stopX, m_centerX + m_distance);
                        m_currentY  = m_centerY - m_distance;
                        m_direction = DirType::DIR_2;
                        return true;
                    }

                    if(m_overlap[3]){

                        // 0 and 3 are neighbors and if we need to jump to 3
                        // need to check whether it's overlapping with the start point of direction 0

                        // (NewX, NewY) is starting point of direction 3
                        int nNewX = m_centerX - m_distance;
                        int nNewY = (std::max<int>)(m_startY, m_centerY - m_distance);

                        // (D0StartX, D0StartY) is the starting point of direction 0
                        // and we have traversed it
                        int nD0StartX = (std::max<int>)(m_startX, m_centerX - m_distance);
                        int nD0StartY = m_centerY + m_distance;

                        if(true
                                && (nNewX == nD0StartX)
                                && (nNewY == nD0StartY)){
                            // ooops
                            // start point of direction 3 is the point we have tranversed
                            // do nothing here and wait to jump to next round
                        }else{
                            m_direction = DirType::DIR_3;
                            m_currentX  = nNewX;
                            m_currentY  = nNewY;
                            return true;
                        }
                    }

                    // can't take direction 1, 2, 3
                    // means we have done for current round
                    m_distance++;
                    CheckOverlap();
                    return MoveToNextRound();

                }else{
                    // move forward to next gird
                    // haven't finish direction 0 yet
                    m_currentX++;
                    return true;
                }
                break;
            }
        case DirType::DIR_1:
            {
                // for rest cases I won't write detailed comments
                // refer to case-0 for reasoning

                if(m_currentY == (std::max<int>)(m_startY, m_centerY - m_distance)){
                    if(m_overlap[2]){
                        int nNewX = (std::min<int>)(m_stopX, m_centerX + m_distance);
                        int nNewY = m_centerY - m_distance;
                        if(true
                                && (nNewX == m_currentX)
                                && (nNewY == m_currentY)){ nNewX--; }

                        if(mathf::pointInRectangle(nNewX, nNewY, m_startX, m_startY, m_stopX - m_startX + 1, m_stopY - m_startY + 1)){
                            m_direction = DirType::DIR_2;
                            m_currentX  = nNewX;
                            m_currentY  = nNewY;
                            return true;
                        }
                    }

                    if(m_overlap[3]){
                        int nNewX = m_centerX - m_distance;
                        int nNewY = (std::max<int>)(m_startY, m_centerY - m_distance);

                        int nD0StartX = (std::max<int>)(m_startX, m_centerX - m_distance);
                        int nD0StartY = m_centerY + m_distance;

                        if(true
                                && (nNewX == nD0StartX)
                                && (nNewY == nD0StartY)){
                        }else{
                            m_direction = DirType::DIR_3;
                            m_currentX  = nNewX;
                            m_currentY  = nNewY;
                            return true;
                        }
                    }

                    m_distance++;
                    CheckOverlap();
                    return MoveToNextRound();

                }else{
                    m_currentY--;
                    return true;
                }
                break;
            }
        case DirType::DIR_2:
            {
                if(m_currentX == (std::max<int>)(m_startX, m_centerX - m_distance)){
                    if(m_overlap[3]){
                        int nNewX = m_centerX - m_distance;
                        int nNewY = (std::max<int>)(m_startY, m_centerY - m_distance);
                        if(true
                                && (nNewX == m_currentX)
                                && (nNewY == m_currentY)){ nNewY++; }

                        if(mathf::pointInRectangle(nNewX, nNewY, m_startX, m_startY, m_stopX - m_startX + 1, m_stopY - m_startY + 1)){
                            int nD0StartX = (std::max<int>)(m_startX, m_centerX - m_distance);
                            int nD0StartY = m_centerY + m_distance;
                            if(true
                                    && (nNewX == nD0StartX)
                                    && (nNewY == nD0StartY)){
                            }else{
                                m_direction = DirType::DIR_3;
                                m_currentX  = nNewX;
                                m_currentY  = nNewY;
                                return true;
                            }
                        }
                    }

                    m_distance++;
                    CheckOverlap();
                    return MoveToNextRound();
                }else{
                    m_currentX--;
                    return true;
                }
                break;
            }
        case DirType::DIR_3:
            {
                // complicated part
                // direction 3 will jump to direction 0 if finished

                if(m_stopY >= m_centerY + m_distance){
                    if(m_currentY >= m_centerY + m_distance - 1){

                        // when (vertical) edge of direction 3 is enough long
                        // even if there are still valid points below current point
                        // we make a turn to start next round

                        m_distance++;
                        CheckOverlap();
                        return MoveToNextRound();
                    }else{
                        m_currentY++;
                        return true;
                    }
                }else{
                    if(m_currentY == m_stopY){
                        m_distance++;
                        CheckOverlap();
                        return MoveToNextRound();
                    }else{
                        m_currentY++;
                        return true;
                    }
                }
                break;
            }
        default:
            {
                throw fflerror("invalid inn direction: %d", static_cast<int>(m_direction));
            }
    }
}
