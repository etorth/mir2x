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

// Use a 8-byte hardcode to identify a font texture
//  4-byte: utf8 code
//  3-bits: font file index
//  7-bits: font size
//  6-bits: R
//  6-bits: G
//  6-bits: B
//  1-bits: underline
//  1-bits: line through
//  1-bits: bold
//  1-bits: italic

struct{
    uint32_t UTF8Code;   // max length of UTF-8 is 4-byte by RFC-3629
    uint8_t  FontIndex; // Font file index, support 0 ~ 7
    uint8_t  FontSize;  // Font size, support 0 ~ 128
    uint8_t  R;         // R for 0 ~ 64, use only 6-bits
    uint8_t  G;
    uint8_t  B;
    uint8_t  Bold;
    uint8_t  UnderLine;
    uint8_t  StrikeThrough;
    uint8_t  Italic;
    
    UTF8CharTextureIndicator(uint32_t nUTF8Code, uint8_t nFontIndex, uint8_t nFontSize,
        uint8_t nR, uint8_t nG, uint8_t nB)
            : UTF8Code(nUTF8Code)
            , FontIndex(nFontIndex)
            , FontSize(nFontSize)
            , R(nR)
            , G(nG)
            , B(nB)
            , Bold(0)
            , UnderLine(0)
            , StrikeThrough(0)
            , Italic(0)
    {
    }
    
    uint64_t HC()
    {
        uint64_t nHC = 0;
        nHC &= ((uint64_t)(UTF8Code));
        nHC &= ((uint64_t)(FontIndex & 0X07) << 32);
        nHC &= ((uint64_t)(FontSize  & 0X8F) << 35);
        
        nHC &= ((uint64_t)(R >> 2) << 41);
        nHC &= ((uint64_t)(G >> 2) << 47);
        nHC &= ((uint64_t)(B >> 2) << 53);
        
        nHC &= ((uint64_t)(Bold          ? ((uint64_t)(1) << 59) : 0));
        nHC &= ((uint64_t)(UnderLine     ? ((uint64_t)(1) << 60) : 0));
        nHC &= ((uint64_t)(StrikeThrough ? ((uint64_t)(1) << 61) : 0));
        nHC &= ((uint64_t)(Italic        ? ((uint64_t)(1) << 61) : 0));
    }
}UTF8CharTextureIndicator;
