/*
 * =====================================================================================
 *
 *       Filename: tokenbox.hpp
 *        Created: 7/2/2015 3:31:13 PM
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
#include "utf8char.hpp"
#include "emoticon.hpp"

// everything can be retrieved is in cache
// everything can be updated but non-retrieving is in state
//
// try most to move information to SECTION
// TOKENBOX only store specified information
//
//
// start with MA- is for memory align
// do not refer it in programming

// general state
typedef struct{
    int                 MASection;
    int                 Valid;
    int                 W1;
    int                 W2;
}TOKENBOXSTATE;

// general cache
typedef struct{
    TOKENBOXSTATE       MAState;          // just for memory align
    int                 StartX;
    int                 StartY;
    int                 W;
    int                 H;
    int                 H1;
    int                 H2;
}TOKENBOXCACHE;

typedef struct{
    uint32_t            Key;
}EMOTICONBOXCACHE;

typedef struct{
    uint64_t            Key;
}UTF8CHARBOXCACHE;

typedef struct{
    TOKENBOXCACHE       MACache;
    EMOTICONBOXCACHE    Cache;
}EMOTICONBOX;

typedef struct{
    TOKENBOXCACHE       MACache;
    UTF8CHARBOXCACHE    Cache;
    uint32_t            UTF8Code;
}UTF8CHARBOX;

typedef union{
    int                 Section;
    TOKENBOXSTATE       State;
    TOKENBOXCACHE       Cache;
    UTF8CHARBOX         UTF8CharBox;
    EMOTICONBOX         EmoticonBox;
}TOKENBOX;
