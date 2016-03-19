/*
 * =====================================================================================
 *
 *       Filename: tokenboard.cpp
 *        Created: 06/17/2015 10:24:27 PM
 *  Last Modified: 03/18/2016 18:32:24
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
#include "emoticondbn.hpp"
#include "fontexdbn.hpp"
#include "section.hpp"
#include "tokenboard.hpp"
#include "tokenbox.hpp"
#include "utf8char.hpp"

#include <SDL2/SDL.h>
#include <algorithm>
#include <functional>
#include <tinyxml2.h>
#include <utf8.h>
#include <unordered_map>
#include <string>
#include <cassert>

TokenBoard::TokenBoard(bool bWrap, int nMaxWidth, int nMinTextMargin)
    : m_PW(nMaxWidth)
    , m_W(0)
    , m_H(0)
    , m_Wrap(bWrap)
    , m_CurrentWidth(0)
    , m_SkipEvent(false)
    , m_Resolution(20)
    , m_MinTextMargin(nMinTextMargin)
{
}

bool TokenBoard::Load(const tinyxml2::XMLDocument &rstDoc,
        const std::unordered_map<std::string, std::function<void()>> &rstIDhandlerMap)
{
    const tinyxml2::XMLElement *pRoot = rstDoc.RootElement();
    if (pRoot == nullptr){ return false; }

    const tinyxml2::XMLElement *pCurrentObject = nullptr;
    if(false){
    }else if((pCurrentObject = pRoot->FirstChildElement("object"))){
    }else if((pCurrentObject = pRoot->FirstChildElement("Object"))){
    }else if((pCurrentObject = pRoot->FirstChildElement("OBJECT"))){
    }else{
        return false;
    }

    int nSection = 0;

    while(pCurrentObject){
        if(ObjectReturn(*pCurrentObject)){
            // so for description like 
            // <object type = "return"></object><object type = "return"></object>
            // we can only get one new line
            if(!m_CurrentLine.empty()){
                AddNewTokenBoxLine(true);
            }
        }else if(ObjectEmocticon(*pCurrentObject)){
            ParseEmoticonObject(*pCurrentObject, nSection++, rstIDhandlerMap);
        }else if(ObjectEventText(*pCurrentObject)){
            ParseEventTextObject(*pCurrentObject, nSection++, rstIDhandlerMap);
        }else{
            // failed to parse something
        }
        pCurrentObject = NextObject(pCurrentObject);
    }

    if(!m_CurrentLine.empty()){
        AddNewTokenBoxLine(true);
    }

    if(!m_SkipEvent){
        MakeEventTextBitmap();
    }

    return true;
}

int TokenBoard::W()
{
    return m_W;
}

int TokenBoard::H()
{
    return m_H;
}

bool TokenBoard::ObjectReturn(const tinyxml2::XMLElement &rstCurrentObject)
{
    return false
        || (rstCurrentObject.Attribute("TYPE", "RETURN"))
        || (rstCurrentObject.Attribute("TYPE", "Return"))
        || (rstCurrentObject.Attribute("TYPE", "return"))
        || (rstCurrentObject.Attribute("Type", "RETURN"))
        || (rstCurrentObject.Attribute("Type", "Return"))
        || (rstCurrentObject.Attribute("Type", "return"))
        || (rstCurrentObject.Attribute("type", "RETURN"))
        || (rstCurrentObject.Attribute("type", "Return"))
        || (rstCurrentObject.Attribute("type", "return"));
}

bool TokenBoard::ObjectEmocticon(const tinyxml2::XMLElement &rstCurrentObject)
{
    return false
        || (rstCurrentObject.Attribute("TYPE", "Emoticon"))
        || (rstCurrentObject.Attribute("TYPE", "emoticon"))
        || (rstCurrentObject.Attribute("TYPE", "EMOTICON"))
        || (rstCurrentObject.Attribute("Type", "Emoticon"))
        || (rstCurrentObject.Attribute("Type", "emoticon"))
        || (rstCurrentObject.Attribute("Type", "EMOTICON"))
        || (rstCurrentObject.Attribute("type", "Emoticon"))
        || (rstCurrentObject.Attribute("type", "emoticon"))
        || (rstCurrentObject.Attribute("type", "EMOTICON"));
}

bool TokenBoard::ObjectEventText(const tinyxml2::XMLElement &rstCurrentObject)
{
    return false
        || (rstCurrentObject.Attribute("Type") == nullptr)
        || (rstCurrentObject.Attribute("type") == nullptr)
        || (rstCurrentObject.Attribute("TYPE") == nullptr)
        || (rstCurrentObject.Attribute("TYPE", "EVENTTEXT"))
        || (rstCurrentObject.Attribute("TYPE", "EventText"))
        || (rstCurrentObject.Attribute("TYPE", "Eventtext"))
        || (rstCurrentObject.Attribute("TYPE", "eventText"))
        || (rstCurrentObject.Attribute("TYPE", "eventtext"))
        || (rstCurrentObject.Attribute("Type", "EVENTTEXT"))
        || (rstCurrentObject.Attribute("Type", "EventText"))
        || (rstCurrentObject.Attribute("Type", "Eventtext"))
        || (rstCurrentObject.Attribute("Type", "eventText"))
        || (rstCurrentObject.Attribute("Type", "eventtext"))
        || (rstCurrentObject.Attribute("type", "EVENTTEXT"))
        || (rstCurrentObject.Attribute("type", "EventText"))
        || (rstCurrentObject.Attribute("type", "Eventtext"))
        || (rstCurrentObject.Attribute("type", "eventText"))
        || (rstCurrentObject.Attribute("type", "eventtext"));
}

int TokenBoard::GetEmoticonSet(const tinyxml2::XMLElement &rstCurrentObject)
{
    const char *pText = nullptr;

    if(false){
    }else if((pText = rstCurrentObject.Attribute("SET"))){
    }else if((pText = rstCurrentObject.Attribute("Set"))){
    }else if((pText = rstCurrentObject.Attribute("set"))){
    }else{
        pText = "0";
    }
    // here means when set can not be found or conversion fails
    // return 0
    return std::atoi(pText);
}

int TokenBoard::GetEmoticonIndex(const tinyxml2::XMLElement &rstCurrentObject)
{
    const char *pText = nullptr;

    if(false){
    }else if((pText = rstCurrentObject.Attribute("INDEX"))){
    }else if((pText = rstCurrentObject.Attribute("Index"))){
    }else if((pText = rstCurrentObject.Attribute("index"))){
    }else{
        pText = "0";
    }
    return std::atoi(pText);
}

const tinyxml2::XMLElement *TokenBoard::NextObject(const tinyxml2::XMLElement *pCurrentObject)
{
    const tinyxml2::XMLElement *pRet = nullptr;
    if(false){
    }else if((pRet = pCurrentObject->NextSiblingElement("OBJECT"))){
    }else if((pRet = pCurrentObject->NextSiblingElement("Object"))){
    }else if((pRet = pCurrentObject->NextSiblingElement("object"))){
    }else{
        pRet = nullptr;
    }
    return pRet;
}

int TokenBoard::GetFontSize(const tinyxml2::XMLElement &rstCurrentObject)
{
    const char *pText            = nullptr;
    const int   nDefaultFontSize = 18;

    if(false){
    }else if((pText = rstCurrentObject.Attribute("SIZE"))){
    }else if((pText = rstCurrentObject.Attribute("Size"))){
    }else if((pText = rstCurrentObject.Attribute("size"))){
    }else{
        return nDefaultFontSize;
    }
    int nFontSize = std::atoi(pText);
    return nFontSize == 0 ? nDefaultFontSize : nFontSize;
}

int TokenBoard::GetFontIndex(const tinyxml2::XMLElement &rstCurrentObject)
{
    const char *pText;

    if(false){
    }else if((pText = rstCurrentObject.Attribute("FONT"))){
    }else if((pText = rstCurrentObject.Attribute("Font"))){
    }else if((pText = rstCurrentObject.Attribute("font"))){
    }else{
        // default to set FontIndex to be zero
        pText = "0";
    }
    return std::atoi(pText);
}

int TokenBoard::GetFontStyle(const tinyxml2::XMLElement &rstCurrentObject)
{
    const char *pText;

    if(false){
    }else if((pText = rstCurrentObject.Attribute("STYLE"))){
    }else if((pText = rstCurrentObject.Attribute("Style"))){
    }else if((pText = rstCurrentObject.Attribute("style"))){
    }else{
        pText = "0";
    }

    return std::atoi(pText);
}

SDL_Color TokenBoard::GetEventTextCharColor(const tinyxml2::XMLElement &rstCurrentObject, int nEvent)
{
    if(nEvent < 0 || nEvent > 2){
        // for non-event text, try to parse ``color" attribute
        // if failed, return pure white
        return GetEventTextCharColor(rstCurrentObject);
    }

    const char *attrText[3][3] = {
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
    }else if((pText = rstCurrentObject.Attribute(attrText[nEvent][0]))){
    }else if((pText = rstCurrentObject.Attribute(attrText[nEvent][1]))){
    }else if((pText = rstCurrentObject.Attribute(attrText[nEvent][2]))){
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

// no event version, try to parse ``color" attribute for non-event text
SDL_Color TokenBoard::GetEventTextCharColor(const tinyxml2::XMLElement &rstCurrentObject)
{
    const char *pText;
    SDL_Color   color;

    if(false){
    }else if((pText = rstCurrentObject.Attribute("COLOR"))){
    }else if((pText = rstCurrentObject.Attribute("Color"))){
    }else if((pText = rstCurrentObject.Attribute("color"))){
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

bool TokenBoard::ParseEmoticonObject(const tinyxml2::XMLElement &rstCurrentObject,
        int/*nSection*/, const std::unordered_map<std::string, std::function<void()>> &/*rstIDhandlerMap*/)
{
    // currently I didn't support event response for emoticon box
    // since my idea is not ready for it
    // so just put a null handler here
    m_IDFuncV.push_back(std::function<void()>());

    SECTION  stSection;
    TOKENBOX stTokenBox;
    std::memset(&stSection , 0, sizeof(stSection) );
    std::memset(&stTokenBox, 0, sizeof(stTokenBox));

    stSection.Info.Type                 = SECTIONTYPE_EMOTICON;
    stSection.State.Emoticon.FrameIndex = 0;
    stSection.State.Emoticon.MS         = 0.0;
    stSection.Info.Emoticon.Set         = GetEmoticonSet(rstCurrentObject);
    stSection.Info.Emoticon.Index       = GetEmoticonIndex(rstCurrentObject);

    extern EmoticonDBN *g_EmotionDBN;
    auto pTexture = g_EmotionDBN->Retrieve(
            stSection.Info.Emoticon.Set,          // emoticon set
            stSection.Info.Emoticon.Index,        // emoticon index
            0,                                    // first frame of the emoticon
            nullptr,                              // don't need the start info here
            nullptr,                              // don't need the satrt info here
            &stTokenBox.Cache.W,
            &stTokenBox.Cache.H,
            &stSection.Info.Emoticon.FPS,         // o
            &stTokenBox.Cache.H1,                 // o
            &stSection.Info.Emoticon.FrameCount); // o

    if(pTexture){
        // this emoticon is valid
        stTokenBox.Cache.H1            = stTokenBox.Cache.H - stTokenBox.Cache.H2;
        stTokenBox.State.Valid         = 1;
        stSection.State.Emoticon.Valid = 1;
    }

    m_SectionV.push_back(stSection);
    return AddNewTokenBox(stTokenBox);
}

bool TokenBoard::ParseEventTextObject(const tinyxml2::XMLElement &rstCurrentObject,
        int nSection, const std::unordered_map<std::string, std::function<void()>> &rstIDhandlerMap)
{
    SECTION stSection;
    std::memset(&stSection, 0, sizeof(stSection));

    // parse event text section desc
    stSection.Info.Type           = SECTIONTYPE_EVENTTEXT;
    stSection.Info.Text.FileIndex = GetFontIndex(rstCurrentObject);
    stSection.Info.Text.Style     = GetFontStyle(rstCurrentObject);
    stSection.Info.Text.Size      = GetFontSize(rstCurrentObject);

    std::function<void()> fnCallback;
    const char *szID = rstCurrentObject.Attribute("ID");

    if(szID){
        // add a handler here
        auto pFuncInst = rstIDhandlerMap.find(std::string(szID));
        if(pFuncInst != rstIDhandlerMap.end()){
            fnCallback = pFuncInst->second;
        }
    }

    m_IDFuncV.push_back(fnCallback);

    stSection.Info.Text.Color[0] = GetEventTextCharColor(rstCurrentObject, (szID) ? 0 : (-1));
    stSection.Info.Text.Color[1] = GetEventTextCharColor(rstCurrentObject, (szID) ? 1 : (-1));
    stSection.Info.Text.Color[2] = GetEventTextCharColor(rstCurrentObject, (szID) ? 2 : (-1));

    stSection.State.Text.Event = 0;
    m_SectionV.push_back(stSection);

    // then we parse event text content
    const char *pStart = rstCurrentObject.GetText();
    const char *pEnd   = pStart;

    // when get inside this funciton
    // the section structure has been well-prepared
    //
    int nDefaultSize = (int)m_SectionV[nSection].Info.Text.Size;

    uint32_t nFontAttrKey = 0
        + (((uint64_t)m_SectionV[nSection].Info.Text.FileIndex) << 16)
        + (((uint64_t)m_SectionV[nSection].Info.Text.Size)      <<  8)
        + (((uint64_t)m_SectionV[nSection].Info.Text.Style)     <<  0);


    while(*pEnd != '\0'){
        pStart = pEnd;
        utf8::unchecked::advance(pEnd, 1);

        // should be true
        assert(pEnd - pStart <= 4);

        uint32_t nUTF8Key = 0;
        std::memcpy(&nUTF8Key, pStart, pEnd - pStart);

        // fully set the token box unit
        TOKENBOX stTokenBox;
        std::memset(&stTokenBox, 0, sizeof(stTokenBox));

        uint64_t nKey = (((uint64_t)nFontAttrKey << 32) + nUTF8Key);

        stTokenBox.Section = nSection;
        stTokenBox.UTF8CharBox.UTF8Code  = nUTF8Key;
        stTokenBox.UTF8CharBox.Cache.Key = nKey;

        extern FontexDBN *g_FontexDBN;
        auto pTexture = g_FontexDBN->Retrieve(nKey);
        if(pTexture){
            SDL_QueryTexture(pTexture, nullptr, nullptr, &stTokenBox.Cache.W, &stTokenBox.Cache.H);
            stTokenBox.Cache.H1    = stTokenBox.Cache.H;
            stTokenBox.Cache.H2    = 0;
            stTokenBox.State.Valid = 1;
        }else{
            // failed to retrieve, set a []
            stTokenBox.Cache.W  = nDefaultSize;
            stTokenBox.Cache.H  = nDefaultSize;
            stTokenBox.Cache.H1 = nDefaultSize;
            stTokenBox.Cache.H2 = 0;
        }
        AddNewTokenBox(stTokenBox);
    }
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
    //
    //  keep this as it is, and update it later

    int   nMaxH2    = -1;
    int   nCurrentX =  0;

    for(auto &stTokenBox: m_LineV[nthLine]){
        int nW  = stTokenBox.Cache.W;
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
        int nH1 = stTokenBox.Cache.H1;
        int nW1 = stTokenBox.State.W1;
        int nW2 = stTokenBox.State.W2;

        nCurrentY  = (std::max)(nCurrentY,
                GetNthLineTokenBoxStartY(nthLine, nCurrentX + nW1, nW, nH1));
        nCurrentX += (nW1 + nW + nW2);
    }
    return nCurrentY;
}

// everything dynamic and can be retrieve is in Cache
//            dynamic but can not be retrieved is in State
//            static is in Info
void TokenBoard::DrawEx(
        int nDstX, int nDstY, // start position of drawing on the screen
        int nSrcX, int nSrcY, // region to draw, a cropped region on the token board
        int nSrcW, int nSrcH)
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

    for(int nLine = 0; nLine < (int)m_LineV.size(); ++nLine){
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

                        if(!RectangleOverlapRegion(nSrcX, nSrcY, nSrcW, nSrcH, &nX, &nY, &nW, &nH)){
                            // no overlap for current, nothing to do
                            continue;
                        }

                        extern SDLDevice *g_SDLDevice;
                        extern FontexDBN *g_FontexDBN;
                        g_SDLDevice->DrawImage(
                                g_FontexDBN->Retrieve(rstTokenBox.UTF8CharBox.Cache.Key),
                                nX + nDstX, nY + nDstY, nW, nH, nColor);
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
                                nX + nDstX, nY + nDstY, nW, nH);
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

bool TokenBoard::AddNewTokenBox(TOKENBOX &rstTokenBox)
{
    if(Add(rstTokenBox, m_PW)){
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
