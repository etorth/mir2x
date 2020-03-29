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
    , m_leafList()
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

void XMLParagraph::insertUTF8String(int leaf, int leafOff, const char *utf8String)
{
    if(leafRef(leaf).Type() != LEAF_UTF8GROUP){
        throw fflerror("the %zu-th leaf is not a XMLText", leaf);
    }

    if(leafOff < 0 || leafOff > (int)(leafRef(leaf).utf8CharOffRef().size())){
        throw fflerror("leaf offset %d is not a valid insert offset", leafOff);
    }

    if(utf8String == nullptr){
        throw fflerror("null utf8 text string");
    }

    if(std::strlen(utf8String) == 0){
        return;
    }

    if(!utf8::is_valid(utf8String, utf8String + std::strlen(utf8String))){
        throw fflerror("invalid utf-8 string: %s", utf8String);
    }

    const auto oldValue = leafRef(leaf).Node()->Value();
    const auto newValue = (std::string(oldValue, oldValue + leafOff) + utf8String) + std::string(oldValue + leafOff);
    m_leafList[leaf].Node()->SetValue(newValue.c_str());

    auto addedValueOff = UTF8Func::buildUTF8Off(utf8String);
    if(leafOff > 0){
        for(auto &elemRef: addedValueOff){
            elemRef += leafRef(leaf).utf8CharOffRef()[leafOff - 1];
        }
    }

    for(int i = leafOff; i < (int)(leafRef(leaf).utf8CharOffRef().size()); ++i){
        m_leafList[leaf].utf8CharOffRef()[i] += (addedValueOff.back() + 1);
    }
    leafRef(leaf).utf8CharOffRef().insert(leafRef(leaf).utf8CharOffRef().begin() + leafOff, addedValueOff.begin(), addedValueOff.end());
}

void XMLParagraph::DeleteUTF8Char(int nLeaf, int nLeafOff, int nTokenCount)
{
    if(leafRef(nLeaf).Type() != LEAF_UTF8GROUP){
        throw std::invalid_argument(str_fflprintf(": The %zu-th leaf is not a XMLText", nLeaf));
    }

    if(nLeafOff >= (int)(leafRef(nLeaf).utf8CharOffRef().size())){
        throw std::invalid_argument(str_fflprintf(": The %zu-th leaf has only %zu tokens", nLeaf, leafRef(nLeaf).utf8CharOffRef().size()));
    }

    if(nTokenCount == 0){
        return;
    }

    if((nLeafOff + nTokenCount - 1) >= (int)(leafRef(nLeaf).utf8CharOffRef().size())){
        throw std::invalid_argument(str_fflprintf(": The %zu-th leaf has only %zu tokens", nLeaf, leafRef(nLeaf).utf8CharOffRef().size()));
    }

    if(nTokenCount == (int)(leafRef(nLeaf).utf8CharOffRef().size())){
        DeleteLeaf(nLeaf);
        return;
    }

    int nCharOff = leafRef(nLeaf).utf8CharOffRef()[nLeafOff];
    int nCharLen = [this, nLeaf, nCharOff, nLeafOff, nTokenCount]() -> int
    {
        if(nLeafOff + nTokenCount >= (int)(leafRef(nLeaf).utf8CharOffRef().size())){
            return (int)(leafRef(nLeaf).utf8CharOffRef().size()) - nCharOff;
        }else{
            return leafRef(nLeaf).utf8CharOffRef()[nLeafOff + nTokenCount] - nCharOff;
        }
    }();

    auto szNewValue = std::string(leafRef(nLeaf).Node()->Value()).erase(nCharOff, nCharLen);
    leafRef(nLeaf).Node()->SetValue(szNewValue.c_str());

    for(int nIndex = nLeafOff; nIndex + nLeafOff < nTokenCount; ++nIndex){
        leafRef(nLeaf).utf8CharOffRef()[nIndex] = leafRef(nLeaf).utf8CharOffRef()[nIndex + nLeafOff] - nCharLen;
    }
    leafRef(nLeaf).utf8CharOffRef().resize(leafRef(nLeaf).utf8CharOffRef().size() - nTokenCount);
}

void XMLParagraph::Delete(int nLeaf, int nLeafOff, int nTokenCount)
{
    if(nLeafOff >= (int)(leafRef(nLeaf).utf8CharOffRef().size())){
        throw std::invalid_argument(str_fflprintf(": The %zu-th leaf has only %zu tokens", nLeaf, leafRef(nLeaf).utf8CharOffRef().size()));
    }

    if(nTokenCount == 0){
        return;
    }

    int nLeftToken = leafRef(nLeaf).utf8CharOffRef().size() - nLeafOff - 1;
    for(int nIndex = 0; nIndex < leafCount(); ++nIndex){
        nLeftToken += leafRef(nIndex).utf8CharOffRef().size();
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
                    int nNeedDelete = std::min<int>(leafRef(nCurrLeaf).utf8CharOffRef().size() - nCurrLeafOff, nTokenCount - nDeletedToken);
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
    if(nLeafOff >= (int)(leafRef(nLeaf).utf8CharOffRef().size())){
        throw std::invalid_argument(str_fflprintf(": The %zu-th leaf has only %zu tokens", nLeaf, leafRef(nLeaf).utf8CharOffRef().size()));
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

    while((nAdvancedToken < nTokenCount) && (nCurrLeaf < leafCount())){
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
                    if(nCurrLeaf == (leafCount() - 1)){
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
        auto szNewValue = std::string(pCloneLeaf->Value() + leafRef(nLeaf).utf8CharOffRef()[nLeafOff]);
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

void XMLParagraph::loadXML(const char *xmlString)
{
    if(xmlString == nullptr){
        throw fflerror("null xml string");
    }

    if(!utf8::is_valid(xmlString, xmlString + std::strlen(xmlString))){
        throw fflerror("xml is not a valid utf8 string: %s", xmlString);
    }

    tinyxml2::XMLDocument xmlDoc;
    if(xmlDoc.Parse(xmlString) != tinyxml2::XML_SUCCESS){
        throw fflerror("tinyxml2::XMLDocument::Parse() failed: %s", xmlString);
    }

    loadXMLNode(xmlDoc.RootElement());
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

    m_leafList.clear();
    for(auto pNode = XMLFunc::GetTreeFirstLeaf(m_XMLDocument.FirstChild()); pNode; pNode = XMLFunc::GetNextLeaf(pNode)){
        if(XMLFunc::CheckValidLeaf(pNode)){
            m_leafList.emplace_back(pNode);
        }
    }
}
