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

bool XMLFunc::CheckTextLeaf(const tinyxml2::XMLNode *pNode)
{
    if(!pNode){
        throw std::invalid_argument(str_fflprintf(": Invalid argument: nullptr"));
    }

    if(!pNode->NoChildren()){
        return false;
    }

    return pNode->ToText() != nullptr;
}

bool XMLFunc::CheckEmojiLeaf(const tinyxml2::XMLNode *pNode)
{
    if(!pNode){
        throw std::invalid_argument(str_fflprintf(": Invalid argument: nullptr"));
    }

    if(!pNode->NoChildren()){
        return false;
    }

    if(!pNode->ToElement()){
        return false;
    }

    constexpr const char * szEmojiTags []
    {
        "EMOJI",
        "Emoji",
        "emoji",
    };

    for(size_t nIndex = 0; nIndex < std::extent<decltype(szEmojiTags)>::value; ++nIndex){
        if(std::strcmp(szEmojiTags[nIndex], pNode->ToElement()->Name()) == 0){
            return true;
        }
    }
    return false;
}

bool XMLFunc::CheckImageLeaf(const tinyxml2::XMLNode *pNode)
{
    if(!pNode){
        throw std::invalid_argument(str_fflprintf(": Invalid argument: nullptr"));
    }

    if(!pNode->NoChildren()){
        return false;
    }

    if(!pNode->ToElement()){
        return false;
    }

    constexpr const char * szImageTags []
    {
        "IMAGE",
        "Image",
        "image",
    };

    for(size_t nIndex = 0; nIndex < std::extent<decltype(szImageTags)>::value; ++nIndex){
        if(std::strcmp(szImageTags[nIndex], pNode->ToElement()->Name()) == 0){
            return true;
        }
    }
    return false;
}

bool XMLFunc::CheckValidLeaf(const tinyxml2::XMLNode *pNode)
{
    if(!pNode){
        throw std::invalid_argument(str_fflprintf(": Invalid argument: nullptr"));
    }

    if(!pNode->NoChildren()){
        throw std::invalid_argument(str_fflprintf(": Invalid argument: not a leaf"));
    }

    return CheckTextLeaf(pNode) || CheckEmojiLeaf(pNode) || CheckImageLeaf(pNode);
}

const char *XMLFunc::FindAttribute(const tinyxml2::XMLNode *pNode, const char *szAttributeName, bool bRecursive)
{
    if(!pNode){
        throw std::invalid_argument(str_fflprintf(": Invalid argument: (nullptr)"));
    }

    for(; pNode; pNode = pNode->Parent()){
        if(auto pElement = pNode->ToElement()){
            if(auto szAttributeValue = pElement->Attribute(szAttributeName)){
                return szAttributeValue;
            }

            if(!bRecursive){
                return nullptr;
            }
        }
    }
    return nullptr;
}


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
