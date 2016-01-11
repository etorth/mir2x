/*
 * =====================================================================================
 *
 *       Filename: utf8char.hpp
 *        Created: 7/3/2015 2:05:13 PM
 *  Last Modified: 08/20/2015 10:13:22 PM
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

#define FONTSTYLE_SOLID         0
#define FONTSTYLE_SHADED        1
#define FONTSTYLE_BLENDED       2

typedef struct{
    int     Index;
    int     Size;
    int     Style;
}FONTINFO;

typedef struct{
    FONTINFO    FontInfo;       // can't change
    SDL_Color   Color;          // can change
    char        Data[8];        // change is supported but not recommended
}UTF8CHARTEXTUREINDICATOR;
