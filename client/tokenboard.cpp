/*
 * =====================================================================================
 *
 *       Filename: tokenboard.cpp
 *        Created: 6/17/2015 10:24:27 PM
 *  Last Modified: 08/21/2015 10:52:00 PM
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

#include <SDL.h>
#include <algorithm>
#include "tinyxml2.h"
#include "tokenboard.hpp"
#include "tokenbox.hpp"
#include "configurationmanager.hpp"
#include "texturemanager.hpp"
#include "devicemanager.hpp"
#include <utf8.h>
#include "utf8char.hpp"
#include "emoticon.hpp"
#include "fonttexturemanager.hpp"
#include "emoticonmanager.hpp"
#include "section.hpp"
#include "misc.hpp"
#include <functional>

TokenBoard::TokenBoard(int nMaxWidth, bool bShrinkageWidth)
    : m_PW(nMaxWidth)
    , m_W(0)
    , m_H(0)
    , m_Resolution(20)
    , m_ShrinkageWidth(bShrinkageWidth)
    , m_CurrentWidth(0)
    , m_HasEventText(false)
{
	m_CurrentWidth = 0;
}

bool TokenBoard::Load(const tinyxml2::XMLDocument &doc)
{
    const tinyxml2::XMLElement *root = doc.RootElement();
    if (root == nullptr){ return false; }

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
    TOKENBOX stTokenBox;
    stTokenBox.Section = nSection;

    const char *pstart = pCurrentObject->GetText();
    const char *pend   = pstart;

    while(*pend != '\0'){
        pstart = pend;
        utf8::unchecked::advance(pend, 1);
        std::memset(stTokenBox.UTF8CharBox.Data, 0, 8);
        if(pend - pstart == 1 && (*pstart == '\n' || *pstart == '\t' || *pstart == '\r')){
            // continue;
            stTokenBox.UTF8CharBox.Data[0] = ' ';
        }else{
            std::memcpy(stTokenBox.UTF8CharBox.Data, pstart, pend - pstart);
        }
        LoadUTF8CharBoxCache(stTokenBox, 0);
		stTokenBox.UTF8CharBox.Cache.Texture[1] = nullptr;
		stTokenBox.UTF8CharBox.Cache.Texture[2] = nullptr;
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

void TokenBoard::ParseEmoticonObjectAttribute(const tinyxml2::XMLElement *pCurrentObject, int)
{
    m_CurrentSection.Info.Type                 = SECTIONTYPE_EMOTICON;
    m_CurrentSection.State.Emoticon.FrameIndex = 0;
    m_CurrentSection.State.Emoticon.Ticks      = 0;
    m_CurrentSection.Info.Emoticon.Set         = GetEmoticonSet(pCurrentObject);
    m_CurrentSection.Info.Emoticon.Index       = GetEmoticonIndex(pCurrentObject);

    if (m_CurrentSection.Info.Emoticon.Set == 0){
        m_CurrentSection.Info.Emoticon.Index = 0;
    }

    m_CurrentSection.Info.Emoticon.FPS = GetEmoticonManager()->RetrieveEmoticonInfo(
            m_CurrentSection.Info.Emoticon.Set, m_CurrentSection.Info.Emoticon.Index).FPS;
	m_CurrentSection.Info.Emoticon.FrameCount = GetEmoticonManager()->RetrieveEmoticonFrameCount(
		m_CurrentSection.Info.Emoticon.Set, m_CurrentSection.Info.Emoticon.Index);

    m_SectionV.push_back(m_CurrentSection);
}

bool TokenBoard::ParseEmoticonObject(const tinyxml2::XMLElement *pCurrentObject, int nSection)
{
    ParseEmoticonObjectAttribute(pCurrentObject, nSection);
    TOKENBOX stTokenBox;
    stTokenBox.Section = nSection;
	LoadEmoticonCache(stTokenBox, 0);
    AddNewTokenBox(stTokenBox, m_PW);
    return true;
}

bool TokenBoard::ParseEventTextObject(const tinyxml2::XMLElement *pCurrentObject, int nSection)
{
    ParseEventTextObjectAttribute(pCurrentObject, nSection);
    ParseTextContentSection(pCurrentObject, nSection);
    m_HasEventText = true;
    return true;
}

bool TokenBoard::ParseTextObject(const tinyxml2::XMLElement *pCurrentObject, int nSection)
{
    ParseTextObjectAttribute(pCurrentObject, nSection);
    ParseTextContentSection(pCurrentObject, nSection);
    return true;
}

void TokenBoard::UpdateSection(SECTION &stSection, Uint32 ticks)
{
    // only emoticon needs to be updated
    // but we add API here for further
    switch(stSection.Info.Type){
        case SECTIONTYPE_EMOTICON:
            {
                Uint32 nMaxTicks     = (Uint32)std::lround(1000.0 / stSection.Info.Emoticon.FPS);
                Uint32 nCurrentTicks = (Uint32)std::lround(1.0 * ticks + stSection.State.Emoticon.Ticks);
                int    nFrameIndex   = stSection.State.Emoticon.FrameIndex + nCurrentTicks / nMaxTicks;

                stSection.State.Emoticon.FrameIndex = nFrameIndex % stSection.Info.Emoticon.FrameCount;
                stSection.State.Emoticon.Ticks      = nCurrentTicks % nMaxTicks;
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
    //

    // here we use switch, kind of slow but ok
    // since it's only for loading
    // we only need to make as efficient as possible when updating and blitting

    if(m_CurrentWidth + stTokenBox.Cache.W >= nMaxWidth){
        return false;
    }else{
        stTokenBox.State.W1 = 0;
        stTokenBox.State.W2 = 0;
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

void TokenBoard::LoadEmoticonCache(TOKENBOX &stTokenBox, int nFrameIndex)
{
	int nSet   = m_SectionV[stTokenBox.Section].Info.Emoticon.Set;
	int nIndex = m_SectionV[stTokenBox.Section].Info.Emoticon.Index;

    stTokenBox.EmoticonBox.Cache.FrameIndex = 0;
    stTokenBox.EmoticonBox.Cache.FrameInfo  = GetEmoticonManager()->RetrieveEmoticonFrameInfo(nSet, nIndex, nFrameIndex);
	stTokenBox.EmoticonBox.Cache.Texture    = GetEmoticonManager()->RetrieveTexture(nSet, nIndex, nFrameIndex);

	stTokenBox.Cache.W  = GetEmoticonManager()->RetrieveEmoticonInfo(nSet, nIndex).W;
	stTokenBox.Cache.H  = GetEmoticonManager()->RetrieveEmoticonInfo(nSet, nIndex).H;
	stTokenBox.Cache.H1 = GetEmoticonManager()->RetrieveEmoticonInfo(nSet, nIndex).H1;
	stTokenBox.Cache.H2 = GetEmoticonManager()->RetrieveEmoticonInfo(nSet, nIndex).H2;
}

void TokenBoard::LoadUTF8CharBoxCache(TOKENBOX &stTokenBox, int nIndex)
{
    UTF8CHARTEXTUREINDICATOR stIndicator;
    stIndicator.FontInfo = m_SectionV[stTokenBox.Section].Info.Text.FontInfo;
    stIndicator.Color    = m_SectionV[stTokenBox.Section].Info.Text.Color[nIndex % 3];
    std::memcpy(stIndicator.Data, stTokenBox.UTF8CharBox.Data, 8);

    stTokenBox.UTF8CharBox.Cache.Texture[nIndex % 3] = GetFontTextureManager()->RetrieveTexture(stIndicator);
    SDL_QueryTexture(stTokenBox.UTF8CharBox.Cache.Texture[nIndex % 3],
            nullptr, nullptr, &(stTokenBox.Cache.W), &(stTokenBox.Cache.H));

    // don't set StartX/StartY here
    // since this function may be invoked 3 times
	stTokenBox.Cache.H1 = stTokenBox.Cache.H;
	stTokenBox.Cache.H2 = 0;
}

// everything dynamic and can be retrieve is in Cache
//            dynamic but can not be retrieved is in State
//            static is in Info
void TokenBoard::Draw(int nBoardStartX, int nBoardStartY)
{
    for(int nLine = 0; nLine < m_LineV.size(); ++nLine){
        for(auto &stTokenBox: m_LineV[nLine]){
            switch(m_SectionV[stTokenBox.Section].Info.Type){
                case SECTIONTYPE_TEXT:
                case SECTIONTYPE_EVENTTEXT:
                    {
                        int nEvent = m_SectionV[stTokenBox.Section].State.Text.Event;
                        if(stTokenBox.UTF8CharBox.Cache.Texture[nEvent] == nullptr){
                            LoadUTF8CharBoxCache(stTokenBox, nEvent);
                        }
                        SDL_Rect stDstRect = {
                            nBoardStartX + stTokenBox.Cache.StartX,
                            nBoardStartY + stTokenBox.Cache.StartY,
                            stTokenBox.Cache.W,
                            stTokenBox.Cache.H
                        };
                        SDL_RenderCopy(
                                GetDeviceManager()->GetRenderer(),
                                stTokenBox.UTF8CharBox.Cache.Texture[nEvent],
                                nullptr, &stDstRect);

						DrawRectLine(stDstRect);
                        break;
                    }
                case SECTIONTYPE_EMOTICON:
                    {
                        int nFrameIndex = m_SectionV[stTokenBox.Section].State.Emoticon.FrameIndex;
                        if(nFrameIndex != stTokenBox.EmoticonBox.Cache.FrameIndex){
                            LoadEmoticonCache(stTokenBox, nFrameIndex);
                        }
                        int nSrcStartX = stTokenBox.EmoticonBox.Cache.FrameInfo.X;
                        int nSrcStartY = stTokenBox.EmoticonBox.Cache.FrameInfo.Y;
                        int nSrcW      = stTokenBox.EmoticonBox.Cache.FrameInfo.W;
                        int nSrcH      = stTokenBox.EmoticonBox.Cache.FrameInfo.H;
                        int nDstStartX = stTokenBox.Cache.StartX + stTokenBox.EmoticonBox.Cache.FrameInfo.DX;
                        int nDstStartY = stTokenBox.Cache.StartY + stTokenBox.EmoticonBox.Cache.FrameInfo.DY;
                        SDL_Rect stSrcRect = {nSrcStartX, nSrcStartY, nSrcW, nSrcH};
                        SDL_Rect stDstRect = {
                            nDstStartX + nBoardStartX,
                            nDstStartY + nBoardStartY,
                            nSrcW,
                            nSrcH
                        };

                        SDL_RenderCopy(
                                GetDeviceManager()->GetRenderer(),
                                stTokenBox.EmoticonBox.Cache.Texture,
                                &stSrcRect, &stDstRect);
						DrawRectLine(stDstRect);
                        break;
                    }
                default:
                    break;
            }
        }
    }
}

void TokenBoard::DrawRectLine(const SDL_Rect & stRect)
{
    return;
    // for debug
    SDL_SetRenderDrawColor(
            GetDeviceManager()->GetRenderer(), 255, 0, 0, 128);
    SDL_RenderDrawLine(
            GetDeviceManager()->GetRenderer(),
            stRect.x, stRect.y, stRect.x + stRect.w, stRect.y);
    SDL_RenderDrawLine(
            GetDeviceManager()->GetRenderer(),
            stRect.x, stRect.y, stRect.x, stRect.y + stRect.h);
    SDL_RenderDrawLine(
            GetDeviceManager()->GetRenderer(),
            stRect.x + stRect.w, stRect.y, stRect.x + stRect.w, stRect.y + stRect.h);
    SDL_RenderDrawLine(
            GetDeviceManager()->GetRenderer(),
            stRect.x, stRect.y + stRect.h, stRect.x + stRect.w, stRect.y + stRect.h);
    SDL_SetRenderDrawColor(
            GetDeviceManager()->GetRenderer(), 0, 0, 0, 128);
}

void TokenBoard::AddNewTokenBoxLine(bool bEndByReturn)
{
    if(!bEndByReturn){
        SpacePadding(m_PW);
    }

    SetTokenBoxStartX(m_CurrentLine);
    // always align lines to left
    // otherwise we need m_LineStartX
    // but GetNthNewLineStartY() need TOKENBOX::Cache.StartX
    // kind of inconsistent

    m_LineStartY.push_back(GetNthNewLineStartY(m_LineV.size()));
    SetTokenBoxStartY(m_CurrentLine, m_LineStartY.back());
    m_LineV.emplace_back(std::move(m_CurrentLine));

    m_W = (std::max)(m_W, m_CurrentWidth);
    m_H = m_LineStartY.back() + m_CurrentLineMaxH2 + 1;
    ResetCurrentLine();
}


bool TokenBoard::AddNewTokenBox(TOKENBOX &stTokenBox, int nMaxWidth)
    // bool TokenBoard::AddNewTokenBox(const TOKENBOX &stTokenBox, int nMaxWidth)
{
    if (!Add(stTokenBox, nMaxWidth)){
        if (!m_CurrentLine.empty()){
            AddNewTokenBoxLine(false);
            // TODO: here we assume when current line is empty
            // we can always insert a new token box successfully
            // but when the width is toooooooo small this may fail
            return Add(stTokenBox, nMaxWidth);
        }
        else{
            SDL_Log("Token is too large or line width is too small!");
            return false;
        }
    }
    return true;
}



bool TokenBoard::IntervalOverlapped(int startX1, int endX1, int startX2, int endX2)
{
    return !(endX1 < startX2 || endX2 < startX1);
}

void TokenBoard::AddEventHandler(const char *szSectionName, std::function<void()> fnHandler)
{
}

bool TokenBoard::HandleEvent(int nFrameStartX, int nFrameStartY, const SDL_Event &stEvent)
{
    if(!m_HasEventText){
        return false;
    }

    int nEventType, nEventX, nEventY, nEventDX, nEventDY;
    switch(stEvent.type){
        case SDL_MOUSEBUTTONUP:
            nEventType = 1;
            nEventX    = stEvent.button.x;
            nEventY    = stEvent.button.y;
            break;
        case SDL_MOUSEBUTTONDOWN:
            nEventType = 2;
            nEventX    = stEvent.button.x;
            nEventY    = stEvent.button.y;
            break;
        case SDL_MOUSEMOTION:
            nEventType = 1;
            nEventX    = stEvent.motion.x;
            nEventY    = stEvent.motion.y;
            break;
        default:
            return false;
    }

    nEventDX = nEventX - nFrameStartX;
    nEventDY = nEventY - nFrameStartY;

    // pLastTokenBox only store EventText
    static const TOKENBOX *pLastTokenBox = nullptr;
    if(pLastTokenBox){
        // W1 and W2 should also count in
        // otherwise mouse can escape from the flaw of two contiguous tokens
        // means when move mouse horizontally, event text will turn on and off
        int nStartX = pLastTokenBox->Cache.StartX - pLastTokenBox->State.W1;
        // int nStartY = pLastTokenBox->Cache.StartY - pLastTokenBox->Cache.H1;
        int nStartY = pLastTokenBox->Cache.StartY;
        int nW      = pLastTokenBox->Cache.W;
        int nH      = pLastTokenBox->Cache.H;
        if(PointInRect(nEventDX, nEventDY, nStartX, nStartY, nW, nH)){
            m_SectionV[pLastTokenBox->Section].State.Text.Event = nEventType;
            return true;
        }else{
            m_SectionV[pLastTokenBox->Section].State.Text.Event = 0;
        }
    }

    if(PointInRect(nEventX, nEventY, nFrameStartX, nFrameStartY, W(), H())){
        int nGridX = nEventDX / m_Resolution;
        int nGridY = nEventDY / m_Resolution;

        // only put EventText Section in Bitmap
        for(auto pTokenBox: m_TokenBoxBitmap[nGridX][nGridY]){
            int nStartX = pTokenBox->Cache.StartX - pTokenBox->State.W1;
            // int nStartY = pTokenBox->Cache.StartY - pTokenBox->Cache.H1;
            int nStartY = pTokenBox->Cache.StartY;
            int nW      = pTokenBox->Cache.W;
            int nH      = pTokenBox->Cache.H;

            if (PointInRect(nEventX, nEventY, nStartX, nStartY, nW, nH)){
                m_SectionV[pTokenBox->Section].State.Text.Event = nEventType;
                pLastTokenBox = pTokenBox;
                return true;
            }
        }
        // in board but not in any section
        return true;
    }

    // even not in board
    return false;
}

int TokenBoard::TokenBoxType(const TOKENBOX &stTokenBox)
{
    return m_SectionV[stTokenBox.Section].Info.Type;
}

void TokenBoard::MakeEventTextBitmap()
{
    // only make it for EventText since only it accepts event
    int nW = W();
    int nH = H();

    int nBitmapRow = nW / m_Resolution + ((nW % m_Resolution) ? 1 : 0);
    int nBitmapCol = nH / m_Resolution + ((nH % m_Resolution) ? 1 : 0);

    m_TokenBoxBitmap = std::vector<std::vector<std::vector<const TOKENBOX *>>>(
            nBitmapRow, std::vector<std::vector<const TOKENBOX *>>(nBitmapCol, {}));

    for(auto &vTokenLine : m_LineV){
        for(auto &stTokenBox : vTokenLine){
            if(TokenBoxType(stTokenBox) == SECTIONTYPE_EVENTTEXT){
                MarkEventTextBitmap(stTokenBox);
            }
        }
    }
}

void TokenBoard::MarkEventTextBitmap(TOKENBOX &stTokenBox)
{
    int nStartX = stTokenBox.Cache.StartX - stTokenBox.State.W1;
    // int nStartY = stTokenBox.Cache.StartY - stTokenBox.Cache.H1;
    int nStartY = stTokenBox.Cache.StartY;
    int nW      = stTokenBox.Cache.W;
    int nH      = stTokenBox.Cache.H;

    int nStartGridX = nStartX / m_Resolution;
    int nStartGridY = nStartY / m_Resolution;
    int nEndGridX   = (nStartX + nW) / m_Resolution;
    int nEndGridY   = (nStartY + nH) / m_Resolution;
    for(int nX = nStartGridX; nX <= nEndGridX; ++nX){
        for(int nY = nStartGridY; nY <= nEndGridY; ++nY){
            m_TokenBoxBitmap[nX][nY].push_back(&stTokenBox);
        }
    }
}
