/*
 * =====================================================================================
 *
 *       Filename: section.hpp
 *        Created: 8/18/2015 6:56:11 PM
 *  Last Modified: 03/18/2016 13:29:16
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
#include <SDL2/SDL.h>

typedef struct{
    int                     Type;
    int                     Set;
    int                     Index;
    int                     FPS;
    int                     FrameCount;
}EMOTICONSECTIONINFO;

typedef struct{
    int                     Valid;
    double                  MS;
    int                     FrameIndex;
}EMOTICONSECTIONSTATE;

typedef struct{
    int                     Event;
}TEXTSECTIONSTATE;

typedef struct{
    int                     Type;
    uint8_t                 FileIndex;
    uint8_t                 Size;
    uint8_t                 Style;
    SDL_Color               Color[3];
}TEXTSECTIONINFO;

enum{
    SECTIONTYPE_ALL         =      0,
    SECTIONTYPE_EVENTTEXT   = 1 << 1,
    SECTIONTYPE_EMOTICON    = 1 << 2,
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
