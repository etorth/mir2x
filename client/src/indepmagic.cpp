/*
 * =====================================================================================
 *
 *       Filename: indepmagic.cpp
 *        Created: 08/07/2017 21:31:24
 *  Last Modified: 08/08/2017 12:22:02
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
#include "indepmagic.hpp"
IndepMagic::IndepMagic(uint32_t nUID,
        int nMagicID,
        int nMagicParam,
        int nSpeed,
        int nDirection,
        int nX,
        int nY,
        int nAimX,
        int nAimY,
        uint32_t nAimUID)
    : m_UID(nUID)
    , m_MagicID(nMagicID)
    , m_MagicParam(nMagicParam)
    , m_Speed(nSpeed)
    , m_Direction(nDirection)
    , m_X(nX)
    , m_Y(nY)
    , m_AimX(nAimX)
    , m_AimY(nAimY)
    , m_AimUID(nAimUID)
{}

IndepMagic::IndepMagic(uint32_t nUID,
        int nMagicID,
        int nMagicParam,
        int nSpeed,
        int nDirection,
        int nX,
        int nY,
        int nAimX,
        int nAimY)
    : IndepMagic(nUID,
            nMagicID,
            nMagicParam,
            nSpeed,
            nDirection,
            nX,
            nY,
            nAimX,
            nAimY,
            0)
{}

IndepMagic::IndepMagic(uint32_t nUID,
        int nMagicID,
        int nMagicParam,
        int nSpeed,
        int nDirection,
        int nX,
        int nY,
        uint32_t nAimUID)
    : IndepMagic(nUID,
            nMagicID,
            nMagicParam,
            nSpeed,
            nDirection,
            nX,
            nY,
            -1,
            -1,
            nAimUID)
{}
