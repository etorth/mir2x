/*
 * =====================================================================================
 *
 *       Filename: grounditem.hpp
 *        Created: 07/30/2017 20:59:10
 *  Last Modified: 07/30/2017 21:05:23
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

    GroundItem(uint32_t nID, int nX, int nY)
        : ID(nID)
        , X(nX)
        , Y(nY)
    {}
};
