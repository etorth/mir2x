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
#include <Fl/Fl_Table_Row.H>

class Fl_TableImpl: public Fl_Table_Row
{
    public:
        Fl_TableImpl(int nX, int nY, int nW, int nH, const char *szLabel = nullptr)
            : Fl_Table_Row(nX, nY, nW, nH, szLabel)
        {}
        
    public:
        int GetFontWidth() const;
};
