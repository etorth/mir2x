/*
 * =====================================================================================
 *
 *       Filename: xmlparagraphleaf.cpp
 *        Created: 12/21/2018 02:45:29
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

#include <utf8.h>
#include <stdexcept>
#include "bevent.hpp"
#include "strfunc.hpp"
#include "utf8func.hpp"
#include "xmlparagraphleaf.hpp"

bool isTextLeaf(const tinyxml2::XMLNode *pNode)
{
    if(!pNode){
        throw std::invalid_argument(str_fflprintf(": Invalid argument: nullptr"));
    }

    if(!pNode->NoChildren()){
        return false;
    }

    return pNode->ToText() != nullptr;
}

bool isEmojiLeaf(const tinyxml2::XMLNode *pNode)
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

    for(size_t nIndex = 0; std::extent<decltype(szEmojiTags)>::value; ++nIndex){
        if(std::strcmp(szEmojiTags[nIndex], pNode->ToElement()->Name())){
            return true;
        }
    }
    return false;
}

bool isImageLeaf(const tinyxml2::XMLNode *pNode)
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

    for(size_t nIndex = 0; std::extent<decltype(szImageTags)>::value; ++nIndex){
        if(std::strcmp(szImageTags[nIndex], pNode->ToElement()->Name())){
            return true;
        }
    }
    return false;
}

bool isValidLeaf(const tinyxml2::XMLNode *pNode)
{
    if(!pNode){
        throw std::invalid_argument(str_fflprintf(": Invalid argument: nullptr"));
    }

    if(!pNode->NoChildren()){
        throw std::invalid_argument(str_fflprintf(": Invalid argument: not a leaf"));
    }

    return isTextLeaf(pNode) || isEmojiLeaf(pNode) || isImageLeaf(pNode);
}

XMLParagraphLeaf::XMLParagraphLeaf(tinyxml2::XMLNode *pNode)
    : m_Type(LEAF_UTF8GROUP)
    , m_Node(pNode)
    , m_UTF8CharOff()
    , m_Event(BEVENT_OFF)
{
    if(pNode == nullptr){
        throw std::invalid_argument(str_fflprintf(": Invalid argument: (nullptr)"));
    }

    if(!isValidLeaf(pNode)){
        throw std::invalid_argument(str_fflprintf(": Invalid argument: not a valid XMParagraph leaf"));
    }

    m_Node = pNode;

    if(isTextLeaf(pNode)){
        if(!utf8::is_valid(pNode->Value(), pNode->Value() + std::strlen(pNode->Value()))){
            throw std::invalid_argument(str_fflprintf(": Invalid argument: not a utf8 string"));
        }

        m_Type        = LEAF_UTF8GROUP;
        m_UTF8CharOff = UTF8Func::BuildOff(pNode->Value());
        return;
    }

    else if(isEmojiLeaf(pNode)){
        m_Type = LEAF_EMOJI;
        return;
    }

    else if(isImageLeaf(pNode)){
        m_Type = LEAF_IMAGE;
        return;
    }

    throw std::invalid_argument(str_fflprintf(": Invalid argument: node type not recognized"));
}

void XMLParagraphLeaf::MarkEvent(int nEvent)
{
    switch(nEvent){
        case BEVENT_ON:
        case BEVENT_OFF:
        case BEVENT_DOWN:
            {
                break;
            }
        default:
            {
                throw std::invalid_argument(str_fflprintf(": Invalid event: %d", nEvent));
            }
    }
    m_Event = nEvent;
}

bool XMLParagraphLeaf::CheckXMLNodeAsLeaf(const tinyxml2::XMLNode *pNode)
{
    return isValidLeaf(pNode);
}

uint32_t XMLParagraphLeaf::PeekUTF8Code(size_t nLeafOff) const
{
    if(nLeafOff >= Length()){
        throw std::invalid_argument(str_fflprintf(": Provided LeafOff exceeds leaf length: %zu", Length()));
    }

    if(Type() != LEAF_UTF8GROUP){
        throw std::runtime_error(str_fflprintf(": Try peek utf8 code from a leaf with type: %d", Type()));
    }

    return UTF8Func::PeekUTF8Code(Node()->Value() + UTF8CharOffRef()[nLeafOff]);
}
