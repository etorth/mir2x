/*
 * =====================================================================================
 *
 *       Filename: section.hpp
 *        Created: 8/18/2015 6:56:11 PM
 *  Last Modified: 08/19/2015 3:38:59 AM
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
#include "tokenbox.hpp"

typedef struct{
    int                     Type;
    int                     Set;
    int                     Index;
    int                     FPS;
    int                     FrameCount;
}EMOTICONSECTIONINFO;

typedef struct{
    int                     Ticks;
    int                     FrameIndex;
}EMOTICONSECTIONSTATE;

typedef struct{
    int                     Event;
}TEXTSECTIONSTATE;

typedef struct{
    int                     Type;
    FONTINFO                FontInfo;
    SDL_Color               Color[3];
}TEXTSECTIONINFO;

enum{
    SECTIONTYPE_ALL         =      0,
    SECTIONTYPE_EVENTTEXT   = 1 << 1,
    SECTIONTYPE_TEXT        = 1 << 2,
    SECTIONTYPE_EMOTICON    = 1 << 3,
};

typedef union{
    int                     Type;
    EMOTICONSECTIONINFO     Emoticon;
    TEXTSECTIONINFO         Text;
}SECTIONINFO;

typedef union{
    EMOTICONSECTIONSTATE    Emoticon;
    TEXTSECTIONSTATE        Text;
}SECTIONSTATE;

typedef struct{
    SECTIONINFO             Info;
    SECTIONSTATE            State;
}SECTION;
