/*
 * =====================================================================================
 *
 *       Filename: tokenboard.cpp
 *        Created: 6/17/2015 10:24:27 PM
 *  Last Modified: 03/14/2016 00:11:13
 *
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

#include "emoticon.hpp"
#include "misc.hpp"
#include "section.hpp"
#include "tokenboard.hpp"
#include "tokenbox.hpp"
#include "utf8char.hpp"

#include <SDL2/SDL.h>
#include <algorithm>
#include <functional>
#include <tinyxml2.h>
#include <utf8.h>

TokenBoard::TokenBoard(bool bWrap, int nMaxWidth, int nMinTextMargin)
    : m_PW(nMaxWidth)
    , m_MinTextMargin(nMinTextMargin)
    , m_W(0)
    , m_H(0)
    , m_Resolution(20)
    , m_Wrap(bWrap)
    , m_CurrentWidth(0)
    , m_HasEventText(false)
    , m_Valid(false)
{
}

bool TokenBoard::Load(const tinyxml2::XMLElement *pDoc,
        std::function<void(bool, uint64_t, int &, int &)> fnTokenBoxSize,
        std::function<void(uint32_t, int &, int &, int &)> fnEmoticonInfo,
        std::unordered_map<std::string, std::function<void()> stIDhandlerMap)
{
    m_Valid = false;
}

bool TokenBoard::Load(const tinyxml2::XMLDocument *pDoc)
{
    m_Valid = false;

    if(pDoc == nullptr){ return false; }
    
    const tinyxml2::XMLElement *pRoot = pDoc->RootElement();
    if (pRoot == nullptr){ return false; }

    const tinyxml2::XMLElement *pCurrentObject = nullptr;
    if(false){
    }else if(pCurrentObject = root->FirstChildElement("object")){
    }else if(pCurrentObject = root->FirstChildElement("Object")){
    }else if(pCurrentObject = root->FirstChildElement("OBJECT")){
    }else{
        return false;
    }

    return ParseXMLContent(pCurrentObject);
}

int TokenBoard::W()
{
    return m_W;
}

int TokenBoard::H()
{
    return m_H;
}

bool TokenBoard::ParseXMLContent(const tinyxml2::XMLElement *pFirstObject)
{
    int  nSection       = 0;
    auto pCurrentObject = pFirstObject;

    while(pCurrentObject){
        if(ObjectReturn(pCurrentObject)){
            if(!m_CurrentLine.empty()){
                AddNewTokenBoxLine(true);
            }
        }else if(ObjectEmocticon(pCurrentObject)){
            ParseEmoticonObject(pCurrentObject, nSection++);
        }else if(ObjectText(pCurrentObject)){
            ParseTextObject(pCurrentObject, nSection++);
        }else if(ObjectEventText(pCurrentObject)){
            ParseEventTextObject(pCurrentObject, nSection++);
        }
        pCurrentObject = NextObject(pCurrentObject);
    }

    if(!m_CurrentLine.empty()){
        AddNewTokenBoxLine(true);
    }

    if(m_HasEventText){
        MakeEventTextBitmap();
    }

    return true;
}

bool TokenBoard::ObjectReturn(const tinyxml2::XMLElement *pCurrentObject)
{
    return false
        || (pCurrentObject->Attribute("TYPE", "RETURN"))
        || (pCurrentObject->Attribute("TYPE", "Return"))
        || (pCurrentObject->Attribute("TYPE", "return"))
        || (pCurrentObject->Attribute("Type", "RETURN"))
        || (pCurrentObject->Attribute("Type", "Return"))
        || (pCurrentObject->Attribute("Type", "return"))
        || (pCurrentObject->Attribute("type", "RETURN"))
        || (pCurrentObject->Attribute("type", "Return"))
        || (pCurrentObject->Attribute("type", "return"))
        ;
}

bool TokenBoard::ObjectEmocticon(const tinyxml2::XMLElement *pCurrentObject)
{
    return false
        || (pCurrentObject->Attribute("TYPE", "Emoticon"))
        || (pCurrentObject->Attribute("TYPE", "emoticon"))
        || (pCurrentObject->Attribute("TYPE", "EMOTICON"))
        || (pCurrentObject->Attribute("Type", "Emoticon"))
        || (pCurrentObject->Attribute("Type", "emoticon"))
        || (pCurrentObject->Attribute("Type", "EMOTICON"))
        || (pCurrentObject->Attribute("type", "Emoticon"))
        || (pCurrentObject->Attribute("type", "emoticon"))
        || (pCurrentObject->Attribute("type", "EMOTICON"))
        ;
}

bool TokenBoard::ObjectText(const tinyxml2::XMLElement *pCurrentObject)
{
    if(false
            || ObjectReturn(pCurrentObject)
            || ObjectEmocticon(pCurrentObject)
            || ObjectEventText(pCurrentObject)
      ){
        return false;
    }

    return false
        || (pCurrentObject->Attribute("Type") == nullptr)
        || (pCurrentObject->Attribute("type") == nullptr)
        || (pCurrentObject->Attribute("TYPE") == nullptr)
        || (pCurrentObject->Attribute("TYPE", "Text"))
        || (pCurrentObject->Attribute("TYPE", "text"))
        || (pCurrentObject->Attribute("TYPE", "TEXT"))
        || (pCurrentObject->Attribute("Type", "Text"))
        || (pCurrentObject->Attribute("Type", "text"))
        || (pCurrentObject->Attribute("Type", "TEXT"))
        || (pCurrentObject->Attribute("type", "Text"))
        || (pCurrentObject->Attribute("type", "text"))
        || (pCurrentObject->Attribute("type", "TEXT"))
        ;
}


bool TokenBoard::ObjectEventText(const tinyxml2::XMLElement *pCurrentObject)
{
    return false
        || (pCurrentObject->Attribute("TYPE", "EVENTTEXT"))
        || (pCurrentObject->Attribute("TYPE", "EventText"))
        || (pCurrentObject->Attribute("TYPE", "Eventtext"))
        || (pCurrentObject->Attribute("TYPE", "eventText"))
        || (pCurrentObject->Attribute("TYPE", "eventtext"))
        || (pCurrentObject->Attribute("Type", "EVENTTEXT"))
        || (pCurrentObject->Attribute("Type", "EventText"))
        || (pCurrentObject->Attribute("Type", "Eventtext"))
        || (pCurrentObject->Attribute("Type", "eventText"))
        || (pCurrentObject->Attribute("Type", "eventtext"))
        || (pCurrentObject->Attribute("type", "EVENTTEXT"))
        || (pCurrentObject->Attribute("type", "EventText"))
        || (pCurrentObject->Attribute("type", "Eventtext"))
        || (pCurrentObject->Attribute("type", "eventText"))
        || (pCurrentObject->Attribute("type", "eventtext"))
        ;
}

int TokenBoard::GetEmoticonSet(const tinyxml2::XMLElement *pCurrentObject)
{
    const char *pText = nullptr;

    if(false){
    }else if(pText = pCurrentObject->Attribute("SET")){
    }else if(pText = pCurrentObject->Attribute("Set")){
    }else if(pText = pCurrentObject->Attribute("set")){
    }else{
        pText = "0";
    }
    // here means when set can not be found or conversion fails
    // return 0
    return std::atoi(pText);
}

int TokenBoard::GetEmoticonIndex(const tinyxml2::XMLElement *pCurrentObject)
{
    const char *pText = nullptr;

    if(false){
    }else if(pText = pCurrentObject->Attribute("INDEX")){
    }else if(pText = pCurrentObject->Attribute("Index")){
    }else if(pText = pCurrentObject->Attribute("index")){
    }else{
        pText = "0";
    }
    return std::atoi(pText);
}

const tinyxml2::XMLElement *TokenBoard::NextObject(const tinyxml2::XMLElement *pCurrentObject)
{
    const tinyxml2::XMLElement *pRet = nullptr;
    pRet = pCurrentObject->NextSiblingElement("OBJECT");
    if(pRet){ return pRet; }
    pRet = pCurrentObject->NextSiblingElement("Object");
    if(pRet){ return pRet; }
    pRet = pCurrentObject->NextSiblingElement("object");
    if(pRet){ return pRet; }
    return nullptr;
}


int TokenBoard::GetFontSize(const tinyxml2::XMLElement *pCurrentObject)
{
    const char *pText           = nullptr;
    const int   defaultFontSize = 18;

    if(false){
    }else if(pText = pCurrentObject->Attribute("SIZE")){
    }else if(pText = pCurrentObject->Attribute("Size")){
    }else if(pText = pCurrentObject->Attribute("size")){
    }else{
        return defaultFontSize;
    }
    int fontSize = std::atoi(pText);
    return fontSize == 0 ? defaultFontSize : fontSize;
}

int TokenBoard::GetFontIndex(const tinyxml2::XMLElement *pCurrentObject)
{
    const char *pText;

    if(false){
    }else if(pText = pCurrentObject->Attribute("FONT")){
    }else if(pText = pCurrentObject->Attribute("Font")){
    }else if(pText = pCurrentObject->Attribute("font")){
    }else{
        // default to set FontIndex to be zero
        pText = "0";
    }
    return std::atoi(pText);
}

int TokenBoard::GetFontStyle(const tinyxml2::XMLElement *pCurrentObject)
{
    const char *pText;

    if(false){
    }else if(pText = pCurrentObject->Attribute("STYLE")){
    }else if(pText = pCurrentObject->Attribute("Style")){
    }else if(pText = pCurrentObject->Attribute("style")){
    }else{
        pText = "0";
    }

    return std::atoi(pText);
}

SDL_Color TokenBoard::GetEventTextCharColor(const tinyxml2::XMLElement *pCurrentObject, int nEvent)
{
    if(nEvent < 0 || nEvent > 2){
        return {0X00, 0X00, 0X00, 0X00};
    }

    char *attrText[3][3] = {
        {"OFF",  "Off",  "off" },
        {"ON",   "On",   "on"  },
        {"PUSH", "Push", "push"}
    };

    SDL_Color defaultColor[3] = {
        { 0XFF, 0XFF, 0X00, 0XFF },
        { 0X00, 0XFF, 0X00, 0XFF },
        { 0XFF, 0X00, 0X00, 0XFF }
    };
    const char *pText;
    SDL_Color   color;

    if(false){
    }else if(pText = pCurrentObject->Attribute(attrText[nEvent][0])){
    }else if(pText = pCurrentObject->Attribute(attrText[nEvent][1])){
    }else if(pText = pCurrentObject->Attribute(attrText[nEvent][2])){
    }else{
        return defaultColor[nEvent];
    }

    color.r = 0x00;
    color.g = 0x00;
    color.b = 0x00;
    color.a = 0xff;

    if(false
            || !std::strcmp(pText, "RED")
            || !std::strcmp(pText, "Red")
            || !std::strcmp(pText, "red")){
        color.r = 0xff;
    }else if(false
            || !std::strcmp(pText, "GREEN")
            || !std::strcmp(pText, "Green")
            || !std::strcmp(pText, "green")){
        color.g = 0xff;
    }else if(false
            || !std::strcmp(pText, "BLUE")
            || !std::strcmp(pText, "Blue")
            || !std::strcmp(pText, "blue")){
        color.b = 0xff;
    }else if(false
            || !std::strcmp(pText, "YELLOW")
            || !std::strcmp(pText, "Yellow")
            || !std::strcmp(pText, "yellow")){
        color.r = 0xff;
        color.g = 0xff;
    }else{
        // TODO 0xab11df11
    }
    return color;
}

SDL_Color TokenBoard::GetTextCharColor(const tinyxml2::XMLElement *pCurrentObject)
{
    const char *pText;
    SDL_Color   color;

    if(false){
    }else if(pText = pCurrentObject->Attribute("COLOR")){
    }else if(pText = pCurrentObject->Attribute("Color")){
    }else if(pText = pCurrentObject->Attribute("color")){
    }else{
    }
    if(pText == nullptr){
        color.r = 0xff;
        color.g = 0xff;
        color.b = 0xff;
        color.a = 0xff;
        return color;
    }

    color.r = 0x00;
    color.g = 0x00;
    color.b = 0x00;
    color.a = 0xff;
    if(false
            || !std::strcmp(pText, "RED")
            || !std::strcmp(pText, "Red")
            || !std::strcmp(pText, "red")){
        color.r = 0xff;
    }else if(false
            || !std::strcmp(pText, "GREEN")
            || !std::strcmp(pText, "Green")
            || !std::strcmp(pText, "green")){
        color.g = 0xff;
    }else if(false
            || !std::strcmp(pText, "BLUE")
            || !std::strcmp(pText, "Blue")
            || !std::strcmp(pText, "blue")){
        color.b = 0xff;
    }else{
        // TODO 0xab11df11
    }
    return color;
}

FONTINFO TokenBoard::ParseFontInfo(const tinyxml2::XMLElement *pCurrentObject)
{
    // we use the same function for text/eventtext
    // so in the xml the font attribute should be of the same format
    // not a big issue
    FONTINFO fontInfo;
    fontInfo.Index = GetFontIndex(pCurrentObject);
    fontInfo.Style = GetFontStyle(pCurrentObject);
    fontInfo.Size  = GetFontSize(pCurrentObject);
    return fontInfo;
}

void TokenBoard::ParseTextContentSection(const tinyxml2::XMLElement *pCurrentObject, int nSection)
{
    const char *pstart = pCurrentObject->GetText();
    const char *pend   = pstart;

    // when get inside this funciton
    // the section structure has been well-prepared
    //
    int nDefaultSize = (int)m_SectionV[nSection].Info.Text.Size;

    uint32_t nFontAttrKey = 0
        + (((uint64_t)m_SectionV[nSection].Info.Text.FileIndex) << 16)
        + (((uint64_t)m_SectionV[nSection].Info.Text.Size)      <<  8)
        + (((uint64_t)m_SectionV[nSection].Info.Text.Style)     <<  0);


    while(*pend != '\0'){
        pstart = pend;
        utf8::unchecked::advance(pend, 1);

        // should be true
        assert(pend - pstart <= 4);

        uint32_t nUTF8Key = 0;
        std::memcpy(&nUTF8Key, pstart, pend - pstart);

        // fully set the token box unit
        TOKENBOX stTokenBox;
        LoadUTF8CharBoxSizeCache(stTokenBox,
                nSection, nDefaultSize, nFontAttrKey, nUTF8Key, fnQueryTokenBoxInfo);

        AddNewTokenBox(stTokenBox, m_PW);
    }
}

void TokenBoard::ParseEventTextObjectAttribute(const tinyxml2::XMLElement *pCurrentObject, int)
{
    m_CurrentSection.Info.Type          = SECTIONTYPE_EVENTTEXT;
    m_CurrentSection.Info.Text.FontInfo = ParseFontInfo(pCurrentObject);
    m_CurrentSection.Info.Text.Color[0] = GetEventTextCharColor(pCurrentObject, 0);
    m_CurrentSection.Info.Text.Color[1] = GetEventTextCharColor(pCurrentObject, 1);
    m_CurrentSection.Info.Text.Color[2] = GetEventTextCharColor(pCurrentObject, 2);
    m_CurrentSection.State.Text.Event = 0;
    m_SectionV.push_back(m_CurrentSection);
}

void TokenBoard::ParseTextObjectAttribute(const tinyxml2::XMLElement *pCurrentObject, int)
{
    m_CurrentSection.Info.Type          = SECTIONTYPE_TEXT;
    m_CurrentSection.Info.Text.FontInfo = ParseFontInfo(pCurrentObject);
    m_CurrentSection.Info.Text.Color[0] = GetTextCharColor(pCurrentObject);
    m_CurrentSection.State.Text.Event   = 0;
    m_SectionV.push_back(m_CurrentSection);
}

void TokenBoard::ParseEmoticonObjectAttribute(
        const tinyxml2::XMLElement *pCurrentObject, int nSection,
        std::function<bool()> fnEmoticonRetrieve)
{
    m_CurrentSection.Info.Type                 = SECTIONTYPE_EMOTICON;
    m_CurrentSection.State.Emoticon.FrameIndex = 0;
    m_CurrentSection.State.Emoticon.MS         = 0;
    m_CurrentSection.Info.Emoticon.Set         = GetEmoticonSet(pCurrentObject);
    m_CurrentSection.Info.Emoticon.Index       = GetEmoticonIndex(pCurrentObject);

    // [][]  [][][][]  [][]  [][]  [][]  [][]
    // set   index     frame fps   count ratio
    //
    // []: 0-F
    //
    // ratio = H1 / (H1 + H2) * 256;
    fnEmoticonRetrieve(
            m_CurrentSection.Info.Emoticon.Set,        // input
            m_CurrentSection.Info.Emoticon.Index,      // input
            m_CurrentSection.Info.Emoticon.FPS,        // o
            m_CurrentSection.Info.Emoticon.FrameCount, // o
            m_CurrentSection.Info.Emoticon.YOffsetR);  // o

    m_SectionV.push_back(m_CurrentSection);
}

bool TokenBoard::ParseEmoticonObject(const tinyxml2::XMLElement *pCurrentObject, int nSection)
{
    ParseEmoticonObjectAttribute(pCurrentObject, nSection);

    // emoticon doesn't have content
    // all content has been moved to section part
    TOKENBOX stTokenBox;
    LoadEmoticonSizeCache(stTokenBox, nSection);

    return AddNewTokenBox(stTokenBox, m_PW);
}

bool TokenBoard::ParseEventTextObject(const tinyxml2::XMLElement *pCurrentObject, int nSection)
{
    m_HasEventText = true;

    ParseEventTextObjectAttribute(pCurrentObject, nSection);
    return ParseEventTextObjectContent(pCurrentObject, nSection);
}

void TokenBoard::UpdateSection(SECTION &stSection, Uint32 ticks)
{
    // only emoticon needs to be updated
    // but we add API here for further
    switch(stSection.Info.Type){
        case SECTIONTYPE_EMOTICON:
            {
                Uint32 nMaxTicks     = (Uint32)std::lround(1000.0 / stSection.Info.Emoticon.FPS);
                Uint32 nCurrentTicks = (Uint32)std::lround(1.0 * ticks + stSection.State.Emoticon.MS);
                int    nFrameIndex   = stSection.State.Emoticon.FrameIndex + nCurrentTicks / nMaxTicks;

                stSection.State.Emoticon.FrameIndex = nFrameIndex % stSection.Info.Emoticon.FrameCount;
                stSection.State.Emoticon.MS      = nCurrentTicks % nMaxTicks;
                break;
            }
        default:
            break;
    }
}

void TokenBoard::Update(Uint32 ticks)
{
    for(auto &stSection: m_SectionV){
        UpdateSection(stSection, ticks);
    }
}

int TokenBoard::SectionTypeCount(const std::vector<TOKENBOX> & vLine, int nSectionType)
{
    int nCount = 0;
    if(nSectionType != 0){
        for(const auto &stTokenBox: vLine){
            if(m_SectionV[stTokenBox.Section].Info.Type & nSectionType){
                nCount++;
            }
        }
        return nCount;
    }else{
        return m_CurrentLine.size();
    }
}

int TokenBoard::DoPadding(int nDWidth, int nSectionType)
{
    //
    // ++----------+------------+
    // |+        + |
    // ||        | |
    // |+        + |
    // +-----------+------------+
    //
    //  nDWidth      : recommended width
    //  nSectionType : specify SectionType for padding
    //
    // return should always be non-negative
    // meaning we can't be out of the boundary
    //
    // only do padding, won't calculate StartX/Y for simplity

    int nCount = SectionTypeCount(m_CurrentLine, nSectionType);
    if(m_CurrentLine.size() > 1 && nCount != 0){
        int nPaddingSpace = nDWidth / nCount;

        // first round
        // first token may reserve for no padding
        if(TokenBoxType(m_CurrentLine[0]) != TokenBoxType(m_CurrentLine[1])){
            if(nSectionType == 0 || TokenBoxType(m_CurrentLine[0]) & nSectionType){
                m_CurrentLine[0].State.W1 = 0;
                m_CurrentLine[0].State.W2 = nPaddingSpace / 2;

                nDWidth        -= nPaddingSpace / 2;
                m_CurrentWidth += nPaddingSpace / 2;
                if(nDWidth == 0){
                    return 0;
                }
            }
        }

        for(size_t nIndex = 1; nIndex < m_CurrentLine.size() - 1; ++nIndex){
            if(nSectionType == 0 || TokenBoxType(m_CurrentLine[nIndex]) & nSectionType){
                m_CurrentLine[nIndex].State.W1 = nPaddingSpace / 2;
                m_CurrentLine[nIndex].State.W2 = nPaddingSpace - nPaddingSpace / 2;

                nDWidth        -= nPaddingSpace;
                m_CurrentWidth += nPaddingSpace;
                if(nDWidth == 0){
                    return 0;
                }
            }
        }

        if(nSectionType == 0 || TokenBoxType(m_CurrentLine.back()) & nSectionType){
            m_CurrentLine.back().State.W1 = nPaddingSpace / 2;
            m_CurrentLine.back().State.W2 = 0;

            nDWidth        -= nPaddingSpace / 2;
            m_CurrentWidth += nPaddingSpace / 2;
            if(nDWidth == 0){
                return 0;
            }
        }

        // second round
        if(TokenBoxType(m_CurrentLine[0]) != TokenBoxType(m_CurrentLine[1])){
            if(nSectionType == 0 || TokenBoxType(m_CurrentLine[0]) & nSectionType){
                m_CurrentLine[0].State.W2++;;
                nDWidth--;
                m_CurrentWidth++;
                if(nDWidth == 0){
                    return 0;
                }
            }
        }

        for(size_t nIndex = 1; nIndex < m_CurrentLine.size() - 1; ++nIndex){
            if(nSectionType == 0 || TokenBoxType(m_CurrentLine[nIndex]) & nSectionType){
                if(m_CurrentLine[nIndex].State.W1 <=  m_CurrentLine[nIndex].State.W2){
                    m_CurrentLine[nIndex].State.W1++;
                }else{
                    m_CurrentLine[nIndex].State.W2++;
                }
                nDWidth--;
                m_CurrentWidth++;
                if(nDWidth == 0){
                    return 0;
                }
            }
        }

        if(nSectionType == 0 || TokenBoxType(m_CurrentLine.back()) & nSectionType){
            m_CurrentLine.back().State.W1++;
            nDWidth--;
            m_CurrentWidth++;
            if(nDWidth == 0){
                return 0;
            }
        }
    }
    return nDWidth;
}

void TokenBoard::SetTokenBoxStartY(std::vector<TOKENBOX> &vTokenBox, int nBaseLineY)
{
    int nCurrentX = 0;
    for(auto &stTokenBox: vTokenBox){
        stTokenBox.Cache.StartY = nBaseLineY - stTokenBox.Cache.H1;
    }
}

void TokenBoard::SetTokenBoxStartX(std::vector<TOKENBOX> &vTokenBox)
{
    int nCurrentX = 0;
    for(auto &stTokenBox: vTokenBox){
        nCurrentX += stTokenBox.State.W1;
        stTokenBox.Cache.StartX = nCurrentX;
        nCurrentX += stTokenBox.Cache.W;
        nCurrentX += stTokenBox.State.W2;
    }
}

int TokenBoard::SpacePadding(int paddingWidth)
{
    // return the real line width after padding
    // if paddingWidth < m_CurrentWidth
    // this function will do nothing and return m_CurrentWidth
    //
    // +----+-------------------+----+
    // |    |                   |    |
    // |    | ..........        |    |
    // +----+-------------------+----+

    int nDWidth = paddingWidth - (m_CurrentWidth + 1);
    if(nDWidth > 0){
        // try to padding all by emoticons
        int newDWidth = DoPadding(nDWidth, SECTIONTYPE_EMOTICON);
        if(newDWidth == nDWidth){
            // doesn't work, padding by all tokenbox
            newDWidth = DoPadding(nDWidth, 0);
            return m_CurrentWidth;
        }
    }
    return m_CurrentWidth;
}

bool TokenBoard::Add(TOKENBOX &stTokenBox, int nMaxWidth)
{
    // Emoticon itself has a box
    // and need to align to the baseline of words
    //                             
    //           +----------+             -----
    //           |          |               |  
    // +-------+ |          |               | H1
    // | Words | | Emoticon |  Words        |    
    // +-------+-+----------+---------------|--
    //           |          |               | H2
    //           +----------+             -----
    //                             

    if(m_Wrap && m_CurrentWidth + stTokenBox.Cache.W >= nMaxWidth){
        // when wrapping, width control is enabled
        return false;
    }else{
        stTokenBox.State.W1 = 0;
        stTokenBox.State.W2 = 0;
        // here it's copy
        m_CurrentLine.push_back(stTokenBox);
        m_CurrentWidth    += stTokenBox.Cache.W;
        m_CurrentLineMaxH2 = (std::max)(m_CurrentLineMaxH2, stTokenBox.Cache.H2);
        return true;
    }
}

void TokenBoard::ResetCurrentLine()
{
    m_CurrentLine.clear();
    m_CurrentWidth     = 0;
    m_CurrentLineMaxH2 = 0;
}

int TokenBoard::GetNthLineIntervalMaxH2(int nthLine, int nIntervalStartX, int nIntervalWidth)
{
    //    This function only take care of nthLine
    //    has nothing to do with (n-1)th or (n+1)th Line
    //    TODO: we ignore the following case here:
    //
    // ---------+----+----+-------------+-------  <-BaseLine
    // |        |    |    |             |    
    // |        | W2 | W1 |             |       
    // |        |    +----+-------------+
    // |        |    |
    // +--------+----+                     
    //             |    |
    //             |    |
    //             |    |
    //        +----+----+-----+
    //        | W1 |    | W2  |
    //        |  ->|    |<-   |
    //            Interval
    //
    //  Here we take the padding space as part of the token box.
    //  If not, token in (n + 1)-th line may go through this space
    //  keep this as it is, and update it later

    int   nMaxH2    = -1;
    int   nCurrentX =  0;

    for(auto &stTokenBox: m_LineV[nthLine]){
        int nW  = stTokenBox.Cache.W;
        int nH  = stTokenBox.Cache.H;
        int nH1 = stTokenBox.Cache.H1;
        int nH2 = stTokenBox.Cache.H2;
        int nW1 = stTokenBox.State.W1;
        int nW2 = stTokenBox.State.W2;

        if(IntervalOverlapped(nCurrentX, nCurrentX + nW1 + nW + nW2,
                    nIntervalStartX, nIntervalStartX + nIntervalWidth)){
            nMaxH2 = (std::max)(nMaxH2, nH2);
        }
        nCurrentX += (nW1 + nW + nW2);
    }
    // maybe the line is not long enough to cover Interval
    // in this case we return -1
    return nMaxH2;
}

int TokenBoard::GetNthLineTokenBoxStartY(int nthLine, int nStartX, int nBoxWidth, int nBoxHeight)
{
    while(nthLine - 1 >= 0){
        int nMaxH2 = GetNthLineIntervalMaxH2(nthLine - 1, nStartX, nBoxWidth);
        if(nMaxH2 >= 0){
            return m_LineStartY[nthLine - 1] + nBoxHeight + nMaxH2;
        }
        nthLine--;
    }
    // there is no line upside
    return nBoxHeight;
}

int TokenBoard::GetNthNewLineStartY(int nthLine)
{
    // m_CurrentLine is padded already
    //
    //                             
    //           +----------+                  
    //           |          |                  
    // +-------+ |          |                   
    // | Words | | Emoticon |  Words             
    // +-------+-+----------+------------------
    //           |          |   +----------+  
    //           +----------+   |          |
    //                          |          |                  
    //                +-------+ |          |                   
    //                | Words | | Emoticon |  Words             
    //                +-------+-+----------+-------           
    //                          |          |                   
    //                          +----------+                  
    //                                            
    //
    int nCurrentX =  0;
    int nCurrentY = -1;
    for(auto &stTokenBox: m_CurrentLine){
        int nW  = stTokenBox.Cache.W;
        int nH  = stTokenBox.Cache.H;
        int nH1 = stTokenBox.Cache.H1;
        int nH2 = stTokenBox.Cache.H2;
        int nW1 = stTokenBox.State.W1;
        int nW2 = stTokenBox.State.W2;

        nCurrentY  = (std::max)(nCurrentY,
                GetNthLineTokenBoxStartY(nthLine, nCurrentX + nW1, nW, nH1));
        nCurrentX += (nW1 + nW + nW2);
    }
    return nCurrentY;
}


void TokenBoard::LoadUTF8CharBoxSizeCache(TOKENBOX &rstTokenBox,
        int nSection, int nDefaultSize, uint32_t nFontAttrKey, uint32_t nUTF8Key, 
        std::function<void(bool, uint64_t, int &, int &, int &, int &)> fnQueryTokenBoxInfo)
{
    uint64_t nKey = (((uint64_t)nFontAttrKey << 32) + nUTF8Key);

    rstTokenBox.Section = nSection;
    rstTokenBox.UTF8CharBox.Data.UTF8Code = nUTF8Key;
    rstTokenBox.UTF8CharBox.Cache.Key     = nKey;

    if(!fnQueryFontexInfo(true, nKey,
                stTokenBox.Cache.W, stTokenBox.Cache.H,
                stTokenBox.Cache.H1, stTokenBox.Cache.H2)){
        // failed to retrieve, set a []
        stTokenBox.Cache.W  = nDefaultSize;
        stTokenBox.Cache.H  = nDefaultSize;
        stTokenBox.Cache.H1 = nDefaultSize;
        stTokenBox.Cache.H2 = 0;
    }
}

void TokenBoard::LoadEmoticonSizeCache(TOKENBOX &rstTokenBox,
        int nSection, int nDefaultSize, uint8_t nEmoticonSet, uint16_t nEmoticonIndex,
        std::function<void(bool, uint64_t, int &, int &, int &, int &)> fnQueryTokenBoxInfo)
{
    // take the first frame for size info
    uint32_t nKey = ((uint32_t)nEmoticonSet << 24) + ((uint32_t)nEmoticonIndex << 8);

    if(!fnQueryTokenBoxInfo(false, (uint64_t)nKey,
                stTokenBox.Cache.W, stTokenBox.Cache.H, stTokenBox.Cache.H1, stTokenBox.Cache.H2)){
        stTokenBox.Cache.W = nDefaultSize;
        stTokenBox.Cache.H = nDefaultSize;
        stTokenBox.Cache.H1 = std::lround(nDefaultSize * 0.7);
        stTokenBox.Cache.H2 = std::lround(nDefaultSize * 0.3);
    }
}

// isolate it from any SDL interface:
//
// 1. external functions won't see SECTIONTYPE_XXXX
// 2. internal functions won't see SDL_XXXX
void TokenBoard::LoadUTF8CharBoxSizeCache(TOKENBOX &rstTokenBox,
        int nSection, int nDefaultSize, uint32_t nFontAttrKey, uint32_t nUTF8Key, 
        std::function<void(bool, uint64_t, int &, int &, int &, int &)> fnQueryTokenBoxInfo)
{
    uint64_t nKey = (((uint64_t)nFontAttrKey << 32) + nUTF8Key);

    rstTokenBox.Section = nSection;
    rstTokenBox.UTF8CharBox.Data.UTF8Code = nUTF8Key;
    rstTokenBox.UTF8CharBox.Cache.Key     = nKey;

    if(!fnQueryFontexInfo(true, nKey,
                stTokenBox.Cache.W, stTokenBox.Cache.H,
                stTokenBox.Cache.H1, stTokenBox.Cache.H2)){
        // failed to retrieve, set a []
        stTokenBox.Cache.W  = nDefaultSize;
        stTokenBox.Cache.H  = nDefaultSize;
        stTokenBox.Cache.H1 = nDefaultSize;
        stTokenBox.Cache.H2 = 0;
    }
}

// everything dynamic and can be retrieve is in Cache
//            dynamic but can not be retrieved is in State
//            static is in Info
void TokenBoard::DrawEx(
        int nDstX, int nDstY, // start position of drawing on the screen
        int nSrcX, int nSrcY, // region to draw, a cropped region on the token board
        int nSrcW, int nSrcH,
        std::function<void(uint64_t, int, int, int, int, int, int)> fnDrawEx)
{
    // we assume all token box are well-prepared here!
    // means cache, state, info are all valid now when invoke this function
    //

    // if(false
    //         || nSrcX >= m_W
    //         || nSrcY >= m_H
    //         || nSrcX + nDstW <= 0
    //         || nSrcY + nDstH <= 0){
    //     return;
    // }
    //
    // if(nSrcX < 0){
    //     nSrcW += nSrcX;
    //     nSrcX  = 0;
    // }
    //
    // if(nSrcY < 0){
    //     nSrcH += nSrcY;
    //     nSrcY  = 0;
    // }
    //
    // if(nSrcX + nSrcW > m_W){
    //     nSrcW = m_W - nSrcX;
    // }
    //
    // if(nSrcY + nSrcH > m_H){
    //     nSrcH = m_H - nSrcY;
    // }

    // now nSrc(X, Y, W, H) is a sub-area on the board

    for(int nLine = 0; nLine < m_LineV.size(); ++nLine){
        for(auto &rstTokenBox: m_LineV[nLine]){


            switch(m_SectionV[rstTokenBox.Section].Info.Type){
                case SECTIONTYPE_EVENTTEXT:
                    {
                        // I had a hard time here
                        //
                        // TODO
                        //
                        // the purpose of introducing FontexDB is for saving video memory
                        // not to support hot source data switch!
                        //
                        // assumptions:
                        //  1. if a fontex can't be retrieved, it can never be retrieved
                        //     that's to say, the data source is fixed
                        //
                        //  2. if a fontex can be retrieved, then it can be retrieved everytime
                        //     the fontex release and restore are transparant to token board
                        //
                        //  3. if a token box cache is invalid, we need to set it at place
                        //     but if it's valid, it always matches the data
                        //
                        //     that to say, the cache data set by fontex are always the same
                        //     we can't steal the data source and replace by a new one at runtime
                        //


                        int nX, nY, nW, nH;
                        nX = rstTokenBox.Cache.StartX;
                        nY = rstTokenBox.Cache.StartY;
                        nW = rstTokenBox.Cache.W - rstTokenBox.State.W1 - rstTokenBox.State.W2;
                        nH = rstTokenBox.Cache.H1;

                        if(!RectangleOverlapRegion(nSrcX, nSrcY, nSrcW, nSrcH, nX, nY, nW, nH)){
                            // no overlap for current, nothing to do
                            continue;
                        }

                        fnDrawTokenBox(
                                true,
                                rstTokenBox.UTF8CharBox.Cache.Valid,
                                rstTokenBox.UTF8CharBox.Cache.Key,
                                nX + nDstX,
                                nY + nDstY,
                                nX - rstTokenBox.Cache.StartX,
                                nY - rstTokenBox.Cache.StartY,
                                nW, nH,
                                nColor);

                        break;
                    }

                case SECTIONTYPE_EMOTICON:
                    {
                        int nX, nY, nW, nH;
                        nX = rstTokenBox.Cache.StartX;
                        nY = rstTokenBox.Cache.StartY;
                        nW = rstTokenBox.Cache.W - rstTokenBox.State.W1 - rstTokenBox.State.W2;
                        nH = rstTokenBox.Cache.H;

                        if(!RectangleOverlapRegion(nSrcX, nSrcY, nSrcW, nSrcH, nX, nY, nW, nH)){
                            // no overlap for current, nothing to do
                            continue;
                        }

                        int nFrameIndex = m_SectionV[rstTokenBox.Section].State.Emoticon.FrameIndex;

                        fnDrawTokenBox(
                                false,
                                rstTokenBox.EmoticonBox.Cache.Valid,
                                rstTokenBox.EmoticonBox.Cache.Key + nFrameIndex,
                                nX + nDstX,
                                nY + nDstY,
                                nX - rstTokenBox.Cache.StartX,
                                nY - rstTokenBox.Cache.StartY,
                                nW, nH, 0);

                        break;
                    }
                default:
                    break;
            }
        }
    }
}

void TokenBoard::AddNewTokenBoxLine(bool bEndByReturn)
{
    if(m_Wrap && bEndByReturn == false){
        SpacePadding(m_PW);
    }

    // set X cache
    SetTokenBoxStartX(m_CurrentLine);

    // always align lines to left
    // otherwise we need m_LineStartX
    // but GetNthNewLineStartY() need TOKENBOX::Cache.StartX
    // kind of inconsistent

    m_LineStartY.push_back(GetNthNewLineStartY(m_LineV.size()));

    // set Y cache
    SetTokenBoxStartY(m_CurrentLine, m_LineStartY.back());

    m_LineV.emplace_back(std::move(m_CurrentLine));

    // for wrap or not
    m_W = (std::max)(m_W, m_CurrentWidth);
    m_H = m_LineStartY.back() + m_CurrentLineMaxH2 + 1;

    // this is a must for AddNewTokenBox() to end the recursion
    ResetCurrentLine();
}

bool TokenBoard::AddNewTokenBox(TOKENBOX &rstTokenBox, int nMaxWidth)
{
    if(Add(rstTokenBox, nMaxWidth)){
        return true;
    }else{
        // try to add but failed
        if(m_CurrentLine.empty()){
            // allowed width is tooo small
            // won't fix it, just fail and leave
            return false;
        }else{
            // 1. insert a finished line
            // 2. recursively call itself to add the token again
            AddNewTokenBoxLine(false);
            // the depth of recursion at most is 2
            return AddNewTokenBox(rstTokenBox, nMaxWidth);
        }
    }
}

bool TokenBoard::ProcessEvent(int nFrameStartX, int nFrameStartY, const SDL_Event &rstEvent)
{
    if(m_SkipEvent){
        return false;
    }

    // field ``Event" is only for rendering 0/1/2
    // need more information for checking clicks
    //
    bool bClickUp = false;

    int nEventType, nEventX, nEventY, nEventDX, nEventDY;

    switch(rstEvent.type){
        case SDL_MOUSEBUTTONUP:
            nEventType = 1;
            nEventX    = rstEvent.button.x;
            nEventY    = rstEvent.button.y;
            bClickUp   = true;
            break;
        case SDL_MOUSEBUTTONDOWN:
            nEventType = 2;
            nEventX    = rstEvent.button.x;
            nEventY    = rstEvent.button.y;
            break;
        case SDL_MOUSEMOTION:
            nEventType = 1;
            nEventX    = rstEvent.motion.x;
            nEventY    = rstEvent.motion.y;
            break;
        default:
            return false;
    }

    nEventDX = nEventX - nFrameStartX;
    nEventDY = nEventY - nFrameStartY;

    // use the cached hot spot data for text box
    // first check last token box
    //
    if(m_LastTokenBox){
        // 
        // W1 and W2 should also count in
        // otherwise mouse can escape from the flaw of two contiguous tokens
        // means when move mouse horizontally, event text will turn on and off
        //
        int nX = m_LastTokenBox->Cache.StartX - m_LastTokenBox->State.W1;
        int nY = m_LastTokenBox->Cache.StartY;
        int nW = m_LastTokenBox->Cache.W;
        int nH = m_LastTokenBox->Cache.H;

        if(PointInRectangle(nEventDX, nEventDY, nX, nY, nW, nH)){
            // 
            // save the old state for callback function
            int nOldEvent = m_SectionV[m_LastTokenBox->Section].State.Text.Event;
            //
            // set as current event type
            m_SectionV[m_LastTokenBox->Section].State.Text.Event = nEventType;

            // previous state is ``pressed" and now is ``releasing"
            if(nOldEvent == 2 && bClickUp){
                auto &rstLabel = m_SectionV[m_LastTokenBox->Section].State.Text.ID;
                // this will be slow since the key is string
                // I need more experiments to decide whether to improve it or not
                if(!rstLabel.empty() && m_Callback.find(rstLabel) != m_Callback.end()){
                    m_Callback[rstLable]();
                }
            }
            // done, captured event
            return true;
        }else{
            // set as ``out"
            m_SectionV[m_LastTokenBox->Section].State.Text.Event  = 0;
            m_SectionV[m_LastTokenBox->Section].State.Text.Update = true;
        }
    }

    // then try resolution grid directly
    // here we don't need to try something like ``last resolution grid"
    // since the are of the same expense
    //
    if(PointInRectangle(nEventX, nEventY, nFrameStartX, nFrameStartY, W(), H())){
        int nGridX = nEventDX / m_Resolution;
        int nGridY = nEventDY / m_Resolution;

        // only put EventText Section in Bitmap
        // emoticon won't accept events
        //
        for(auto &rstPair: m_TokenBoxBitmap[nGridX][nGridY]){
            //
            // for each possible tokenbox in the grid
            //
            // this is enough since cover of all tokenboxs in one gird
            // is much larger than the grid itself
            const auto &rstTokenBox = m_LineV[rstPair.first][rstPair.second];

            int nStartX = rstTokenBox.Cache.StartX - rstTokenBox.State.W1;
            int nStartY = rstTokenBox.Cache.StartY;
            int nW      = rstTokenBox.Cache.W;
            int nH      = rstTokenBox.Cache.H;

            if (PointInRectangle(nEventX, nEventY, nStartX, nStartY, nW, nH)){
                // save the old state
                //
                int nOldState = m_SectionV[rstTokenBox.Section].State.Text.Event;
                //
                // set event and last hot spot
                m_SectionV[rstTokenBox.Section].State.Text.Event = nEventType;
                m_LastTokenBox = &rstTokenBox;

                // previous state is ``pressed" and now is ``releasing"
                if(nOldEvent == 2 && bClickUp){
                    auto &rstLabel = m_SectionV[m_LastTokenBox->Section].State.Text.ID;
                    // this will be slow since the key is string
                    // I need more experiments to decide whether to improve it or not
                    if(!rstLabel.empty() && m_Callback.find(rstLabel) != m_Callback.end()){
                        m_Callback[rstLable]();
                    }
                }
                // done, captured event
                return true;
            }
        }
        // in board but not in any section
        // capture the event, but we need do nothing
        return true;
    }

    // even not in board
    // we don't need to set previous section by ``out" seperately
    // since when examing ``m_LastTokenBox"
    // we have already done for this setting
    //
    return false;
}

int TokenBoard::TokenBoxType(const TOKENBOX &rstTokenBox)
{
    return m_SectionV[rstTokenBox.Section].Info.Type;
}

void TokenBoard::MakeEventTextBitmap()
{
    // only make it for EventText since only it accepts event
    //
    // TODO
    // maybe emoticons will also accept events
    // but that introduces more issues
    //    1. what to show the difference for press/release/out?
    //    2. what frame index should be set when pressed?
    //    3. ...
    // this is too much, so just disable it in the design
    //

    int nGW = (W() + (m_Resolution - 1)) / m_Resolution;
    int nGH = (H() + (m_Resolution - 1)) / m_Resolution;

    // this should be easier to clarify
    m_TokenBoxBitmap.resize(nGW);
    for(int nGX = 0; nGX < nGW; ++nGX){
        m_TokenBoxBitmap[nGX].resize(nGH, {});
    }

    for(int nX = 0; nX < m_TokenBoxBitmap.size(); ++nX){
        for(int nY = 0; nY < m_TokenBoxBitmap[0].size(); ++nY){
            if(TokenBoxType(nX, nY) == SECTIONTYPE_EVENTTEXT){
                MarkEventTextBitmap(nX, nY);
            }
        }
    }
}

void TokenBoard::MarkEventTextBitmap(const TOKENBOX &rstTokenBox)
{
    // put the whole box (with the margin) to the bitmap
    // not only the real box inside
    //
    // E : emoticon
    // T : text
    //
    //              |<---Cache.W---->|
    //              |                |
    //    ---- ---- +---+--------+---+  <--- Cache.StartY
    //     ^    ^   |   |        |   |
    //     |    |   |   |        |   +---+-----+---+
    //     |   H1   |   |        |   |   |     |   |
    //     |    |   |   |   E    |   |   |  T  |   |
    //     H    |   |   |        |   |   |     |   |
    //     |   -x---+---+--------+---+---+-----+---+-             
    //     |    |   |   |        |   |
    //     |   H2   |   |        |   |
    //     V    V   |   |        |   |
    //    ---- -----+---+--------+---+
    //              ^   ^
    //              |   |
    //              |   |
    //              |   +--Cache.StartX
    //              +------Cache.StartX - State.W1

    // get the whole box's (x, y, w, h), with the margin
    //
    int nX = rstTokenBox.Cache.StartX - rstTokenBox.State.W1;
    int nY = rstTokenBox.Cache.StartY;
    int nW = rstTokenBox.Cache.W;
    int nH = rstTokenBox.Cache.H;

    // map it to all the grid box
    // maybe more than one
    int nGX1 = nX / m_Resolution;
    int nGY1 = nY / m_Resolution;
    int nGX2 = (nX + nW) / m_Resolution;
    int nGY2 = (nY + nH) / m_Resolution;

    for(int nX = nGX1; nX <= nGX2; ++nX){
        for(int nY = nGY1; nY <= nGY2; ++nY){
            m_TokenBoxBitmap[nX][nY].emplace_back(&rstTokenBox);
        }
    }
}

int TokenBoard::GuessResoltion()
{
    // TODO
    // make this function more reasonable and functional
    return 20;
}
