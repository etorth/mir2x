/*
 * =====================================================================================
 *
 *       Filename: fltableimpl.hpp
 *        Created: 12/04/2018 22:59:06
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
#include <Fl/Fl_Table.H>

class Fl_TableImpl: public Fl_Table
{
    public:
        Fl_TableImpl(int nX, int nY, int nW, int nH, const char *szLabel = nullptr)
            : Fl_Table(nX, nY, nW, nH, szLabel)
        {}
};
