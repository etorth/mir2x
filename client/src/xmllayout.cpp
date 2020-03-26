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
#include "strfunc.hpp"
#include "xmltypeset.hpp"
#include "mathfunc.hpp"
#include "colorfunc.hpp"
#include "xmllayout.hpp"

extern Log *g_Log;

void XMLLayout::addPar(int loc, const std::array<int, 4> &margin, size_t maxLineWidth, int lalign, bool canThrough, size_t wordSpace, size_t lineSpace, const tinyxml2::XMLNode *node)
{
    if(loc < 0 || loc > parCount()){
        throw fflerror("invalid location: %d", loc);
    }

    if(!node){
        throw fflerror("null xml node");
    }

    auto pNew = std::make_unique<XMLTypeset>
    (
        maxLineWidth,
        lalign,
        canThrough,
        lineSpace,
        wordSpace,
        m_DefaultFont,
        m_DefaultFontSize,
        m_DefaultFontStyle,
        m_DefaultFontColor
    );

    pNew->loadXMLNode(node);
    auto pInsert = m_parNodeList.insert(ithIterator(loc), {-1, margin, std::move(pNew)});

    if(pInsert == m_parNodeList.begin()){
        pInsert->startY = margin[1];
    }
    else{
        const auto prevNode = std::prev(pInsert);
        pInsert->startY = prevNode->startY + prevNode->margin[0] + prevNode->margin[1] + prevNode->tpset->PH();
    }

    const int extraH = pInsert->tpset->PH() + pInsert->margin[0] + pInsert->margin[1];
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

        if(!MathFunc::ROICrop(
                    &srcXCrop, &srcYCrop,
                    &srcWCrop, &srcHCrop,
                    &dstXCrop, &dstYCrop,

                    W(),
                    H(),

                    0, node.startY, node.tpset->PW(), node.tpset->PH(), 0, 0, -1, -1)){
            continue;
        }

        node.tpset->drawEx(dstXCrop, dstYCrop, srcXCrop - node.margin[2], srcYCrop - node.startY, srcWCrop, srcHCrop);
    }
}

int XMLLayout::PW()
{
    int maxW = 0;
    for(const auto &node: m_parNodeList){
        maxW = std::max<int>(maxW, node.tpset->PW());
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

    for(auto p = pRoot->FirstChild(); p; p = p->NextSibling()){
        if(!p->ToElement()){
            g_Log->AddLog(LOGTYPE_WARNING, "Not an element: %s", p->Value());
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
            g_Log->AddLog(LOGTYPE_WARNING, "Not a paragraph element: %s", p->Value());
            continue;
        }

        addPar(parCount(), {0, 0, 0, 0}, m_W, LALIGN_LEFT, false, 0, 0, p);
    }
}

void XMLLayout::update(double ms)
{
    for(const auto &node: m_parNodeList){
        node.tpset->update(ms);
    }
}
