/*
 * =====================================================================================
 *
 *       Filename: grounditem.hpp
 *        Created: 07/30/2017 20:59:10
 *  Last Modified: 12/01/2017 18:02:22
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

#pragma once
struct GroundItem
{
    uint32_t ID;

    int X;
    int Y;

    GroundItem(uint32_t nID = 0, int nX = -1, int nY = -1)
        : ID(nID)
        , X(nX)
        , Y(nY)
    {}

    operator bool () const
    {
        return ID != 0;
    }
};
