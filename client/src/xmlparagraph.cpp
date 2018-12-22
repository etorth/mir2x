/*
 * =====================================================================================
 *
 *       Filename: xmlparagraph.cpp
 *        Created: 12/11/2018 04:20:39
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

#include <tuple>
#include <utf8.h>
#include <stdexcept>
#include <tinyxml2.h>

#include "log.hpp"
#include "bevent.hpp"
#include "strfunc.hpp"
#include "xmlfunc.hpp"
#include "utf8func.hpp"
#include "xmlparagraph.hpp"

extern Log *g_Log;

static bool isTextLeaf(const tinyxml2::XMLNode *pNode)
{
    if(!pNode){
        throw std::invalid_argument(str_fflprintf(": Invalid argument: nullptr"));
    }

    if(!pNode->NoChildren()){
        return false;
    }

    return pNode->ToText() != nullptr;
}

static bool isEmojiLeaf(const tinyxml2::XMLNode *pNode)
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

static bool isImageLeaf(const tinyxml2::XMLNode *pNode)
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

static bool isValidLeaf(const tinyxml2::XMLNode *pNode)
{
    if(!pNode){
        throw std::invalid_argument(str_fflprintf(": Invalid argument: nullptr"));
    }

    if(!pNode->NoChildren()){
        throw std::invalid_argument(str_fflprintf(": Invalid argument: not a leaf"));
    }

    return isTextLeaf(pNode) || isEmojiLeaf(pNode) || isImageLeaf(pNode);
}

static const char *findAttribute(const tinyxml2::XMLNode *pNode, const char *szAttributeName)
{
    if(!pNode){
        throw std::invalid_argument(str_fflprintf(": Invalid argument: (nullptr)"));
    }

    for(; pNode; pNode = pNode->Parent()){
        if(auto pElement = pNode->ToElement()){
            if(auto szAttributeValue = pElement->Attribute(szAttributeName)){
                return szAttributeValue;
            }
        }
    }
    return nullptr;
}

XMLParagraphLeaf::XMLParagraphLeaf(tinyxml2::XMLNode *pNode)
    : m_Node([pNode]()
      {
          if(pNode == nullptr){
              throw std::invalid_argument(str_fflprintf(": Invalid argument: (nullptr)"));
          }

          if(!isValidLeaf(pNode)){
              throw std::invalid_argument(str_fflprintf(": Invalid argument: not a valid XMParagraph leaf"));
          }
          return pNode;
      }())
    , m_Type([this]()
      {
          if(isTextLeaf(Node())){
              if(!utf8::is_valid(Node()->Value(), Node()->Value() + std::strlen(Node()->Value()))){
                  throw std::invalid_argument(str_fflprintf(": Not a utf8 string: %s", Node()->Value()));
              }
              return LEAF_UTF8GROUP;
          }

          if(isEmojiLeaf(Node())){
              return LEAF_EMOJI;
          }

          if(isImageLeaf(Node())){
              return LEAF_IMAGE;
          }

          throw std::invalid_argument(str_fflprintf(": Invalid argument: node type not recognized"));
      }())
    , m_U64Key(0)
    , m_UTF8CharOff()
    , m_Event(BEVENT_OFF)
{
    if(Type() == LEAF_UTF8GROUP){
        m_UTF8CharOff = UTF8Func::BuildUTF8Off(UTF8Text());
    }

    try{
        if(const auto *pszBGColor = findAttribute(Node(), "font_bgcolor")){
            m_BGColor.emplace(ColorFunc::String2RGBA(pszBGColor));
        }
    }catch(...){}
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

XMLParagraph::XMLParagraph()
    : m_XMLDocument()
    , m_LeafList()
{}

void XMLParagraph::DeleteLeaf(size_t nLeaf)
{
    auto pNode   = LeafRef(nLeaf).Node();
    auto pParent = pNode->Parent();

    while(pParent && (pParent->FirstChild() == pParent->LastChild())){
        pNode   = pParent;
        pParent = pParent->Parent();
    }

    if(pParent){
        pParent->DeleteChild(pNode);
    }
}

void XMLParagraph::InsertUTF8Char(size_t nLeaf, size_t nLeafOff, const char *szUTF8String)
{
    if(LeafRef(nLeaf).Type() != LEAF_UTF8GROUP){
        throw std::invalid_argument(str_fflprintf(": The %zu-th leaf is not a XMLText", nLeaf));
    }

    if(nLeafOff >= LeafRef(nLeaf).UTF8CharOffRef().size()){
        throw std::invalid_argument(str_fflprintf(": The %zu-th leaf has only %zu tokens", nLeaf, LeafRef(nLeaf).UTF8CharOffRef().size()));
    }

    if(szUTF8String == nullptr){
        throw std::invalid_argument(str_fflprintf(": Invalid argument: nullptr"));
    }

    if(std::strlen(szUTF8String) == 0){
        return;
    }

    if(!utf8::is_valid(szUTF8String, szUTF8String + std::strlen(szUTF8String))){
        throw std::invalid_argument(str_fflprintf(": Invalid utf-8 string: %s", szUTF8String));
    }

    auto pszOldValue    = LeafRef(nLeaf).Node()->Value();
    auto szNewValue     = (std::string(pszOldValue, pszOldValue + nLeafOff) + szUTF8String) + std::string(pszOldValue + nLeafOff);
    auto stvNewValueOff = UTF8Func::BuildUTF8Off(szNewValue.c_str());

    m_LeafList[nLeaf].Node()->SetValue(szNewValue.c_str());

    if(nLeafOff > 0){
        for(auto &rnOff: stvNewValueOff){
            rnOff += LeafRef(nLeaf).UTF8CharOffRef()[nLeafOff];
        }
    }

    for(size_t nIndex = nLeafOff; nIndex < LeafRef(nLeaf).UTF8CharOffRef().size(); ++nIndex){
        m_LeafList[nLeaf].UTF8CharOffRef()[nIndex] += (stvNewValueOff.back() + 1);
    }
    LeafRef(nLeaf).UTF8CharOffRef().insert(LeafRef(nLeaf).UTF8CharOffRef().begin(), stvNewValueOff.begin(), stvNewValueOff.end());
}

void XMLParagraph::DeleteUTF8Char(size_t nLeaf, size_t nLeafOff, size_t nTokenCount)
{
    if(LeafRef(nLeaf).Type() != LEAF_UTF8GROUP){
        throw std::invalid_argument(str_fflprintf(": The %zu-th leaf is not a XMLText", nLeaf));
    }

    if(nLeafOff >= LeafRef(nLeaf).UTF8CharOffRef().size()){
        throw std::invalid_argument(str_fflprintf(": The %zu-th leaf has only %zu tokens", nLeaf, LeafRef(nLeaf).UTF8CharOffRef().size()));
    }

    if(nTokenCount == 0){
        return;
    }

    if((nLeafOff + nTokenCount - 1) >= LeafRef(nLeaf).UTF8CharOffRef().size()){
        throw std::invalid_argument(str_fflprintf(": The %zu-th leaf has only %zu tokens", nLeaf, LeafRef(nLeaf).UTF8CharOffRef().size()));
    }

    if(nTokenCount == LeafRef(nLeaf).UTF8CharOffRef().size()){
        DeleteLeaf(nLeaf);
        return;
    }

    size_t nCharOff = LeafRef(nLeaf).UTF8CharOffRef()[nLeafOff];
    size_t nCharLen = [this, nLeaf, nCharOff, nLeafOff, nTokenCount]()
    {
        if(nLeafOff + nTokenCount >= LeafRef(nLeaf).UTF8CharOffRef().size()){
            return LeafRef(nLeaf).UTF8CharOffRef().size() - nCharOff;
        }else{
            return LeafRef(nLeaf).UTF8CharOffRef()[nLeafOff + nTokenCount] - nCharOff;
        }
    }();

    auto szNewValue = std::string(LeafRef(nLeaf).Node()->Value()).erase(nCharOff, nCharLen);
    LeafRef(nLeaf).Node()->SetValue(szNewValue.c_str());

    for(size_t nIndex = nLeafOff; nIndex + nLeafOff < nTokenCount; ++nIndex){
        LeafRef(nLeaf).UTF8CharOffRef()[nIndex] = LeafRef(nLeaf).UTF8CharOffRef()[nIndex + nLeafOff] - nCharLen;
    }
    LeafRef(nLeaf).UTF8CharOffRef().resize(LeafRef(nLeaf).UTF8CharOffRef().size() - nTokenCount);
}

void XMLParagraph::Delete(size_t nLeaf, size_t nLeafOff, size_t nTokenCount)
{
    if(nLeafOff >= LeafRef(nLeaf).UTF8CharOffRef().size()){
        throw std::invalid_argument(str_fflprintf(": The %zu-th leaf has only %zu tokens", nLeaf, LeafRef(nLeaf).UTF8CharOffRef().size()));
    }

    if(nTokenCount == 0){
        return;
    }

    size_t nLeftToken = LeafRef(nLeaf).UTF8CharOffRef().size() - nLeafOff - 1;
    for(size_t nIndex = 0; nIndex < LeafCount(); ++nIndex){
        nLeftToken += LeafRef(nIndex).UTF8CharOffRef().size();
    }

    if(nTokenCount >= nLeftToken + nLeafOff){
        throw std::invalid_argument(str_fflprintf(": The XMLParagraph has only %zu tokens after location: LeafRef = %zu, LeafOff = %zu", nLeftToken, nLeaf, nLeafOff));
    }

    size_t nCurrLeaf     = nLeaf;
    size_t nCurrLeafOff  = nLeafOff;
    size_t nDeletedToken = 0;

    while(nDeletedToken < nTokenCount){
        switch(auto nType = LeafRef(nCurrLeaf).Type()){
            case LEAF_UTF8GROUP:
                {
                    size_t nNeedDelete = std::min<size_t>(LeafRef(nCurrLeaf).UTF8CharOffRef().size() - nCurrLeafOff, nTokenCount - nDeletedToken);
                    DeleteUTF8Char(nCurrLeaf, nCurrLeafOff, nNeedDelete);
                    nDeletedToken += nNeedDelete;
                    break;
                }
            case LEAF_EMOJI:
            case LEAF_IMAGE:
                {
                    DeleteLeaf(nCurrLeaf);
                    nDeletedToken += 1;
                    break;
                }
            default:
                {
                    throw std::runtime_error(str_fflprintf(": Invalid leaf type: %d", nType));
                }
        }
    }
}

std::tuple<size_t, size_t, size_t> XMLParagraph::PrevLeafOff(size_t nLeaf, size_t nLeafOff, size_t) const
{
    if(nLeafOff >= LeafRef(nLeaf).UTF8CharOffRef().size()){
        throw std::invalid_argument(str_fflprintf(": The %zu-th leaf has only %zu tokens", nLeaf, LeafRef(nLeaf).UTF8CharOffRef().size()));
    }

    return {0, 0, 0};
}

std::tuple<size_t, size_t, size_t> XMLParagraph::NextLeafOff(size_t nLeaf, size_t nLeafOff, size_t nTokenCount) const
{
    if(nLeafOff >= LeafRef(nLeaf).Length()){
        throw std::invalid_argument(str_fflprintf(": The %zu-th leaf has only %zu tokens", nLeaf, LeafRef(nLeaf).Length()));
    }

    size_t nCurrLeaf      = nLeaf;
    size_t nCurrLeafOff   = nLeafOff;
    size_t nAdvancedToken = 0;

    while((nAdvancedToken < nTokenCount) && (nCurrLeaf < LeafCount())){
        switch(auto nType = LeafRef(nCurrLeaf).Type()){
            case LEAF_EMOJI:
            case LEAF_IMAGE:
            case LEAF_UTF8GROUP:
                {
                    size_t nCurrTokenLeft = LeafRef(nCurrLeaf).Length() - nCurrLeafOff - 1;
                    if(nCurrTokenLeft >= nTokenCount - nAdvancedToken){
                        return {nCurrLeaf, nCurrLeafOff + (nTokenCount - nAdvancedToken), nTokenCount};
                    }

                    // current leaf is not enough
                    // but this is the last leaf, have to stop
                    if(nCurrLeaf == (LeafCount() - 1)){
                        return {nCurrLeaf, LeafRef(nCurrLeaf).Length() - 1, nAdvancedToken + nCurrTokenLeft};
                    }

                    // take all the rest
                    // go to next leaf and take the first token
                    nCurrLeaf++;
                    nCurrLeafOff = 0;
                    nAdvancedToken += (nCurrTokenLeft + 1);
                    break;
                }
            default:
                {
                    throw std::runtime_error(str_fflprintf(": Invalid leaf type: %d", nType));
                }
        }
    }
    return {nCurrLeaf, nCurrLeafOff, nAdvancedToken};
}

// lowest common ancestor
const tinyxml2::XMLNode *XMLParagraph::LeafCommonAncestor(size_t, size_t) const
{
    return nullptr;
}

tinyxml2::XMLNode *XMLParagraph::Clone(tinyxml2::XMLDocument *pDoc, size_t nLeaf, size_t nLeafOff, size_t nTokenCount)
{
    auto [nEndLeaf, nEndLeafOff, nAdvancedToken] = NextLeafOff(nLeaf, nLeafOff, nTokenCount);
    if(nAdvancedToken != nTokenCount){
        // reach end before finish the given count
    }

    auto pClone = LeafCommonAncestor(nLeaf, nEndLeaf)->DeepClone(pDoc);

    if(nLeafOff != 0){
        if(LeafRef(nLeaf).Type() != LEAF_UTF8GROUP){
            throw std::runtime_error(str_fflprintf(": Non-utf8 string leaf contains multiple tokens"));
        }

        // make a copy here, for safe
        // tinyxml2 doesn't specify if SetValue(Value()) works
        auto pCloneLeaf = XMLFunc::GetTreeFirstLeaf(pClone);
        auto szNewValue = std::string(pCloneLeaf->Value() + LeafRef(nLeaf).UTF8CharOffRef()[nLeafOff]);
        pCloneLeaf->SetValue(szNewValue.c_str());
    }

    if(nEndLeafOff != (LeafRef(nEndLeaf).Length() - 1)){
        if(LeafRef(nEndLeaf).Type() != LEAF_UTF8GROUP){
            throw std::runtime_error(str_fflprintf(": Non-utf8 string leaf contains multiple tokens"));
        }

        auto pCloneLeaf = XMLFunc::GetTreeLastLeaf(pClone);
        auto szNewValue = std::string(pCloneLeaf->Value(), pCloneLeaf->Value() + nLeafOff);
        pCloneLeaf->SetValue(szNewValue.c_str());
    }

    return pClone;
}

void XMLParagraph::Join(const XMLParagraph &rstInput)
{
    if(rstInput.m_XMLDocument.FirstChild() == nullptr){
        return;
    }

    for(auto pNode = rstInput.m_XMLDocument.FirstChild()->FirstChild(); pNode; pNode = pNode->NextSibling()){
        m_XMLDocument.FirstChild()->InsertEndChild(pNode->DeepClone(m_XMLDocument.GetDocument()));
    }
}

void XMLParagraph::LoadXML(const char *szXMLString)
{
    if(szXMLString == nullptr){
        throw std::invalid_argument(str_fflprintf(": Invalid argument: (nullptr)"));
    }

    if(!utf8::is_valid(szXMLString, szXMLString + std::strlen(szXMLString))){
        throw std::invalid_argument(str_fflprintf(": Invalid argument: not an utf8 string: %s", szXMLString));
    }

    Clear();
    switch(auto nRC = m_XMLDocument.Parse(szXMLString)){
        case tinyxml2::XML_SUCCESS:
            {
                break;
            }
        default:
            {
                throw std::runtime_error(str_fflprintf(": tinyxml2::XMLDocument::Parse() failed: %d", nRC));
            }
    }

    for(auto pNode = XMLFunc::GetTreeFirstLeaf(m_XMLDocument.FirstChild()); pNode; pNode = XMLFunc::GetNextLeaf(pNode)){
        if(isValidLeaf(pNode)){
            m_LeafList.emplace_back(pNode);
        }
    }
}
