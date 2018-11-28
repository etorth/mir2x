/*
 * =====================================================================================
 *
 *       Filename: labelboard.cpp
 *        Created: 08/12/2015 09:59:15
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

#include <functional>
#include <unordered_map>

#include "global.hpp"
#include "strfunc.hpp"
#include "colorfunc.hpp"
#include "labelboard.hpp"
#include "tokenboard.hpp"
#include "xmlobjectlist.hpp"

void LabelBoard::FormatText(const char * szFormatStr, ...)
{
    std::string szText;
    bool bError = false;
    {
        va_list ap;
        va_start(ap, szFormatStr);

        try{
            szText = str_vprintf(szFormatStr, ap);
        }catch(const std::exception &e){
            bError = true;
            szText = str_printf("Exception caught in LabelBoard::FormatText(\"%s\", ...): %s", szFormatStr, e.what());
        }

        va_end(ap);
    }

    if(bError){
        g_Log->AddLog(LOGTYPE_WARNING, "%s", szText.c_str());
    }

    // 1. store parameter as m_Content
    // 2. build the token board for drawing
    m_Content = szText;

    char szStyle[16];
    std::sprintf(szStyle, "0X%02X", m_FontStyle);

    char szColor[16];
    std::sprintf(szColor, "0X%08X", ColorFunc::Color2ARGB(m_FontColor));

    XMLObjectList stObjectList;
    stObjectList.Add(
            {
                {"type" , std::string("plaintext")},
                {"font" , std::to_string(m_Font)},
                {"size" , std::to_string(m_FontSize)},
                {"style", std::string(szStyle)},
                {"color", std::string(szColor)}
            }, m_Content.c_str());

    m_TokenBoard.Load(stObjectList);

    m_W = m_TokenBoard.W();
    m_H = m_TokenBoard.H();
}

std::string LabelBoard::Print() const
{
    return "";
}

std::string LabelBoard::PrintXML() const
{
    return "";
}
