/*
 * =====================================================================================
 *
 *       Filename: xmllayout.cpp
 *        Created: 03/25/2020 06:17:07
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

#include "log.hpp"
#include "strf.hpp"
#include "fontexdb.hpp"
#include "xmltypeset.hpp"
#include "mathf.hpp"
#include "colorf.hpp"
#include "xmllayout.hpp"

extern Log *g_Log;
extern FontexDB *g_FontexDB;

void XMLLayout::addPar(int loc, const std::array<int, 4> &margin, const tinyxml2::XMLNode *node)
{
    if(loc < 0 || loc > parCount()){
        throw fflerror("invalid location: %zu", loc);
    }

    if(!node){
        throw fflerror("null xml node");
    }

    const auto elemNode = node->ToElement();
    if(!elemNode){
        throw fflerror("can't cast to XMLElement");
    }

    bool parXML = false;
    for(const char *cstr: {"par", "Par", "PAR"}){
        if(std::string(elemNode->Name()) == cstr){
            parXML = true;
            break;
        }
    }

    if(!parXML){
        throw fflerror("not a paragraph node");
    }

    const int lineWidth = [elemNode, this]()
    {
        const int defMargin = m_margin[2] + m_margin[3];
        if(m_w <= defMargin){
            throw fflerror("invalid default line width");
        }

        // even we use default size
        // we won't let it create one-line mode by default
        // user should explicitly note that they want this special mode

        int lineWidth = std::max<int>(m_w - defMargin, 1);
        elemNode->QueryIntAttribute("lineWidth", &lineWidth);
        return lineWidth;
    }();

    const int lineAlign = [elemNode, this]()
    {
        if(const auto val = elemNode->Attribute("align")){
            if(std::string(val) == "left"       ) return LALIGN_LEFT;
            if(std::string(val) == "right"      ) return LALIGN_RIGHT;
            if(std::string(val) == "center"     ) return LALIGN_CENTER;
            if(std::string(val) == "justify"    ) return LALIGN_JUSTIFY;
            if(std::string(val) == "distributed") return LALIGN_DISTRIBUTED;
        }
        return m_align;
    }();

    const bool canThrough = [elemNode, this]()
    {
        bool canThrough = m_canThrough;
        elemNode->QueryBoolAttribute("canThrough", &canThrough);
        return canThrough;
    }();

    const uint8_t font = [elemNode, this]()
    {
        int font = m_font;
        if(const auto val = elemNode->Attribute("font")){
            try{
                font = std::stoi(val);
            }
            catch(...){
                font = g_FontexDB->findFontName(val);
            }
        }

        if(g_FontexDB->hasFont(font)){
            return font;
        }
        return 0;
    }();

    const uint8_t fontSize = [elemNode, this]()
    {
        int fontSize = m_fontSize;
        elemNode->QueryIntAttribute("size", &fontSize);
        return fontSize;
    }();

    const uint32_t fontColor = [elemNode, this]()
    {
        if(const char *val = elemNode->Attribute("color")){
            return colorf::String2RGBA(val);
        }
        return m_fontColor;
    }();

    const uint8_t fontStyle = m_fontStyle;

    const int lineSpace = [elemNode, this]()
    {
        int lineSpace = m_lineSpace;
        elemNode->QueryIntAttribute("lineSpace", &lineSpace);
        return lineSpace;
    }();

    const int wordSpace = [elemNode, this]()
    {
        int wordSpace = m_lineSpace;
        elemNode->QueryIntAttribute("wordSpace", &wordSpace);
        return wordSpace;
    }();

    auto pNew = std::make_unique<XMLTypeset>
    (
        lineWidth,
        lineAlign,
        canThrough,
        font,
        fontSize,
        fontStyle,
        fontColor,
        lineSpace,
        wordSpace
    );

    pNew->loadXMLNode(node);
    auto pInsert = m_parNodeList.insert(ithIterator(loc), {-1, margin, std::move(pNew)});

    if(pInsert == m_parNodeList.begin()){
        pInsert->startY = margin[1];
    }
    else{
        const auto prevNode = std::prev(pInsert);
        pInsert->startY = prevNode->startY + prevNode->margin[0] + prevNode->margin[1] + prevNode->tpset->ph();
    }

    const int extraH = pInsert->tpset->ph() + pInsert->margin[0] + pInsert->margin[1];
    for(auto p = std::next(pInsert); p != m_parNodeList.end(); ++p){
        p->startY += extraH;
    }
}

void XMLLayout::drawEx(int dstX, int dstY, int srcX, int srcY, int w, int h)
{
    for(const auto &node: m_parNodeList){
        int srcXCrop = srcX;
        int srcYCrop = srcY;
        int dstXCrop = dstX;
        int dstYCrop = dstY;
        int srcWCrop = w;
        int srcHCrop = h;

        if(!mathf::ROICrop(
                    &srcXCrop, &srcYCrop,
                    &srcWCrop, &srcHCrop,
                    &dstXCrop, &dstYCrop,

                    W(),
                    H(),

                    0, node.startY, node.tpset->pw(), node.tpset->ph(), 0, 0, -1, -1)){
            continue;
        }

        node.tpset->drawEx(dstXCrop, dstYCrop, srcXCrop - node.margin[2], srcYCrop - node.startY, srcWCrop, srcHCrop);
    }
}

int XMLLayout::pw()
{
    int maxW = 0;
    for(const auto &node: m_parNodeList){
        maxW = std::max<int>(maxW, node.tpset->pw());
    }
    return maxW;
}

void XMLLayout::loadXML(const char *xmlString)
{
    m_parNodeList.clear();
    tinyxml2::XMLDocument xmlDoc;

    if(xmlDoc.Parse(xmlString) != tinyxml2::XML_SUCCESS){
        throw fflerror("parse xml failed: %s", xmlString ? xmlString : "(null)");
    }

    bool layoutXML = false;
    auto pRoot = xmlDoc.RootElement();

    for(const char *cstr: {"layout", "Layout", "LAYOUT"}){
        if(std::string(pRoot->Name()) == cstr){
            layoutXML = true;
            break;
        }
    }

    if(!layoutXML){
        throw fflerror("string is not layout xml");
    }

    if(pRoot->FirstAttribute()){
        g_Log->addLog(LOGTYPE_WARNING, "Layout XML won't accept attributes, ignored.");
    }

    for(auto p = pRoot->FirstChild(); p; p = p->NextSibling()){
        if(!p->ToElement()){
            g_Log->addLog(LOGTYPE_WARNING, "Not an element: %s", p->Value());
            continue;
        }

        bool parXML = false;
        for(const char *cstr: {"par", "Par", "PAR"}){
            if(std::string(p->Value()) == cstr){
                parXML = true;
                break;
            }
        }

        if(!parXML){
            g_Log->addLog(LOGTYPE_WARNING, "Not a paragraph element: %s", p->Value());
            continue;
        }

        addPar(parCount(), m_margin, p);
    }
}

void XMLLayout::update(double ms)
{
    for(const auto &node: m_parNodeList){
        node.tpset->update(ms);
    }
}
