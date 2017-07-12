/*
 * =====================================================================================
 *
 *       Filename: label.cpp
 *        Created: 08/12/2015 09:59:15
 *  Last Modified: 07/11/2017 16:33:46
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

void Label::SetText(const char * szFormatStr, ...)
{
    // 1. store parameter as m_Content
    // 2. build the token board for drawing
    auto fnBuildBoard = [this](const char *szMessage) -> void
    {
        m_Content = szMessage ? szMessage : "";

        char szStyle[16];
        std::sprintf(szStyle, "0X%02X", (m_FontKey & 0X00FF0000) >> 16);

        char szColor[16];
        std::sprintf(szColor, "0X%08X", ColorFunc::Color2U32ARGB(m_Color));

        XMLObjectList stObjectList;
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
    };

    int nLogSize = 0;

    // 1. try static buffer
    //    give a enough size so we can hopefully stop here
    {
        char szSBuf[1024];

        va_list ap;
        va_start(ap, szFormatStr);
        nLogSize = std::vsnprintf(szSBuf, (sizeof(szSBuf) / sizeof(szSBuf[0])), szFormatStr, ap);
        va_end(ap);

        if(nLogSize >= 0){
            if((size_t)(nLogSize + 1) < (sizeof(szSBuf) / sizeof(szSBuf[0]))){
                fnBuildBoard(szSBuf);
                return;
            }else{
                // do nothing
                // have to try the dynamic buffer method
            }
        }else{
            fnBuildBoard((std::string("Parse fromatted string failed: ") + szFormatStr).c_str());
            return;
        }
    }

    // 2. try dynamic buffer
    //    use the parsed buffer size above to get enough memory
    while(true){
        std::vector<char> szDBuf(nLogSize + 1 + 64);

        va_list ap;
        va_start(ap, szFormatStr);
        nLogSize = std::vsnprintf(&(szDBuf[0]), szDBuf.size(), szFormatStr, ap);
        va_end(ap);

        if(nLogSize >= 0){
            if((size_t)(nLogSize + 1) < szDBuf.size()){
                fnBuildBoard(&(szDBuf[0]));
                return;
            }else{
                szDBuf.resize(nLogSize + 1 + 64);
            }
        }else{
            fnBuildBoard((std::string("Parse fromatted string failed: ") + szFormatStr).c_str());
            return;
        }
    }
}
