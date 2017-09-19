/*
 * =====================================================================================
 *
 *       Filename: linebrowserboard.cpp
 *        Created: 07/12/2017 23:20:41
 *  Last Modified: 09/18/2017 12:33:16
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

#include <cstring>
#include "linebrowserboard.hpp"

bool LineBrowserBoard::Add(const char *szContent)
{
    std::string szXMLContent;

    szXMLContent += "<ROOT><OBJECT>";
    szXMLContent += szContent ? szContent : "";
    szXMLContent += "</OBJECT></ROOT>";

    return AddXML(szXMLContent.c_str(), {});
}

bool LineBrowserBoard::AddXML(const char *szXML, const std::unordered_map<std::string, std::function<void()>> &rstMap)
{
    bool bRes = true;
    if(!m_TokenBoard.Empty(false)){
        bRes = m_TokenBoard.AppendXML("<ROOT><OBJECT TYPE=\"RETURN\"></OBJECT></ROOT>", {});
    }

    if(bRes){
        bRes = m_TokenBoard.AppendXML(szXML, rstMap);
    }

    return bRes;
}

void LineBrowserBoard::DrawEx(int nDstX, int nDstY, int nSrcX, int nSrcY, int nW, int nH)
{
    if(m_TokenBoard.H() <= H()){
        m_TokenBoard.DrawEx(nDstX, nDstY, nSrcX, nSrcY, nW, nH);
    }else{
        m_TokenBoard.DrawEx(nDstX, nDstY, nSrcX, nSrcY + (m_TokenBoard.H() - H()), nW, nH);
    }
}
