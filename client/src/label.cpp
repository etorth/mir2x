/*
 * =====================================================================================
 *
 *       Filename: label.cpp
 *        Created: 08/12/2015 09:59:15
 *  Last Modified: 04/01/2016 11:57:21
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

void Label::SetText(const char * szInfo)
{
    // TODO
    // 1. support color/style
    // 2. clear tokenboard for multiply-load
    // 3. push_back for tokanboard
    //
    if(szInfo){
        m_Content = szInfo;
    }

    std::string szXMLInfo = "<root><object type=\"eventtext\"";

    // set font index
    szXMLInfo += " font=\"";
    szXMLInfo += std::to_string((m_FontKey & 0X000000FF) >> 0);
    szXMLInfo += "\"";

    szXMLInfo += " size=\"";
    szXMLInfo += std::to_string((m_FontKey & 0X0000FF00) >> 8);
    szXMLInfo += "\"";

    szXMLInfo += ">";

    szXMLInfo += m_Content;

    szXMLInfo += "</object></root>";

    tinyxml2::XMLDocument stDoc;
    stDoc.Parse(szXMLInfo.c_str());

    std::unordered_map<std::string, std::function<void()>> fnIDFuncMap;
    m_TokenBoard.Load(stDoc, fnIDFuncMap);
}
