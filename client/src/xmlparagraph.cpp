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
#include "typecast.hpp"
#include "strf.hpp"
#include "xmlf.hpp"
#include "utf8f.hpp"
#include "xmlparagraph.hpp"

extern Log *g_log;

XMLParagraph::XMLParagraph()
    : m_XMLDocument()
    , m_leafList()
{}

void XMLParagraph::deleteLeaf(int leaf)
{
    auto node   = leafRef(leaf).xmlNode();
    auto parent = node->Parent();

    while(parent && (parent->FirstChild() == parent->LastChild())){
        node   = parent;
        parent = parent->Parent();
    }

    if(parent){
        parent->DeleteChild(node);
    }
    else{
        m_XMLDocument.Clear();
    }
    m_leafList.erase(m_leafList.begin() + leaf);
}

void XMLParagraph::insertUTF8String(int leaf, int leafOff, const char *utf8String)
{
    if(leafRef(leaf).Type() != LEAF_UTF8GROUP){
        throw fflerror("the %d-th leaf is not a XMLText", leaf);
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

    auto &utf8OffRef = leafRef(leaf).utf8CharOffRef();
    const auto [oldLength, newValue] = [leaf, leafOff, utf8String, &utf8OffRef, this]() -> std::tuple<int, std::string>
    {
        // after we call XMLNode::SetValue(), the oldValue pointer get updated
        // use lambda here on purpose to prevent oldValue pointer get exposed to outiler scope

        const auto oldValue  = leafRef(leaf).xmlNode()->Value();
        const auto oldLength = std::strlen(oldValue);

        if(leafOff >= (int)(utf8OffRef.size())){
            return {oldLength, std::string(oldValue) + utf8String};
        }
        return {oldLength, (std::string(oldValue, oldValue + utf8OffRef[leafOff]) + utf8String) + std::string(oldValue + utf8OffRef[leafOff])};
    }();
    m_leafList[leaf].xmlNode()->SetValue(newValue.c_str());

    auto addedValueOff = utf8f::buildUTF8Off(utf8String);
    if(leafOff > 0){
        const auto firstOffDiff = [leafOff, oldLength, &utf8OffRef]() -> int
        {
            if(leafOff < (int)(utf8OffRef.size())){
                return utf8OffRef[leafOff];
            }
            return oldLength;
        }();

        for(auto &elemRef: addedValueOff){
            elemRef += firstOffDiff;
        }
    }

    if(leafOff < (int)(utf8OffRef.size())){
        const int addedLength = std::strlen(utf8String);
        for(int i = leafOff; i < (int)(utf8OffRef.size()); ++i){
            utf8OffRef[i] += addedLength;
        }
    }
    utf8OffRef.insert(utf8OffRef.begin() + leafOff, addedValueOff.begin(), addedValueOff.end());
}

void XMLParagraph::deleteUTF8Char(int leaf, int leafOff, int tokenCount)
{
    if(leafRef(leaf).Type() != LEAF_UTF8GROUP){
        throw fflerror("the %d-th leaf is not a XMLText", leaf);
    }

    if(!leafOffValid(leaf, leafOff)){
        throw fflerror("invalid leafOff: %d", leafOff);
    }

    if(tokenCount == 0){
        return;
    }

    if((leafOff + tokenCount - 1) >= leafRef(leaf).length()){
        throw fflerror("the %d-th leaf has only %d tokens", leaf, leafRef(leaf).length());
    }

    if(tokenCount == leafRef(leaf).length()){
        deleteLeaf(leaf);
        return;
    }

    const int charOff = leafRef(leaf).utf8CharOffRef()[leafOff];
    const int charLen = [this, leaf, charOff, leafOff, tokenCount]() -> int
    {
        if(leafOff + tokenCount < (int)(leafRef(leaf).utf8CharOffRef().size())){
            return leafRef(leaf).utf8CharOffRef()[leafOff + tokenCount] - charOff;
        }
        return (int)(leafRef(leaf).utf8CharOffRef().size()) - charOff;
    }();

    const auto newValue = std::string(leafRef(leaf).xmlNode()->Value()).erase(charOff, charLen);
    leafRef(leaf).xmlNode()->SetValue(newValue.c_str());

    for(int i = leafOff; i + tokenCount < (int)(leafRef(leaf).utf8CharOffRef().size()); ++i){
        leafRef(leaf).utf8CharOffRef()[i] = leafRef(leaf).utf8CharOffRef()[i + tokenCount] - charLen;
    }
    leafRef(leaf).utf8CharOffRef().resize(leafRef(leaf).utf8CharOffRef().size() - tokenCount);
}

void XMLParagraph::deleteToken(int leaf, int leafOff, int tokenCount)
{
    if(!leafOffValid(leaf, leafOff)){
        throw fflerror("invalid leaf off: (%d, %d)", leaf, leafOff);
    }

    if(tokenCount == 0){
        return;
    }

    const bool hasEnoughToken = [leaf, leafOff, tokenCount, this]()
    {
        int tokenLeft = leafRef(leaf).length() - leafOff;
        if(tokenLeft >= tokenCount){
            return true;
        }

        for(int i = leaf + 1; i < leafCount(); ++i){
            tokenLeft += leafRef(i).length();
            if(tokenLeft >= tokenCount){
                return true;
            }
        }
        return false;
    }();

    if(!hasEnoughToken){
        throw fflerror("insufficient token to remove");
    }

    int currLeaf     = leaf;
    int currLeafOff  = leafOff;
    int deletedToken = 0;

    while(deletedToken < tokenCount){
        switch(leafRef(currLeaf).Type()){
            case LEAF_UTF8GROUP:
                {
                    const int needDelete = std::min<int>(leafRef(currLeaf).length() - currLeafOff, tokenCount - deletedToken);
                    deleteUTF8Char(currLeaf, currLeafOff, needDelete);
                    deletedToken += needDelete;
                    break;
                }
            case LEAF_EMOJI:
            case LEAF_IMAGE:
                {
                    deleteLeaf(currLeaf);
                    deletedToken += 1;
                    break;
                }
            default:
                {
                    throw fflerror("invalid leaf type: %d", leafRef(currLeaf).Type());
                }
        }

        currLeaf++;
        currLeafOff = 0;
    }
}

std::tuple<int, int, int> XMLParagraph::prevLeafOff(int leaf, int leafOff, int) const
{
    if(leafOff >= (int)(leafRef(leaf).utf8CharOffRef().size())){
        throw fflerror("the %d-th leaf has only %zu tokens", leaf, leafRef(leaf).utf8CharOffRef().size());
    }

    return {0, 0, 0};
}

std::tuple<int, int, int> XMLParagraph::nextLeafOff(int leaf, int leafOff, int tokenCount) const
{
    if(leafOff >= leafRef(leaf).length()){
        throw fflerror("the %d-th leaf has only %d tokens", leaf, leafRef(leaf).length());
    }

    int nCurrLeaf      = leaf;
    int nCurrLeafOff   = leafOff;
    int nAdvancedToken = 0;

    while((nAdvancedToken < tokenCount) && (nCurrLeaf < leafCount())){
        switch(auto nType = leafRef(nCurrLeaf).Type()){
            case LEAF_EMOJI:
            case LEAF_IMAGE:
            case LEAF_UTF8GROUP:
                {
                    int nCurrTokenLeft = leafRef(nCurrLeaf).length() - nCurrLeafOff - 1;
                    if(nCurrTokenLeft >= tokenCount - nAdvancedToken){
                        return {nCurrLeaf, nCurrLeafOff + (tokenCount - nAdvancedToken), tokenCount};
                    }

                    // current leaf is not enough
                    // but this is the last leaf, have to stop
                    if(nCurrLeaf == (leafCount() - 1)){
                        return {nCurrLeaf, leafRef(nCurrLeaf).length() - 1, nAdvancedToken + nCurrTokenLeft};
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
                    throw fflerror("invalid leaf type: %d", nType);
                }
        }
    }
    return {nCurrLeaf, nCurrLeafOff, nAdvancedToken};
}

// lowest common ancestor
const tinyxml2::XMLNode *XMLParagraph::leafCommonAncestor(int, int) const
{
    return nullptr;
}

tinyxml2::XMLNode *XMLParagraph::Clone(tinyxml2::XMLDocument *pDoc, int leaf, int leafOff, int tokenCount)
{
    auto [nEndLeaf, nEndLeafOff, nAdvancedToken] = nextLeafOff(leaf, leafOff, tokenCount);
    if(nAdvancedToken != tokenCount){
        // reach end before finish the given count
    }

    auto pClone = leafCommonAncestor(leaf, nEndLeaf)->DeepClone(pDoc);

    if(leafOff != 0){
        if(leafRef(leaf).Type() != LEAF_UTF8GROUP){
            throw fflerror("non-utf8 string leaf contains multiple tokens");
        }

        // make a copy here, for safe
        // tinyxml2 doesn't specify if SetValue(Value()) works
        auto pCloneLeaf = xmlf::getTreeFirstLeaf(pClone);
        auto szNewValue = std::string(pCloneLeaf->Value() + leafRef(leaf).utf8CharOffRef()[leafOff]);
        pCloneLeaf->SetValue(szNewValue.c_str());
    }

    if(nEndLeafOff != (leafRef(nEndLeaf).length() - 1)){
        if(leafRef(nEndLeaf).Type() != LEAF_UTF8GROUP){
            throw fflerror("non-utf8 string leaf contains multiple tokens");
        }

        auto pCloneLeaf = xmlf::getTreeLastLeaf(pClone);
        auto szNewValue = std::string(pCloneLeaf->Value(), pCloneLeaf->Value() + leafOff);
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
    for(auto pNode = xmlf::getTreeFirstLeaf(m_XMLDocument.FirstChild()); pNode; pNode = xmlf::getNextLeaf(pNode)){
        if(xmlf::checkValidLeaf(pNode)){
            m_leafList.emplace_back(pNode);
        }
    }
}

void XMLParagraph::insertLeafXML(int loc, const char *xmlString)
{
    if(loc < 0 || loc > leafCount() || !xmlString){
        throw fflerror("invalid argument: loc = %d, xmlString = %p", loc, xmlString);
    }
    insertXMLAfter(leafRef(loc).xmlNode(), xmlString);
}

void XMLParagraph::insertXMLAfter(tinyxml2::XMLNode *after, const char *xmlString)
{
    if(!(after && xmlString)){
        throw fflerror("invalid argument: after = %p, xmlString = %p", to_cvptr(after), to_cvptr(xmlString));
    }

    if(after->GetDocument() != &m_XMLDocument){
        throw fflerror("can't find after node in current XMLDocument");
    }

    // to insert XML as leaves, we requires to insert as a new paragraph
    // for emoji and image we have specified tag <emoji>, <image>, but for text we don't have

    // i don't want to define a <text> tag
    // instead I want to use <font> <underline> <strike> <color> for easier text control

    // example:
    //      <underline>hello world!</underline>
    // or:
    //      <text underline="true">hello world!</text>
    // looks same
    //
    // but:
    //      <underline> hello world, I like <emoji id="12"/> a lot!</underline>
    // is better than:
    //      <text underline="true">hello world, I like </text><emoji id="12"/><text> a lot!</text>
    //
    // <text> tag needs pure text while sometimes we may need to put emoji inside text

    tinyxml2::XMLDocument xmlDoc;
    if(xmlDoc.Parse(xmlString) != tinyxml2::XML_SUCCESS){
        throw fflerror("tinyxml2::XMLDocument::Parse() failed: %s", xmlString);
    }

    auto xmlRoot = xmlDoc.RootElement();
    if(!xmlRoot){
        throw fflerror("no root element");
    }

    bool parXML = false;
    for(const char *cstr: {"par", "Par", "PAR"}){
        if(std::string(xmlRoot->Value()) == cstr){
            parXML = true;
            break;
        }
    }

    if(!parXML){
        throw fflerror("xmlString is not a paragraph");
    }

    for(auto p = xmlRoot->FirstChild(); p; p = p->NextSibling()){
        if(auto cloneNode = p->DeepClone(&m_XMLDocument)){
            if(after->Parent()->InsertAfterChild(after, cloneNode) != cloneNode){
                throw fflerror("insert node failed");
            }
        }
        else{
            throw fflerror("deep clone node failed");
        }
    }

    // TODO rebuild everything
    // this is not necessary, optimize later

    m_leafList.clear();
    for(auto pNode = xmlf::getTreeFirstLeaf(m_XMLDocument.FirstChild()); pNode; pNode = xmlf::getNextLeaf(pNode)){
        if(xmlf::checkValidLeaf(pNode)){
            m_leafList.emplace_back(pNode);
        }
    }
}

void XMLParagraph::deleteToken(int leaf, int leafOff)
{
    switch(leafRef(leaf).Type()){
        case LEAF_UTF8GROUP:
            {
                deleteUTF8Char(leaf, leafOff, 1);
                return;
            }
        case LEAF_EMOJI:
        case LEAF_IMAGE:
            {
                deleteLeaf(leaf);
                return;
            }
        default:
            {
                throw fflerror("invalid leaf type: %d", leafRef(leaf).Type());
            }
    }
}

std::string XMLParagraph::getRawString() const
{
    std::string rawString;
    for(const auto &leaf: m_leafList){
        switch(leaf.Type()){
            case LEAF_UTF8GROUP:
                {
                    rawString += leaf.xmlNode()->Value();
                    break;
                }
            case LEAF_EMOJI:
            case LEAF_IMAGE:
                {
                    break;
                }
            default:
                {
                    throw fflerror("invalid leaf node type");
                }
        }
    }
    return rawString;
}
