/*
 * =====================================================================================
 *
 *       Filename: fltableimpl.cpp
 *        Created: 12/06/2018 00:29:57
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

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include "fltableimpl.hpp"

int Fl_TableImpl::GetFontWidth() const
{
    // disble the @SYMBOL handling
    int nW = 0;
    int nH = 0;

    const char *sz10WidthChars = "MMMMMHHHHH";
    fl_measure(sz10WidthChars, nW, nH, 0);

    return nW / 10;
}
