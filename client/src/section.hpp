/*
 * =====================================================================================
 *
 *       Filename: section.hpp
 *        Created: 8/18/2015 6:56:11 PM
 *  Last Modified: 05/13/2017 22:21:28
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

typedef struct
{
    int                     Type;
    int                     Set;
    int                     Index;
    int                     FPS;
    int                     FrameCount;
}EMOTICONSECTIONINFO;

typedef struct
{
    int                     Valid;
    double                  MS;
    int                     FrameIndex;
}EMOTICONSECTIONSTATE;

typedef struct
{
    int                     Event;
    int                     ID;
}TEXTSECTIONSTATE;

typedef struct
{
    int                     Type;
    uint8_t                 Font;
    uint8_t                 Size;
    uint8_t                 Style;
    SDL_Color               Color[3];
}TEXTSECTIONINFO;

enum SectionType: int
{
    SECTIONTYPE_ALL         = 0,
    SECTIONTYPE_PLAINTEXT   = 1,
    SECTIONTYPE_EVENTTEXT   = 2,
    SECTIONTYPE_EMOTICON    = 3,
    SECTIONTYPE_TEXT        = 4,
};

typedef union
{
    int                     Type;
    EMOTICONSECTIONINFO     Emoticon;
    TEXTSECTIONINFO         Text;
}SECTIONINFO;

typedef union
{
    EMOTICONSECTIONSTATE    Emoticon;
    TEXTSECTIONSTATE        Text;
}SECTIONSTATE;

typedef struct
{
    SECTIONINFO             Info;
    SECTIONSTATE            State;
}SECTION;
