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

XMLParagraph::XMLParagraph()
    : m_XMLDocument()
    , m_LeafList()
{}

void XMLParagraph::DeleteLeaf(int nLeaf)
{
    auto pNode   = leafRef(nLeaf).Node();
    auto pParent = pNode->Parent();

    while(pParent && (pParent->FirstChild() == pParent->LastChild())){
        pNode   = pParent;
        pParent = pParent->Parent();
    }

    if(pParent){
        pParent->DeleteChild(pNode);
    }
}

void XMLParagraph::InsertUTF8Char(int nLeaf, int nLeafOff, const char *szUTF8String)
{
    if(leafRef(nLeaf).Type() != LEAF_UTF8GROUP){
        throw std::invalid_argument(str_fflprintf(": The %zu-th leaf is not a XMLText", nLeaf));
    }

    if(nLeafOff >= (int)(leafRef(nLeaf).UTF8CharOffRef().size())){
        throw std::invalid_argument(str_fflprintf(": The %zu-th leaf has only %zu tokens", nLeaf, leafRef(nLeaf).UTF8CharOffRef().size()));
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

    auto pszOldValue    = leafRef(nLeaf).Node()->Value();
    auto szNewValue     = (std::string(pszOldValue, pszOldValue + nLeafOff) + szUTF8String) + std::string(pszOldValue + nLeafOff);
    auto stvNewValueOff = UTF8Func::buildUTF8Off(szNewValue.c_str());

    m_LeafList[nLeaf].Node()->SetValue(szNewValue.c_str());

    if(nLeafOff > 0){
        for(auto &rnOff: stvNewValueOff){
            rnOff += leafRef(nLeaf).UTF8CharOffRef()[nLeafOff];
        }
    }

    for(int nIndex = nLeafOff; nIndex < (int)(leafRef(nLeaf).UTF8CharOffRef().size()); ++nIndex){
        m_LeafList[nLeaf].UTF8CharOffRef()[nIndex] += (stvNewValueOff.back() + 1);
    }
    leafRef(nLeaf).UTF8CharOffRef().insert(leafRef(nLeaf).UTF8CharOffRef().begin(), stvNewValueOff.begin(), stvNewValueOff.end());
}

void XMLParagraph::DeleteUTF8Char(int nLeaf, int nLeafOff, int nTokenCount)
{
    if(leafRef(nLeaf).Type() != LEAF_UTF8GROUP){
        throw std::invalid_argument(str_fflprintf(": The %zu-th leaf is not a XMLText", nLeaf));
    }

    if(nLeafOff >= (int)(leafRef(nLeaf).UTF8CharOffRef().size())){
        throw std::invalid_argument(str_fflprintf(": The %zu-th leaf has only %zu tokens", nLeaf, leafRef(nLeaf).UTF8CharOffRef().size()));
    }

    if(nTokenCount == 0){
        return;
    }

    if((nLeafOff + nTokenCount - 1) >= (int)(leafRef(nLeaf).UTF8CharOffRef().size())){
        throw std::invalid_argument(str_fflprintf(": The %zu-th leaf has only %zu tokens", nLeaf, leafRef(nLeaf).UTF8CharOffRef().size()));
    }

    if(nTokenCount == (int)(leafRef(nLeaf).UTF8CharOffRef().size())){
        DeleteLeaf(nLeaf);
        return;
    }

    int nCharOff = leafRef(nLeaf).UTF8CharOffRef()[nLeafOff];
    int nCharLen = [this, nLeaf, nCharOff, nLeafOff, nTokenCount]() -> int
    {
        if(nLeafOff + nTokenCount >= (int)(leafRef(nLeaf).UTF8CharOffRef().size())){
            return (int)(leafRef(nLeaf).UTF8CharOffRef().size()) - nCharOff;
        }else{
            return leafRef(nLeaf).UTF8CharOffRef()[nLeafOff + nTokenCount] - nCharOff;
        }
    }();

    auto szNewValue = std::string(leafRef(nLeaf).Node()->Value()).erase(nCharOff, nCharLen);
    leafRef(nLeaf).Node()->SetValue(szNewValue.c_str());

    for(int nIndex = nLeafOff; nIndex + nLeafOff < nTokenCount; ++nIndex){
        leafRef(nLeaf).UTF8CharOffRef()[nIndex] = leafRef(nLeaf).UTF8CharOffRef()[nIndex + nLeafOff] - nCharLen;
    }
    leafRef(nLeaf).UTF8CharOffRef().resize(leafRef(nLeaf).UTF8CharOffRef().size() - nTokenCount);
}

void XMLParagraph::Delete(int nLeaf, int nLeafOff, int nTokenCount)
{
    if(nLeafOff >= (int)(leafRef(nLeaf).UTF8CharOffRef().size())){
        throw std::invalid_argument(str_fflprintf(": The %zu-th leaf has only %zu tokens", nLeaf, leafRef(nLeaf).UTF8CharOffRef().size()));
    }

    if(nTokenCount == 0){
        return;
    }

    int nLeftToken = leafRef(nLeaf).UTF8CharOffRef().size() - nLeafOff - 1;
    for(int nIndex = 0; nIndex < LeafCount(); ++nIndex){
        nLeftToken += leafRef(nIndex).UTF8CharOffRef().size();
    }

    if(nTokenCount >= nLeftToken + nLeafOff){
        throw std::invalid_argument(str_fflprintf(": The XMLParagraph has only %zu tokens after location: leafRef = %zu, LeafOff = %zu", nLeftToken, nLeaf, nLeafOff));
    }

    int nCurrLeaf     = nLeaf;
    int nCurrLeafOff  = nLeafOff;
    int nDeletedToken = 0;

    while(nDeletedToken < nTokenCount){
        switch(auto nType = leafRef(nCurrLeaf).Type()){
            case LEAF_UTF8GROUP:
                {
                    int nNeedDelete = std::min<int>(leafRef(nCurrLeaf).UTF8CharOffRef().size() - nCurrLeafOff, nTokenCount - nDeletedToken);
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

std::tuple<int, int, int> XMLParagraph::PrevLeafOff(int nLeaf, int nLeafOff, int) const
{
    if(nLeafOff >= (int)(leafRef(nLeaf).UTF8CharOffRef().size())){
        throw std::invalid_argument(str_fflprintf(": The %zu-th leaf has only %zu tokens", nLeaf, leafRef(nLeaf).UTF8CharOffRef().size()));
    }

    return {0, 0, 0};
}

std::tuple<int, int, int> XMLParagraph::NextLeafOff(int nLeaf, int nLeafOff, int nTokenCount) const
{
    if(nLeafOff >= leafRef(nLeaf).Length()){
        throw std::invalid_argument(str_fflprintf(": The %zu-th leaf has only %zu tokens", nLeaf, leafRef(nLeaf).Length()));
    }

    int nCurrLeaf      = nLeaf;
    int nCurrLeafOff   = nLeafOff;
    int nAdvancedToken = 0;

    while((nAdvancedToken < nTokenCount) && (nCurrLeaf < LeafCount())){
        switch(auto nType = leafRef(nCurrLeaf).Type()){
            case LEAF_EMOJI:
            case LEAF_IMAGE:
            case LEAF_UTF8GROUP:
                {
                    int nCurrTokenLeft = leafRef(nCurrLeaf).Length() - nCurrLeafOff - 1;
                    if(nCurrTokenLeft >= nTokenCount - nAdvancedToken){
                        return {nCurrLeaf, nCurrLeafOff + (nTokenCount - nAdvancedToken), nTokenCount};
                    }

                    // current leaf is not enough
                    // but this is the last leaf, have to stop
                    if(nCurrLeaf == (LeafCount() - 1)){
                        return {nCurrLeaf, leafRef(nCurrLeaf).Length() - 1, nAdvancedToken + nCurrTokenLeft};
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
const tinyxml2::XMLNode *XMLParagraph::LeafCommonAncestor(int, int) const
{
    return nullptr;
}

tinyxml2::XMLNode *XMLParagraph::Clone(tinyxml2::XMLDocument *pDoc, int nLeaf, int nLeafOff, int nTokenCount)
{
    auto [nEndLeaf, nEndLeafOff, nAdvancedToken] = NextLeafOff(nLeaf, nLeafOff, nTokenCount);
    if(nAdvancedToken != nTokenCount){
        // reach end before finish the given count
    }

    auto pClone = LeafCommonAncestor(nLeaf, nEndLeaf)->DeepClone(pDoc);

    if(nLeafOff != 0){
        if(leafRef(nLeaf).Type() != LEAF_UTF8GROUP){
            throw std::runtime_error(str_fflprintf(": Non-utf8 string leaf contains multiple tokens"));
        }

        // make a copy here, for safe
        // tinyxml2 doesn't specify if SetValue(Value()) works
        auto pCloneLeaf = XMLFunc::GetTreeFirstLeaf(pClone);
        auto szNewValue = std::string(pCloneLeaf->Value() + leafRef(nLeaf).UTF8CharOffRef()[nLeafOff]);
        pCloneLeaf->SetValue(szNewValue.c_str());
    }

    if(nEndLeafOff != (leafRef(nEndLeaf).Length() - 1)){
        if(leafRef(nEndLeaf).Type() != LEAF_UTF8GROUP){
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

void XMLParagraph::loadXML(const char *szXMLString)
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
        if(XMLFunc::CheckValidLeaf(pNode)){
            m_LeafList.emplace_back(pNode);
        }
    }
}

void XMLParagraph::loadXMLNode(const tinyxml2::XMLNode *node)
{
    if(!node){
        throw fflerror("null node pointer");
    }

    if(!node->ToElement()){
        throw fflerror("given node is not an element");
    }

    bool parXML = false;
    for(const char *cstr: {"par", "Par", "PAR"}){
        if(std::string(node->Value()) == cstr){
            parXML = true;
            break;
        }
    }

    if(!parXML){
        throw fflerror("not a paragraph node");
    }

    m_XMLDocument.Clear();
    if(auto pNew = node->DeepClone(&m_XMLDocument); pNew){
        m_XMLDocument.InsertEndChild(pNew);
    }
    else{
        throw fflerror("copy paragraph node failed");
    }

    m_LeafList.clear();
    for(auto pNode = XMLFunc::GetTreeFirstLeaf(m_XMLDocument.FirstChild()); pNode; pNode = XMLFunc::GetNextLeaf(pNode)){
        if(XMLFunc::CheckValidLeaf(pNode)){
            m_LeafList.emplace_back(pNode);
        }
    }
}
