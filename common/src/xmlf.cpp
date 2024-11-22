#include <utility>
#include <algorithm>
#include "totype.hpp"
#include "strf.hpp"
#include "xmlf.hpp"
#include "fflerror.hpp"

bool xmlf::checkTextLeaf(const tinyxml2::XMLNode *node)
{
    fflassert(node);

    if(!node->NoChildren()){
        return false;
    }

    return node->ToText() != nullptr;
}

bool xmlf::checkEmojiLeaf(const tinyxml2::XMLNode *node)
{
    fflassert(node);

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
    fflassert(node);

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
    fflassert(node);
    fflassert(node->NoChildren());

    return checkTextLeaf(node) || checkEmojiLeaf(node) || checkImageLeaf(node);
}

const char *xmlf::findAttribute(const tinyxml2::XMLNode *node, const char *attributeName, bool recursive)
{
    fflassert(node);

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

bool xmlf::hasChild(tinyxml2::XMLNode *root, tinyxml2::XMLNode *child)
{
    fflassert(root);
    fflassert(child);

    while(child){
        if(child == root){
            return true;
        }
        child = child->Parent();
    }
    return false;
}

tinyxml2::XMLNode *xmlf::getNextLeaf(tinyxml2::XMLNode *node, tinyxml2::XMLNode *root)
{
    fflassert(node);
    fflassert(node->NoChildren());

    if(root){
        fflassert(xmlf::hasChild(root, node));
    }

    while(node && (node != root)){
        if(auto next = node->NextSibling()){
            return xmlf::getNodeFirstLeaf(next);
        }
        node = node->Parent();
    }
    return nullptr;
}

tinyxml2::XMLNode *xmlf::getNodeFirstLeaf(tinyxml2::XMLNode *node)
{
    fflassert(node);

    while(!node->NoChildren()){
        node = node->FirstChild();
    }
    return node;
}

tinyxml2::XMLNode *xmlf::getNodeLastLeaf(tinyxml2::XMLNode *node)
{
    fflassert(node);

    while(!node->NoChildren()){
        node = node->LastChild();
    }
    return node;
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

std::string xmlf::toParString(const char *format, ...)
{
    fflassert(format);
    std::string text;
    str_format(format, text);

    tinyxml2::XMLPrinter printer;
    tinyxml2::XMLDocument xmlDoc(true, tinyxml2::PEDANTIC_WHITESPACE);
    const char *xmlString = "<par></par>";

    if(xmlDoc.Parse(xmlString) != tinyxml2::XML_SUCCESS){
        throw fflerror("failed to parse xml template: %s", xmlString);
    }

    xmlDoc.RootElement()->SetText(text.c_str());
    xmlDoc.Print(&printer);
    return printer.CStr();
}

std::string xmlf::toString(const tinyxml2::XMLNode *node)
{
    fflassert(node);
    tinyxml2::XMLPrinter printer;
    node->Accept(&printer);
    return printer.CStr();
}
