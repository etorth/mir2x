/*
 * =====================================================================================
 *
 *       Filename: xmlfunc.cpp
 *        Created: 12/11/2018 21:25:20
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
#include <utility>
#include <algorithm>
#include "xmlfunc.hpp"
#include "strfunc.hpp"

tinyxml2::XMLNode *XMLFunc::GetNextLeaf(tinyxml2::XMLNode *pNode)
{
    if(!pNode){
        throw std::invalid_argument(str_fflprintf(": Invalid argument: (nullptr)"));
    }

    if(!pNode->NoChildren()){
        throw std::invalid_argument(str_fflprintf(": Invalid argument: [%p] is not a leaf node", pNode));
    }

    for(auto pParent = pNode->Parent(); pParent; pParent = pParent->Parent()){
        if(auto pUncle = pParent->NextSibling()){
            return XMLFunc::GetNodeFirstLeaf(pUncle);
        }
    }
    return nullptr;
}

tinyxml2::XMLNode *XMLFunc::GetNodeFirstLeaf(tinyxml2::XMLNode *pNode)
{
    if(!pNode){
        throw std::invalid_argument(str_fflprintf(": Invalid argument: (nullptr)"));
    }

    while(!pNode->NoChildren()){
        pNode = pNode->FirstChild();
    }
    return pNode;
}

tinyxml2::XMLNode *XMLFunc::GetTreeFirstLeaf(tinyxml2::XMLNode *pNode)
{
    if(!pNode){
        throw std::invalid_argument(str_fflprintf(": Invalid argument: (nullptr)"));
    }
    return GetNodeFirstLeaf(pNode->GetDocument()->FirstChild());
}

tinyxml2::XMLNode *XMLFunc::GetNodeLastLeaf(tinyxml2::XMLNode *pNode)
{
    if(!pNode){
        throw std::invalid_argument(str_fflprintf(": Invalid argument: (nullptr)"));
    }

    while(!pNode->NoChildren()){
        pNode = pNode->LastChild();
    }
    return pNode;
}

tinyxml2::XMLNode *XMLFunc::GetTreeLastLeaf(tinyxml2::XMLNode *pNode)
{
    if(!pNode){
        throw std::invalid_argument(str_fflprintf(": Invalid argument: (nullptr)"));
    }
    return GetNodeLastLeaf(pNode->GetDocument()->LastChild());
}

bool XMLFunc::ValidTagName(const std::string &szTagName)
{
    if(szTagName.empty()){
        return false;
    }

    if(szTagName.find(" \t") != std::string::npos){
        return false;
    }

    return std::find_if_not(szTagName.begin(), szTagName.end(), [](char ch) -> bool
    {
        return (ch >='0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
    }) == szTagName.end();
}

bool XMLFunc::ValidAttributeName(const std::string &szAttributeName)
{
    if(szAttributeName.empty()){
        return false;
    }

    if(szAttributeName.find(" \t") != std::string::npos){
        return false;
    }

    return std::find_if_not(szAttributeName.begin(), szAttributeName.end(), [](char ch) -> bool
    {
        return (ch >='0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
    }) == szAttributeName.end();
}

std::string XMLFunc::BuildXMLString(const std::string &szTag, const std::string &szContent, const std::vector<std::pair<std::string, std::string>> &rstAttributeList)
{
    if(!XMLFunc::ValidTagName(szTag)){
        throw std::invalid_argument(str_printf(":Invalid tag name: %s", szTag.c_str()));
    }

    std::string szAttributeList;
    for(size_t nIndex = 0; nIndex < rstAttributeList.size(); ++nIndex){
        if(!XMLFunc::ValidAttributeName(rstAttributeList[nIndex].first)){
            throw std::invalid_argument(str_printf(":Invalid attribute name: %s", rstAttributeList[nIndex].first.c_str()));
        }
        szAttributeList += str_printf("%s%s=\"%s\"", (nIndex == 0) ? "" : " ", rstAttributeList[nIndex].first.c_str(), rstAttributeList[nIndex].second.c_str());
    }

    return str_printf("<%s %s>%s</%s>", szTag.c_str(), szAttributeList.c_str(), szContent.c_str(), szTag.c_str());
}
