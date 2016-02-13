/*
 * =====================================================================================
 *
 *       Filename: passwordbox.cpp
 *        Created: 8/23/2015 1:36:26 AM
 *  Last Modified: 08/23/2015 2:01:05 AM
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
#include "passwordbox.hpp"
#include <algorithm>
#include "fonttexturemanager.hpp"
#include <utf8.h>

PasswordBox::PasswordBox(int nW, int nRH, const FONTINFO & stFontInfo, const SDL_Color & stColor)
    : InputBox(nW, nRH, stFontInfo, stColor)
{}

void PasswordBox::Compile()
{
    m_Line.clear();

    TOKENBOX stTokenBox;

    const char *pStart = m_Content.c_str();
    const char *pEnd   = pStart;

    while(*pEnd != '\0'){
        pStart = pEnd;
        utf8::unchecked::advance(pEnd, 1);
        std::memset(stTokenBox.UTF8CharBox.Data, 0, 8);
        if(pEnd - pStart == 1 && (*pStart == '\n' || *pStart == '\t' || *pStart == '\r')){
            // continue;
            stTokenBox.UTF8CharBox.Data[0] = ' ';
        }else{
            // std::memcpy(stTokenBox.UTF8CharBox.Data, pStart, pEnd - pStart);
            std::memcpy(stTokenBox.UTF8CharBox.Data, "*", 1);
        }
        LoadUTF8CharBoxCache(stTokenBox);
        PushBack(stTokenBox);
    }
    SetTokenBoxStartX();
}
