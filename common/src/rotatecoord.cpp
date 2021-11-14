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
#include "fflerror.hpp"
#include "rotatecoord.hpp"

RotateCoord::RotateCoord(int centerX, int centerY, int startX, int startY, int w, int h)
    : m_centerX(centerX)
    , m_centerY(centerY)
    , m_startX(startX)
    , m_startY(startY)
    , m_stopX(startX + w - 1)
    , m_stopY(startY + h - 1)
{
    fflassert(w > 0);
    fflassert(h > 0);

    // simpler case
    // center point is inside the rectangle

    if(mathf::pointInRectangle<int>(m_centerX, m_centerY, m_startX, m_startY, w, h)){
        m_distance  = 0;
        m_direction = DirType::DIR_0;
        m_currentX  = m_centerX;
        m_currentY  = m_centerY;

        checkOverlap();
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
        m_currentX  = std::max<int>(m_startX, m_centerX - m_distance);
        m_currentY  = m_startY;

        checkOverlap();
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
        }
        else{
            // set start point with direction = 3
            m_direction = DirType::DIR_3;
            m_currentY  = std::max<int>(m_startY, m_centerY - m_distance);
        }

        checkOverlap();
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
        m_currentX  = std::min<int>(m_stopX, m_centerX + m_distance);
        m_currentY  = m_stopY;

        checkOverlap();
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
        m_currentY  = std::min<int>(m_stopY, m_centerY + m_distance);

        checkOverlap();
        return;
    }

    // report bug
    // there is case doesn't cover
    throw fflreach();
}

void RotateCoord::checkOverlap()
{
    m_overlap[0] = mathf::pointInSegment(m_centerY + m_distance, m_startY, m_stopY - m_startY + 1);
    m_overlap[1] = mathf::pointInSegment(m_centerX + m_distance, m_startX, m_stopX - m_startX + 1);
    m_overlap[2] = mathf::pointInSegment(m_centerY - m_distance, m_startY, m_stopY - m_startY + 1);
    m_overlap[3] = mathf::pointInSegment(m_centerX - m_distance, m_startX, m_stopX - m_startX + 1);
}

bool RotateCoord::moveToNextRound()
{
    // distance has been updated, called as:
    //
    //      m_distance++;
    //      checkOverlap();
    //      return moveToNextRound();
    //     
    // always pick the first valid position following current direction

    if(m_overlap[0]){
        m_direction = DirType::DIR_0;
        m_currentX  = std::max<int>(m_startX, m_centerX - m_distance);
        m_currentY  = m_centerY + m_distance;
        return true;
    }
    else if(m_overlap[1]){
        m_direction = DirType::DIR_1;
        m_currentX  = m_centerX + m_distance;
        m_currentY  = std::min<int>(m_stopY, m_centerY + m_distance);
        return true;
    }
    else if(m_overlap[2]){
        m_direction = DirType::DIR_2;
        m_currentX  = std::min<int>(m_stopX, m_centerX + m_distance);
        m_currentY  = m_centerY - m_distance;
        return true;
    }
    else if(m_overlap[3]){
        m_direction = DirType::DIR_3;
        m_currentX  = m_centerX - m_distance;
        m_currentY  = std::max<int>(m_startY, m_centerY - m_distance);
        return true;
    }
    else{
        return false;
    }
}

bool RotateCoord::forward()
{
    // based on boundary but not current position
    // every time when we reach the boundary, check next boundary

    fflassert(m_distance >= 0);

    // center point is inside the rectangle and we are immediately after calling ctor
    // this is the only case that current X and Y are the center point

    if(m_distance == 0){
        m_distance++;
        checkOverlap();
        return moveToNextRound();
    }

    // distance greater than zero
    // check direction

    switch(m_direction){
        case DirType::DIR_0:
            {
                if(m_currentX == std::min<int>(m_stopX, m_centerX + m_distance)){
                    if(m_overlap[1]){

                        // 0 and 1 are neighbor
                        // and (NewX, NewY) is the point of next edge direction 1
                        // so need to check if overlap at the endpoint of direction 0

                        const int newX = m_centerX + m_distance;
                        /* */ int newY = std::min<int>(m_stopY, m_centerY + m_distance);

                        if(true
                                && (newX == m_currentX)
                                && (newY == m_currentY)){
                            newY--;
                        }

                        if(mathf::pointInRectangle(newX, newY, m_startX, m_startY, m_stopX - m_startX + 1, m_stopY - m_startY + 1)){
                            m_direction = DirType::DIR_1;
                            m_currentX  = newX;
                            m_currentY  = newY;
                            return true;
                        }
                    }

                    if(m_overlap[2]){
                        // if we need to jump from 0 to 2
                        // because they are not neighbor nothing needs to be check
                        // just jump to the first valid point

                        m_currentX  = std::min<int>(m_stopX, m_centerX + m_distance);
                        m_currentY  = m_centerY - m_distance;
                        m_direction = DirType::DIR_2;
                        return true;
                    }

                    if(m_overlap[3]){

                        // 0 and 3 are neighbors and if we need to jump to 3
                        // need to check whether it's overlapping with the start point of direction 0

                        // (newX, newY) is starting point of direction 3
                        const int newX = m_centerX - m_distance;
                        const int newY = std::max<int>(m_startY, m_centerY - m_distance);

                        // (d0StartX, d0StartY) is the starting point of direction 0
                        // and we have traversed it
                        const int d0StartX = std::max<int>(m_startX, m_centerX - m_distance);
                        const int d0StartY = m_centerY + m_distance;

                        if(true
                                && (newX == d0StartX)
                                && (newY == d0StartY)){
                            // ooops
                            // start point of direction 3 is the point we have tranversed
                            // do nothing here and wait to jump to next round
                        }
                        else{
                            m_direction = DirType::DIR_3;
                            m_currentX  = newX;
                            m_currentY  = newY;
                            return true;
                        }
                    }

                    // can't take direction 1, 2, 3
                    // means we have done for current round
                    m_distance++;
                    checkOverlap();
                    return moveToNextRound();

                }
                else{
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

                if(m_currentY == std::max<int>(m_startY, m_centerY - m_distance)){
                    if(m_overlap[2]){
                        /* */ int newX = std::min<int>(m_stopX, m_centerX + m_distance);
                        const int newY = m_centerY - m_distance;

                        if(true
                                && (newX == m_currentX)
                                && (newY == m_currentY)){
                            newX--;
                        }

                        if(mathf::pointInRectangle(newX, newY, m_startX, m_startY, m_stopX - m_startX + 1, m_stopY - m_startY + 1)){
                            m_direction = DirType::DIR_2;
                            m_currentX  = newX;
                            m_currentY  = newY;
                            return true;
                        }
                    }

                    if(m_overlap[3]){
                        const int newX = m_centerX - m_distance;
                        const int newY = std::max<int>(m_startY, m_centerY - m_distance);

                        const int d0StartX = std::max<int>(m_startX, m_centerX - m_distance);
                        const int d0StartY = m_centerY + m_distance;

                        if(true
                                && (newX == d0StartX)
                                && (newY == d0StartY)){
                            // do nothing here and wait to jump to next round
                        }
                        else{
                            m_direction = DirType::DIR_3;
                            m_currentX  = newX;
                            m_currentY  = newY;
                            return true;
                        }
                    }

                    m_distance++;
                    checkOverlap();
                    return moveToNextRound();
                }
                else{
                    m_currentY--;
                    return true;
                }
                break;
            }
        case DirType::DIR_2:
            {
                if(m_currentX == std::max<int>(m_startX, m_centerX - m_distance)){
                    if(m_overlap[3]){
                        const int newX = m_centerX - m_distance;
                        /* */ int newY = std::max<int>(m_startY, m_centerY - m_distance);

                        if(true
                                && (newX == m_currentX)
                                && (newY == m_currentY)){
                            newY++;
                        }

                        if(mathf::pointInRectangle(newX, newY, m_startX, m_startY, m_stopX - m_startX + 1, m_stopY - m_startY + 1)){
                            const int d0StartX = std::max<int>(m_startX, m_centerX - m_distance);
                            const int d0StartY = m_centerY + m_distance;
                            if(true
                                    && (newX == d0StartX)
                                    && (newY == d0StartY)){
                                // do nothing here and wait to jump to next round
                            }
                            else{
                                m_direction = DirType::DIR_3;
                                m_currentX  = newX;
                                m_currentY  = newY;
                                return true;
                            }
                        }
                    }

                    m_distance++;
                    checkOverlap();
                    return moveToNextRound();
                }
                else{
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
                        checkOverlap();
                        return moveToNextRound();
                    }
                    else{
                        m_currentY++;
                        return true;
                    }
                }
                else{
                    if(m_currentY == m_stopY){
                        m_distance++;
                        checkOverlap();
                        return moveToNextRound();
                    }
                    else{
                        m_currentY++;
                        return true;
                    }
                }
                break;
            }
        default:
            {
                throw fflreach();
            }
    }
}
