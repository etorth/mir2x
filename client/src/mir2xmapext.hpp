/*
 * =====================================================================================
 *
 *       Filename: mir2xmapext.hpp
 *        Created: 03/11/2016 23:30:36
 *  Last Modified: 03/12/2016 19:22:48
 *
 *    Description: wrapper for Mir2xMap
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
#include "mir2xmap.hpp"

class Actor;
class Mir2xMapExt final
{
    private:
        int m_ViewX;
        int m_ViewY;

    public:
        Mir2xMapExt();
       ~Mir2xMapExt();

    public:
       int ViewX()
       {
           return m_ViewX;
       }

       int ViewY()
       {
           return m_ViewY;
       }

    public:
       bool ValidPosition(int, int, const Actor &);
};
