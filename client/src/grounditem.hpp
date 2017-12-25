/*
 * =====================================================================================
 *
 *       Filename: grounditem.hpp
 *        Created: 07/30/2017 20:59:10
 *  Last Modified: 12/24/2017 23:31:41
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
    uint32_t ID = 0;

    int X = -1;
    int Y = -1;

    GroundItem(uint32_t nID, int nX, int nY)
        : ID(nID)
        , X(nX)
        , Y(nY)
    {}

    operator bool () const
    {
        return ID != 0;
    }
};
