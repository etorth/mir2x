/*
 * =====================================================================================
 *
 *       Filename: actorthreadmonitortable.hpp
 *        Created: 12/04/2018 23:03:26
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
#include "fltableimpl.hpp"

class ActorThreadMonitorTable: public Fl_TableImpl
{
    public:
        ActorThreadMonitorTable(int nX, int nY, int nW, int nH, const char *szLabel = nullptr)
            :Fl_TableImpl(nX, nY, nW, nH, szLabel)
        {}
};
