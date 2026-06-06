#include <tuple>
#include <utf8.h>
#include <stdexcept>
#include <tinyxml2.h>

#include "log.hpp"
#include "bevent.hpp"
#include "totype.hpp"
#include "strf.hpp"
#include "xmlf.hpp"
#include "utf8f.hpp"
#include "xmlparagraph.hpp"

extern Log *g_log;
XMLParagraph *XMLParagraph::split(int leafIndex, int cursorLoc)
{
    fflassert(leafValid(leafIndex));
    fflassert(cursorLoc >= 0 && cursorLoc <= leaf(leafIndex).length());

    XMLParagraph * newPar = new XMLParagraph{};
    XMLParagraph *fromPar = this;
    XMLParagraph *  toPar = newPar;

    if(leafIndex * 2 > leafCount()){
        std::swap(fromPar->m_xmlDocument, toPar->m_xmlDocument);
        std::swap(fromPar->m_leafList   , toPar->m_leafList   );
        std::swap(fromPar               , toPar               );
    }

    const auto fnMoveFrontLeaf = [fromPar, toPar]()
    {
        auto node = fromPar->m_leafList.front().xmlNode()->DeepClone(toPar->m_xmlDocument.get());
        toPar->m_xmlDocument->FirstChild()->InsertEndChild(node);
        toPar->m_leafList.emplace_back(node);

        fromPar->m_xmlDocument->FirstChild()->DeleteChild(fromPar->m_leafList.front().xmlNode());
        fromPar->m_leafList.pop_front();
    };

    for(int i = 0; i < leafIndex - 1; ++i){
        fnMoveFrontLeaf();
    }

    if(cursorLoc == 0){
        // do nothing
    }
    else if(cursorLoc == fromPar->m_leafList.front().length()){
        fnMoveFrontLeaf();
    }
    else{
        auto [node1, node2] = fromPar->m_leafList.front().split(cursorLoc, *toPar->m_xmlDocument, *fromPar->m_xmlDocument);
        toPar->m_xmlDocument->FirstChild()->InsertEndChild(node1);
        toPar->m_leafList.emplace_back(node1);
    }

    return newPar;
}

void XMLParagraph::deleteLeaf(int leafIndex)
{
    auto node   = leaf(leafIndex).xmlNode();
    auto parent = node->Parent();

    while(parent && (parent->FirstChild() == parent->LastChild())){
        node   = parent;
        parent = parent->Parent();
    }

    if(parent){
        parent->DeleteChild(node);
    }
    else{
        m_xmlDocument->Clear();
    }
    m_leafList.erase(m_leafList.begin() + leafIndex);
}

size_t XMLParagraph::insertUTF8String(int leafIndex, int leafOff, const char *utf8String)
{
    if(leaf(leafIndex).type() != LEAF_UTF8STR){
        throw fflpanic("the {}-th leaf is not a XMLText", leafIndex);
    }

    if(leafOff < 0 || leafOff > to_d(leaf(leafIndex).utf8CharOff().size())){
        throw fflpanic("leaf offset {} is not a valid insert offset", leafOff);
    }

    if(utf8String == nullptr){
        throw fflpanic("null utf8 text string");
    }

    if(std::strlen(utf8String) == 0){
        return 0;
    }

    if(!utf8::is_valid(utf8String, utf8String + std::strlen(utf8String))){
        throw fflpanic("invalid utf-8 string: {}", utf8String);
    }

    auto &utf8OffRef = leaf(leafIndex).utf8CharOff();
    const auto [oldLength, newValue] = [leafIndex, leafOff, utf8String, &utf8OffRef, this]() -> std::tuple<int, std::string>
    {
        // after we call XMLNode::SetValue(), the oldValue pointer get updated
        // use lambda here on purpose to prevent oldValue pointer get exposed to outiler scope

        const auto oldValue  = leaf(leafIndex).xmlNode()->Value();
        const auto oldLength = std::strlen(oldValue);

        if(leafOff >= to_d(utf8OffRef.size())){
            return {oldLength, std::string(oldValue) + utf8String};
        }
        return {oldLength, (std::string(oldValue, oldValue + utf8OffRef[leafOff]) + utf8String) + std::string(oldValue + utf8OffRef[leafOff])};
    }();
    m_leafList[leafIndex].xmlNode()->SetValue(newValue.c_str());

    auto addedValueOff = utf8f::buildUTF8Off(utf8String);
    if(leafOff > 0){
        const auto firstOffDiff = [leafOff, oldLength, &utf8OffRef]() -> int
        {
            if(leafOff < to_d(utf8OffRef.size())){
                return utf8OffRef[leafOff];
            }
            return oldLength;
        }();

        for(auto &elemRef: addedValueOff){
            elemRef += firstOffDiff;
        }
    }

    if(leafOff < to_d(utf8OffRef.size())){
        const int addedLength = std::strlen(utf8String);
        for(int i = leafOff; i < to_d(utf8OffRef.size()); ++i){
            utf8OffRef[i] += addedLength;
        }
    }

    utf8OffRef.insert(utf8OffRef.begin() + leafOff, addedValueOff.begin(), addedValueOff.end());
    return addedValueOff.size();
}

void XMLParagraph::deleteUTF8Char(int leafIndex, int leafOff, int tokenCount)
{
    if(leaf(leafIndex).type() != LEAF_UTF8STR){
        throw fflpanic("the {}-th leaf is not a XMLText", leafIndex);
    }

    if(!leafOffValid(leafIndex, leafOff)){
        throw fflpanic("invalid leafOff: {}", leafOff);
    }

    if(tokenCount == 0){
        return;
    }

    if((leafOff + tokenCount - 1) >= leaf(leafIndex).length()){
        throw fflpanic("the {}-th leaf has only {} tokens", leafIndex, leaf(leafIndex).length());
    }

    if(tokenCount == leaf(leafIndex).length()){
        deleteLeaf(leafIndex);
        return;
    }

    const int charOff = leaf(leafIndex).utf8CharOff()[leafOff];
    const int charLen = [this, leafIndex, charOff, leafOff, tokenCount]() -> int
    {
        if(leafOff + tokenCount < to_d(leaf(leafIndex).utf8CharOff().size())){
            return leaf(leafIndex).utf8CharOff()[leafOff + tokenCount] - charOff;
        }
        return to_d(std::strlen(leaf(leafIndex).xmlNode()->Value())) - charOff;
    }();

    const auto newValue = std::string(leaf(leafIndex).xmlNode()->Value()).erase(charOff, charLen);
    leaf(leafIndex).xmlNode()->SetValue(newValue.c_str());

    for(int i = leafOff; i + tokenCount < to_d(leaf(leafIndex).utf8CharOff().size()); ++i){
        leaf(leafIndex).utf8CharOff()[i] = leaf(leafIndex).utf8CharOff()[i + tokenCount] - charLen;
    }
    leaf(leafIndex).utf8CharOff().resize(leaf(leafIndex).utf8CharOff().size() - tokenCount);
}

void XMLParagraph::deleteToken(int leafIndex, int leafOff, int tokenCount)
{
    if(!leafOffValid(leafIndex, leafOff)){
        throw fflpanic("invalid leaf off: ({}, {})", leafIndex, leafOff);
    }

    if(tokenCount == 0){
        return;
    }

    const bool hasEnoughToken = [leafIndex, leafOff, tokenCount, this]()
    {
        int tokenLeft = leaf(leafIndex).length() - leafOff;
        if(tokenLeft >= tokenCount){
            return true;
        }

        for(int i = leafIndex + 1; i < leafCount(); ++i){
            tokenLeft += leaf(i).length();
            if(tokenLeft >= tokenCount){
                return true;
            }
        }
        return false;
    }();

    if(!hasEnoughToken){
        throw fflpanic("insufficient token to remove");
    }

    int currLeaf     = leafIndex;
    int currLeafOff  = leafOff;
    int deletedToken = 0;

    while(deletedToken < tokenCount){
        switch(leaf(currLeaf).type()){
            case LEAF_UTF8STR:
                {
                    const auto leafLength = leaf(currLeaf).length();
                    const auto needDelete = std::min<int>(leafLength - currLeafOff, tokenCount - deletedToken);

                    deleteUTF8Char(currLeaf, currLeafOff, needDelete);
                    deletedToken += needDelete;

                    if(needDelete != leafLength){
                        currLeaf++; // current leaf is not fully deleted
                    }

                    currLeafOff = 0;
                    break;
                }
            case LEAF_EMOJI:
            case LEAF_IMAGE:
                {
                    deleteLeaf(currLeaf);
                    deletedToken += 1;
                    currLeafOff = 0;
                    break;
                }
            default:
                {
                    throw fflpanic("invalid leaf type: {}", leaf(currLeaf).type());
                }
        }
    }
}

std::tuple<int, int, int> XMLParagraph::prevLeafOff(int leafIndex, int leafOff, int) const
{
    if(leafOff >= to_d(leaf(leafIndex).utf8CharOff().size())){
        throw fflpanic("the {}-th leaf has only {} tokens", leafIndex, leaf(leafIndex).utf8CharOff().size());
    }

    return {0, 0, 0};
}

std::tuple<int, int, int> XMLParagraph::nextLeafOff(int leafIndex, int leafOff, int tokenCount) const
{
    if(leafOff >= leaf(leafIndex).length()){
        throw fflpanic("the {}-th leaf has only {} tokens", leafIndex, leaf(leafIndex).length());
    }

    int nCurrLeaf      = leafIndex;
    int nCurrLeafOff   = leafOff;
    int nAdvancedToken = 0;

    while((nAdvancedToken < tokenCount) && (nCurrLeaf < leafCount())){
        switch(auto nType = leaf(nCurrLeaf).type()){
            case LEAF_EMOJI:
            case LEAF_IMAGE:
            case LEAF_UTF8STR:
                {
                    int nCurrTokenLeft = leaf(nCurrLeaf).length() - nCurrLeafOff - 1;
                    if(nCurrTokenLeft >= tokenCount - nAdvancedToken){
                        return {nCurrLeaf, nCurrLeafOff + (tokenCount - nAdvancedToken), tokenCount};
                    }

                    // current leaf is not enough
                    // but this is the last leaf, have to stop
                    if(nCurrLeaf == (leafCount() - 1)){
                        return {nCurrLeaf, leaf(nCurrLeaf).length() - 1, nAdvancedToken + nCurrTokenLeft};
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
                    throw fflpanic("invalid leaf type: {}", nType);
                }
        }
    }
    return {nCurrLeaf, nCurrLeafOff, nAdvancedToken};
}

void XMLParagraph::Join(const XMLParagraph &rstInput)
{
    if(rstInput.m_xmlDocument->FirstChild() == nullptr){
        return;
    }

    for(auto pNode = rstInput.m_xmlDocument->FirstChild()->FirstChild(); pNode; pNode = pNode->NextSibling()){
        m_xmlDocument->FirstChild()->InsertEndChild(pNode->DeepClone(m_xmlDocument->GetDocument()));
    }
}

void XMLParagraph::loadXML(const char *xmlString)
{
    if(xmlString == nullptr){
        throw fflpanic("null xml string");
    }

    if(!utf8::is_valid(xmlString, xmlString + std::strlen(xmlString))){
        throw fflpanic("xml is not a valid utf8 string: {}", xmlString);
    }

    tinyxml2::XMLDocument xmlDoc(true, tinyxml2::PEDANTIC_WHITESPACE);
    if(xmlDoc.Parse(xmlString) != tinyxml2::XML_SUCCESS){
        throw fflpanic("tinyxml2::XMLDocument::Parse() failed: {}", xmlString);
    }

    loadXMLNode(xmlDoc.RootElement());
}

void XMLParagraph::loadXMLNode(const tinyxml2::XMLNode *node)
{
    if(!node){
        throw fflpanic("null node pointer");
    }

    if(!node->ToElement()){
        throw fflpanic("given node is not an element");
    }

    bool parXML = false;
    for(const char *cstr: {"par", "Par", "PAR"}){
        if(to_sv(node->Value()) == cstr){
            parXML = true;
            break;
        }
    }

    if(!parXML){
        throw fflpanic("not a paragraph node");
    }

    m_xmlDocument->Clear();
    if(auto pNew = node->DeepClone(m_xmlDocument.get()); pNew){
        m_xmlDocument->InsertEndChild(pNew);
    }
    else{
        throw fflpanic("copy paragraph node failed");
    }

    m_leafList.clear();
    if(auto parNode = m_xmlDocument->FirstChild(); !parNode->NoChildren()){
        for(auto nodePtr = xmlf::getNodeFirstLeaf(parNode); nodePtr; nodePtr = xmlf::getNextLeaf(nodePtr, parNode)){
            if(xmlf::checkValidLeaf(nodePtr)){
                m_leafList.emplace_back(nodePtr);
            }
        }
    }
}

size_t XMLParagraph::insertLeafXML(int loc, const char *xmlString)
{
    if(loc < 0 || loc > leafCount() || !xmlString){
        throw fflpanic("invalid argument: loc = {}, xmlString = {:p}", loc, to_cvptr(xmlString));
    }

    if(loc == 0){
        return insertXMLAtFront(xmlString);
    }
    return insertXMLAfter(leaf(loc - 1).xmlNode(), xmlString);
}

size_t XMLParagraph::insertXMLAtFront(const char *xmlString)
{
    if(!xmlString){
        throw fflpanic("invalid argument: xmlString = {:p}", to_cvptr(xmlString));
    }

    // to insert XML as leaves, we requires to insert as a new paragraph leaf
    // for emoji and image we have specified tag <emoji>, <image>, but for text we don't have

    tinyxml2::XMLDocument xmlDoc(true, tinyxml2::PEDANTIC_WHITESPACE);
    if(xmlDoc.Parse(xmlString) != tinyxml2::XML_SUCCESS){
        throw fflpanic("tinyxml2::XMLDocument::Parse() failed: {}", xmlString);
    }

    auto xmlRoot = xmlDoc.RootElement();
    if(!xmlRoot){
        throw fflpanic("no root element");
    }

    bool parXML = false;
    for(const char *cstr: {"par", "Par", "PAR"}){
        if(to_sv(xmlRoot->Value()) == cstr){
            parXML = true;
            break;
        }
    }

    if(!parXML){
        throw fflpanic("xmlString is not a paragraph");
    }

    size_t addedLeafCount = 0;
    for(auto p = xmlRoot->LastChild(); p; p = p->PreviousSibling()){
        if(auto cloneNode = p->DeepClone(m_xmlDocument.get())){
            if(m_xmlDocument->FirstChild()->InsertFirstChild(cloneNode) != cloneNode){
                throw fflpanic("insert node failed");
            }
            addedLeafCount++;
        }
        else{
            throw fflpanic("deep clone node failed");
        }
    }

    // TODO rebuild everything
    // this is not necessary, optimize later

    m_leafList.clear();
    if(auto parNode = m_xmlDocument->FirstChild(); !parNode->NoChildren()){
        for(auto nodePtr = xmlf::getNodeFirstLeaf(parNode); nodePtr; nodePtr = xmlf::getNextLeaf(nodePtr, parNode)){
            if(xmlf::checkValidLeaf(nodePtr)){
                m_leafList.emplace_back(nodePtr);
            }
        }
    }

    size_t addedCount = 0;
    for(size_t i = 0; i < addedLeafCount; ++i){
        addedCount += leaf(i).length();
    }

    return addedCount;
}

size_t XMLParagraph::insertXMLAfter(tinyxml2::XMLNode *after, const char *xmlString)
{
    if(!(after && xmlString)){
        throw fflpanic("invalid argument: after = {:p}, xmlString = {:p}", to_cvptr(after), to_cvptr(xmlString));
    }

    if(after->GetDocument() != m_xmlDocument.get()){
        throw fflpanic("can't find after node in current XMLDocument");
    }

    // to insert XML as leaves, we requires to insert as a new paragraph leaf
    // for emoji and image we have specified tag <emoji>, <image>, but for text we don't have

    tinyxml2::XMLDocument xmlDoc(true, tinyxml2::PEDANTIC_WHITESPACE);
    if(xmlDoc.Parse(xmlString) != tinyxml2::XML_SUCCESS){
        throw fflpanic("tinyxml2::XMLDocument::Parse() failed: {}", xmlString);
    }

    auto xmlRoot = xmlDoc.RootElement();
    if(!xmlRoot){
        throw fflpanic("no root element");
    }

    bool parXML = false;
    for(const char *cstr: {"par", "Par", "PAR"}){
        if(to_sv(xmlRoot->Value()) == cstr){
            parXML = true;
            break;
        }
    }

    if(!parXML){
        throw fflpanic("xmlString is not a paragraph");
    }

    size_t addedLeafCount = 0;
    for(auto p = xmlRoot->LastChild(); p; p = p->PreviousSibling()){
        if(auto cloneNode = p->DeepClone(m_xmlDocument.get())){
            if(after->Parent()->InsertAfterChild(after, cloneNode) != cloneNode){
                throw fflpanic("insert node failed");
            }
            addedLeafCount++;
        }
        else{
            throw fflpanic("deep clone node failed");
        }
    }

    // TODO rebuild everything
    // this is not necessary, optimize later

    m_leafList.clear();
    if(auto parNode = m_xmlDocument->FirstChild(); !parNode->NoChildren()){
        for(auto nodePtr = xmlf::getNodeFirstLeaf(parNode); nodePtr; nodePtr = xmlf::getNextLeaf(nodePtr, parNode)){
            if(xmlf::checkValidLeaf(nodePtr)){
                m_leafList.emplace_back(nodePtr);
            }
        }
    }

    int afterLoc = -1;
    size_t addedCount = 0;

    for(int i = 0; i < leafCount(); ++i){
        if(leaf(i).xmlNode() == after){
            afterLoc = i;
            break;
        }
    }

    for(int i = afterLoc + 1; i < leafCount() && addedLeafCount > 0; ++i, --addedLeafCount){
        addedCount += leaf(i).length();
    }

    return addedCount;
}

void XMLParagraph::deleteToken(int leafIndex, int leafOff)
{
    switch(leaf(leafIndex).type()){
        case LEAF_UTF8STR:
            {
                deleteUTF8Char(leafIndex, leafOff, 1);
                return;
            }
        case LEAF_EMOJI:
        case LEAF_IMAGE:
            {
                deleteLeaf(leafIndex);
                return;
            }
        default:
            {
                throw fflpanic("invalid leaf type: {}", leaf(leafIndex).type());
            }
    }
}

std::string XMLParagraph::getRawString() const
{
    std::string rawString;
    for(const auto &leaf: m_leafList){
        switch(leaf.type()){
            case LEAF_UTF8STR:
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
                    throw fflpanic("invalid leaf node type");
                }
        }
    }
    return rawString;
}

size_t XMLParagraph::tokenCount() const
{
    size_t count = 0;
    for(int i = 0; i < leafCount(); ++i){
        count += leaf(i).length();
    }
    return count;
}
