/*
 * =====================================================================================
 *
 *       Filename: linebrowserboard.cpp
 *        Created: 07/12/2017 23:20:41
 *  Last Modified: 07/14/2017 21:42:30
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
    return true
        && m_TokenBoard.AppendXML("<ROOT><OBJECT TYPE=\"RETURN\"></OBJECT></ROOT>", {})
        && m_TokenBoard.AppendXML(szXML, rstMap);
}

void LineBrowserBoard::DrawEx(int nDstX, int nDstY, int nSrcX, int nSrcY, int nW, int nH)
{
    m_TokenBoard.DrawEx(nDstX, nDstY, nSrcX, nSrcY, nW, nH);
}
