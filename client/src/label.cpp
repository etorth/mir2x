/*
 * =====================================================================================
 *
 *       Filename: label.cpp
 *        Created: 08/12/2015 09:59:15
 *  Last Modified: 04/02/2016 03:12:46
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

#include "label.hpp"
#include "tokenboard.hpp"
#include <unordered_map>
#include <functional>
#include "xmlobjectlist.hpp"
#include "colorfunc.hpp"

void Label::SetText(const char * szInfo)
{
    if(szInfo){
        m_Content = szInfo;
    }

    XMLObjectList stObjectList;
    char szStyle[16];
    std::sprintf(szStyle, "0X%02X", (m_FontKey & 0X00FF0000) >> 16);

    char szColor[16];
    std::sprintf(szColor, "0X%08X", Color2U32ARGB(m_Color));


    stObjectList.Add(
            {
                {"type" , std::string("plaintext")},
                {"font" , std::to_string((m_FontKey & 0X000000FF) >> 0)},
                {"size" , std::to_string((m_FontKey & 0X0000FF00) >> 8)},
                {"style", std::string(szStyle)},
                {"color", std::string(szColor)}
            }, m_Content.c_str());

    m_TokenBoard.Load(stObjectList);

    m_W = m_TokenBoard.W();
    m_H = m_TokenBoard.H();
}
