/*
 * =====================================================================================
 *
 *       Filename: xmlf.cpp
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
#include "typecast.hpp"
#include "strf.hpp"
#include "xmlf.hpp"
#include "fflerror.hpp"

bool xmlf::checkTextLeaf(const tinyxml2::XMLNode *node)
{
    if(!node){
        throw fflerror("invalid argument: nullptr");
    }

    if(!node->NoChildren()){
        return false;
    }

    return node->ToText() != nullptr;
}

bool xmlf::checkEmojiLeaf(const tinyxml2::XMLNode *node)
{
    if(!node){
        throw fflerror("invalid argument: nullptr");
    }

    if(!node->NoChildren()){
        return false;
    }

    if(!node->ToElement()){
        return false;
    }

    constexpr const char * emojiTags []
    {
        "EMOJI",
        "Emoji",
        "emoji",
    };

    for(size_t i = 0; i < std::extent<decltype(emojiTags)>::value; ++i){
        if(std::strcmp(emojiTags[i], node->ToElement()->Name()) == 0){
            return true;
        }
    }
    return false;
}

bool xmlf::checkImageLeaf(const tinyxml2::XMLNode *node)
{
    if(!node){
        throw fflerror("invalid argument: nullptr");
    }

    if(!node->NoChildren()){
        return false;
    }

    if(!node->ToElement()){
        return false;
    }

    constexpr const char * imageTags []
    {
        "IMAGE",
        "Image",
        "image",
    };

    for(size_t i = 0; i < std::extent<decltype(imageTags)>::value; ++i){
        if(std::strcmp(imageTags[i], node->ToElement()->Name()) == 0){
            return true;
        }
    }
    return false;
}

bool xmlf::checkValidLeaf(const tinyxml2::XMLNode *node)
{
    if(!node){
        throw fflerror("invalid argument: nullptr");
    }

    if(!node->NoChildren()){
        throw fflerror("invalid argument: not a leaf");
    }

    return checkTextLeaf(node) || checkEmojiLeaf(node) || checkImageLeaf(node);
}

const char *xmlf::findAttribute(const tinyxml2::XMLNode *node, const char *attributeName, bool recursive)
{
    if(!node){
        throw fflerror("invalid argument: (nullptr)");
    }

    for(; node; node = node->Parent()){
        if(auto element = node->ToElement()){
            if(auto attributeValue = element->Attribute(attributeName)){
                return attributeValue;
            }

            if(!recursive){
                return nullptr;
            }
        }
    }
    return nullptr;
}


tinyxml2::XMLNode *xmlf::getNextLeaf(tinyxml2::XMLNode *node)
{
    if(!node){
        throw fflerror("invalid argument: (nullptr)");
    }

    if(!node->NoChildren()){
        throw fflerror("invalid argument: [%p] is not a leaf node", to_cvptr(node));
    }

    if(auto next = node->NextSibling()){
        return xmlf::getNodeFirstLeaf(next);
    }

    for(auto parent = node->Parent(); parent; parent = parent->Parent()){
        if(auto uncle = parent->NextSibling()){
            return xmlf::getNodeFirstLeaf(uncle);
        }
    }
    return nullptr;
}

tinyxml2::XMLNode *xmlf::getNodeFirstLeaf(tinyxml2::XMLNode *node)
{
    if(!node){
        throw fflerror("invalid argument: (nullptr)");
    }

    while(!node->NoChildren()){
        node = node->FirstChild();
    }
    return node;
}

tinyxml2::XMLNode *xmlf::getTreeFirstLeaf(tinyxml2::XMLNode *node)
{
    if(!node){
        throw fflerror("invalid argument: (nullptr)");
    }
    return getNodeFirstLeaf(node->GetDocument()->FirstChild());
}

tinyxml2::XMLNode *xmlf::getNodeLastLeaf(tinyxml2::XMLNode *node)
{
    if(!node){
        throw fflerror("invalid argument: (nullptr)");
    }

    while(!node->NoChildren()){
        node = node->LastChild();
    }
    return node;
}

tinyxml2::XMLNode *xmlf::getTreeLastLeaf(tinyxml2::XMLNode *node)
{
    if(!node){
        throw fflerror("invalid argument: (nullptr)");
    }
    return getNodeLastLeaf(node->GetDocument()->LastChild());
}

bool xmlf::validTagName(const std::string &tagName)
{
    if(tagName.empty()){
        return false;
    }

    if(tagName.find(" \t") != std::string::npos){
        return false;
    }

    return std::find_if_not(tagName.begin(), tagName.end(), [](char ch) -> bool
    {
        return (ch >='0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
    }) == tagName.end();
}

bool xmlf::validAttributeName(const std::string &attributeName)
{
    if(attributeName.empty()){
        return false;
    }

    if(attributeName.find(" \t") != std::string::npos){
        return false;
    }

    return std::find_if_not(attributeName.begin(), attributeName.end(), [](char ch) -> bool
    {
        return (ch >='0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
    }) == attributeName.end();
}

std::string xmlf::buildXMLString(const std::string &tagName, const std::string &content, const std::vector<std::pair<std::string, std::string>> &attributeList)
{
    if(!xmlf::validTagName(tagName)){
        throw fflerror("invalid tag name: %s", tagName.c_str());
    }

    std::string attributeString;
    for(size_t i = 0; i < attributeList.size(); ++i){
        if(!xmlf::validAttributeName(attributeList[i].first)){
            throw fflerror("invalid attribute name: %s", attributeList[i].first.c_str());
        }
        attributeString += str_printf("%s%s=\"%s\"", (i == 0) ? "" : " ", attributeList[i].first.c_str(), attributeList[i].second.c_str());
    }
    return str_printf("<%s %s>%s</%s>", tagName.c_str(), attributeString.c_str(), content.c_str(), tagName.c_str());
}
