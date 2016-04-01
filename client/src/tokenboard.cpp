/*
 * =====================================================================================
 *
 *       Filename: tokenboard.cpp
 *        Created: 06/17/2015 10:24:27 PM
 *  Last Modified: 04/01/2016 01:39:54
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
#include "mathfunc.hpp"
#include "log.hpp"
#include "supwarning.hpp"
#include "colorfunc.hpp"

#include <SDL2/SDL.h>
#include <algorithm>
#include <functional>
#include <tinyxml2.h>
#include <utf8.h>
#include <unordered_map>
#include <string>
#include <cassert>

// XML handle functions
const tinyxml2::XMLElement *TokenBoard::XMLFirstObject(const tinyxml2::XMLElement &rstRoot)
{
    const tinyxml2::XMLElement *pObject = nullptr;

    if(false){
    }else if((pObject = rstRoot.FirstChildElement("object"))){
    }else if((pObject = rstRoot.FirstChildElement("Object"))){
    }else if((pObject = rstRoot.FirstChildElement("OBJECT"))){
    }else{}

    return pObject;
}

const tinyxml2::XMLElement *TokenBoard::XMLNextObject(const tinyxml2::XMLElement &rstObject)
{
    const tinyxml2::XMLElement *pRet = nullptr;

    if(false){
    }else if((pRet = rstObject.NextSiblingElement("OBJECT"))){
    }else if((pRet = rstObject.NextSiblingElement("Object"))){
    }else if((pRet = rstObject.NextSiblingElement("object"))){
    }else{}
    return pRet;
}

int TokenBoard::XMLObjectType(const tinyxml2::XMLElement &rstObject)
{
    if(false
            || (rstObject.Attribute("Type") == nullptr)
            || (rstObject.Attribute("TYPE") == nullptr)
            || (rstObject.Attribute("type") == nullptr)
            || (rstObject.Attribute("TYPE", "PLAINTEXT"))
            || (rstObject.Attribute("TYPE", "PlainText"))
            || (rstObject.Attribute("TYPE", "Plaintext"))
            || (rstObject.Attribute("TYPE", "plainText"))
            || (rstObject.Attribute("TYPE", "plaintext"))
            || (rstObject.Attribute("Type", "PLAINTEXT"))
            || (rstObject.Attribute("Type", "PlainText"))
            || (rstObject.Attribute("Type", "Plaintext"))
            || (rstObject.Attribute("Type", "plainText"))
            || (rstObject.Attribute("Type", "plaintext"))
            || (rstObject.Attribute("type", "PLAINTEXT"))
            || (rstObject.Attribute("type", "PlainText"))
            || (rstObject.Attribute("type", "Plaintext"))
            || (rstObject.Attribute("type", "plainText"))
            || (rstObject.Attribute("type", "plaintext"))){
        return OBJECTTYPE_PLAINTEXT;
    }else if(false
            || (rstObject.Attribute("TYPE", "EVENTTEXT"))
            || (rstObject.Attribute("TYPE", "EventText"))
            || (rstObject.Attribute("TYPE", "Eventtext"))
            || (rstObject.Attribute("TYPE", "eventText"))
            || (rstObject.Attribute("TYPE", "eventtext"))
            || (rstObject.Attribute("Type", "EVENTTEXT"))
            || (rstObject.Attribute("Type", "EventText"))
            || (rstObject.Attribute("Type", "Eventtext"))
            || (rstObject.Attribute("Type", "eventText"))
            || (rstObject.Attribute("Type", "eventtext"))
            || (rstObject.Attribute("type", "EVENTTEXT"))
            || (rstObject.Attribute("type", "EventText"))
            || (rstObject.Attribute("type", "Eventtext"))
            || (rstObject.Attribute("type", "eventText"))
            || (rstObject.Attribute("type", "eventtext"))){
        return OBJECTTYPE_EVENTTEXT;
    }else if(false
            || (rstObject.Attribute("TYPE", "RETURN"))
            || (rstObject.Attribute("TYPE", "Return"))
            || (rstObject.Attribute("TYPE", "return"))
            || (rstObject.Attribute("Type", "RETURN"))
            || (rstObject.Attribute("Type", "Return"))
            || (rstObject.Attribute("Type", "return"))
            || (rstObject.Attribute("type", "RETURN"))
            || (rstObject.Attribute("type", "Return"))
            || (rstObject.Attribute("type", "return"))){
        return OBJECTTYPE_RETURN;
    }else if(false
            || (rstObject.Attribute("TYPE", "Emoticon"))
            || (rstObject.Attribute("TYPE", "emoticon"))
            || (rstObject.Attribute("TYPE", "EMOTICON"))
            || (rstObject.Attribute("Type", "Emoticon"))
            || (rstObject.Attribute("Type", "emoticon"))
            || (rstObject.Attribute("Type", "EMOTICON"))
            || (rstObject.Attribute("type", "Emoticon"))
            || (rstObject.Attribute("type", "emoticon"))
            || (rstObject.Attribute("type", "EMOTICON"))){
        return OBJECTTYPE_EMOTICON;
    }else{
        return OBJECTTYPE_UNKNOWN;
    }
}

// two families of parsing XML
//  1. LoadXML, etc
//  2. InsertXML, etc
//
//  difference is LoadXXX won't assume current board is valid, so
//  it's for init when amount of data comes. InsertXXX assume the
//  current board is valid, it's for edit propose
//
//  for loading, we don't use cursor since we always ``insert" at
//  the end. But for insert we need it

bool TokenBoard::LoadXML(const char *szXML, 
        const std::unordered_map<std::string, std::function<void()>> &rstIDHandleMap)
{
    tinyxml2::XMLDocument stDoc;
    return !stDoc.Parse(szXML) && Load(stDoc, rstIDHandleMap);
}

bool TokenBoard::Load(const tinyxml2::XMLDocument &rstDoc,
        const std::unordered_map<std::string, std::function<void()>> &rstIDHandleMap)
{
    // invalid XML, so we return false
    const tinyxml2::XMLElement *pRoot = rstDoc.RootElement();
    if(pRoot == nullptr){ return false; }

    // 1. clear all
    Clear();

    // 2. add a empty line at the first line
    m_LineV.emplace_back();
    m_EndWithReturn.push_back(false);

    // 3. set the cursor
    m_CursorLoc = {0, 0};

    // 4. call internal insert function to load
    return InnInsert(rstDoc, rstIDHandleMap);
}

// inn function for LoadXXX and InsertXXX
// assumption
//      1. current tokenboard is valid
//      2. cursor is well-prepared
//      3. buffer is prepared, i.e. where start from empty board, there is alreay
//         a empty vector at the end.
bool TokenBoard::InnInsert(const tinyxml2::XMLDocument &rstDoc,
        const std::unordered_map<std::string, std::function<void()>> &rstIDHandleMap)
{
    // invalid XML, so we return false
    const tinyxml2::XMLElement *pRoot = rstDoc.RootElement();
    if(pRoot == nullptr){ return false; }

    // empty object XML, we need to return true
    const tinyxml2::XMLElement *pCurrentObject = XMLFirstObject(*pRoot);

    // put a buffer after to accept coming tokens
    bool bRes = false;
    while(pCurrentObject){
        int nObjectType = XMLObjectType(*pCurrentObject);
        switch(nObjectType){
            case OBJECTTYPE_RETURN:
                {
                    bRes = ParseReturnObject();
                    break;
                }
            case OBJECTTYPE_EMOTICON:
                {
                    bRes = ParseEmoticonObject(*pCurrentObject);
                    break;
                }
            case OBJECTTYPE_EVENTTEXT:
            case OBJECTTYPE_PLAINTEXT:
                {
                    bRes = ParseTextObject(*pCurrentObject, nObjectType, rstIDHandleMap);
                    break;
                }
            default:
                {
                    extern Log *g_Log;
                    g_Log->AddLog(LOGTYPE_INFO, "detected known object type, ignored it");

                    bRes = true;
                    break;
                }
        }

        if(!bRes){ break; }
    }

    // TODO: do I need to check last line?
    return bRes;
}

bool TokenBoard::GetAttributeColor(SDL_Color *pOutColor, const SDL_Color &rstDefaultColor,
        const tinyxml2::XMLElement &rstObject, const std::vector<std::string> &szQueryStringV)
{
    const char *pText = nullptr;
    for(auto &szKey: szQueryStringV){
        if((pText = rstObject.Attribute(szKey.c_str()))){
            break;
        }
    }

    if(pText){
        SDL_Color stColor {0X00, 0X00, 0X00, 0XFF};
        if(false
                || !std::strcmp(pText, "RED")
                || !std::strcmp(pText, "Red")
                || !std::strcmp(pText, "red")){
            stColor.r = 0XFF;
        }else if(false
                || !std::strcmp(pText, "GREEN")
                || !std::strcmp(pText, "Green")
                || !std::strcmp(pText, "green")){
            stColor.g = 0XFF;
        }else if(false
                || !std::strcmp(pText, "BLUE")
                || !std::strcmp(pText, "Blue")
                || !std::strcmp(pText, "blue")){
            stColor.b = 0XFF;
        }else if(false
                || !std::strcmp(pText, "YELLOW")
                || !std::strcmp(pText, "Yellow")
                || !std::strcmp(pText, "yellow")){
            stColor.r = 0XFF;
            stColor.g = 0XFF;
        }else if(false
                || !std::strcmp(pText, "PURPLE")
                || !std::strcmp(pText, "Purple")
                || !std::strcmp(pText, "purple")){
            stColor.r = 0X80;
            stColor.b = 0X80;
        }else{
            // TODO
            // 1. support more color, like pink, purple etc
            // 2. support alpha
            // 3. support color like 0X23FF93AB
        }

        if(pOutColor){ *pOutColor = stColor; }
        return true;
    }else{
        if(pOutColor){ *pOutColor = rstDefaultColor; }
        return false;
    }
}


// get the intergal attribute
// 1. if no error occurs, pOut will be the convert result, return true
// 2. any error, return false and pOut is the default value
bool TokenBoard::GetAttributeAtoi(int *pOut, int nDefaultOut,
        const tinyxml2::XMLElement &rstObject, const std::vector<std::string> &szQueryStringV)
{
    const char *pText = nullptr;
    for(auto &szKey: szQueryStringV){
        if((pText = rstObject.Attribute(szKey.c_str()))){
            break;
        }
    }

    bool bRes = false;
    int  nOut = nDefaultOut;;
    if(pText){
        try{
            nOut = std::stoi(std::string(pText));
            bRes = true;
        }catch(...){
            nOut = nDefaultOut;;
        }
    }

    if(bRes && pOut){ *pOut = nOut; }
    return bRes;
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

bool TokenBoard::ParseEmoticonObject(
        const tinyxml2::XMLElement &rstCurrentObject)
{
    SECTION  stSection;
    TOKENBOX stTokenBox;
    std::memset(&stSection , 0, sizeof(stSection) );
    std::memset(&stTokenBox, 0, sizeof(stTokenBox));

    stSection.Info.Type                 = SECTIONTYPE_EMOTICON;
    stSection.State.Emoticon.FrameIndex = 0;
    stSection.State.Emoticon.MS         = 0.0;

    // won't exam the return value
    GetAttributeAtoi(&(stSection.Info.Emoticon.Set),
            0, rstCurrentObject, {"SET", "Set", "set"});
    GetAttributeAtoi(&(stSection.Info.Emoticon.Index),
            0, rstCurrentObject, {"INDEX", "Index", "index"});

    extern EmoticonDBN *g_EmoticonDBN;
    auto pTexture = g_EmoticonDBN->Retrieve(
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

    int nSectionID = CreateSection(stSection, std::function<void()>());
    if(nSectionID >= 0){
        stTokenBox.Section = nSectionID;
        return AddTokenBoxV({stTokenBox});
    }
    return false;
}

bool TokenBoard::ParseTextObject(
        const tinyxml2::XMLElement &rstCurrentObject, int nObjectType,
        const std::unordered_map<std::string, std::function<void()>> &rstIDHandleMap)
{
    if(nObjectType != OBJECTTYPE_PLAINTEXT || nObjectType != OBJECTTYPE_EVENTTEXT){
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_INFO, "object trying to parse is not text: %d", nObjectType);
        return false;
    }

    SECTION stSection;
    std::memset(&stSection, 0, sizeof(stSection));

    if(nObjectType == OBJECTTYPE_PLAINTEXT){
        stSection.Info.Type = SECTIONTYPE_PLAINTEXT;
    }else{
        stSection.Info.Type = SECTIONTYPE_EVENTTEXT;
    }

    int nTmpFont = 0;
    GetAttributeAtoi(&nTmpFont, 0, rstCurrentObject, {"FONT", "Font", "font"});
    stSection.Info.Text.Font = (uint8_t)nTmpFont;

    // TODO: need to support it
    // GetAttributeAtoi(&(stSection.Info.Text.Style),
    //         0, rstCurrentObject, {"STYLE", "Style", "style"});
    int nTmpSize = 0;
    GetAttributeAtoi(&nTmpSize, 0, rstCurrentObject, {"SIZE", "Size", "size"});
    stSection.Info.Text.Size = (uint8_t)nTmpSize;

    std::function<void()> fnCallback;
    const char *szID = rstCurrentObject.Attribute("ID");

    if(szID){
        // add a handler here
        auto pFuncInst = rstIDHandleMap.find(std::string(szID));
        if(pFuncInst != rstIDHandleMap.end()){
            fnCallback = pFuncInst->second;
        }
    }

    if(nObjectType == OBJECTTYPE_PLAINTEXT){
        GetAttributeColor(stSection.Info.Text.Color,
                {0XFF, 0XFF, 0XFF, 0XFF}, rstCurrentObject, {"COLOR", "Color", "color"});
    }else{
        GetAttributeColor(stSection.Info.Text.Color + 0,
                {0XFF, 0XFF, 0X00, 0XFF}, rstCurrentObject, {"OFF", "Off", "off"});
        GetAttributeColor(stSection.Info.Text.Color + 1,
                {0X00, 0XFF, 0X00, 0XFF}, rstCurrentObject, {"OVER", "Over", "over"});
        GetAttributeColor(stSection.Info.Text.Color + 2,
                {0XFF, 0X00, 0X00, 0XFF}, rstCurrentObject, {"DOWN", "Down", "down"});
    }

    stSection.State.Text.Event = 0;

    int nSectionID = CreateSection(stSection, fnCallback);
    if(nSectionID < 0){ return false; }

    // then we parse event text content
    const char *pStart = rstCurrentObject.GetText();
    const char *pEnd   = pStart;

    // when get inside this funciton
    // the section structure has been well-prepared
    //
    int nDefaultSize = (int)m_SectionV[nSectionID].Info.Text.Size;

    uint32_t nFontAttrKey = 0
        + (((uint32_t)m_SectionV[nSectionID].Info.Text.Font)  << 16)
        + (((uint32_t)m_SectionV[nSectionID].Info.Text.Size)  <<  8)
        + (((uint32_t)m_SectionV[nSectionID].Info.Text.Style) <<  0);

    std::vector<TOKENBOX> stTBV;
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

        stTokenBox.Section = nSectionID;
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

        stTBV.push_back(stTokenBox);
    }

    return AddTokenBoxV(stTBV);
}

void TokenBoard::Update(double fMS)
{
    if(fMS < 0.0 || m_SkipUpdate){ return; }

    for(auto &rstInst: m_SectionV){
        auto &rstSection = rstInst.second;
        switch(rstSection.Info.Type){
            case SECTIONTYPE_EMOTICON:
                {
                    // 1.
                    double fUnitMS     = 1000.0 / rstSection.Info.Emoticon.FPS;
                    double fCurrentMS  = fMS + rstSection.State.Emoticon.MS;
                    int    nRawIndex   = (int)std::lround(fCurrentMS / fUnitMS);
                    int    nFrameCount = rstSection.Info.Emoticon.FrameCount;

                    // 2.
                    rstSection.State.Emoticon.FrameIndex = nRawIndex % nFrameCount;
                    rstSection.State.Emoticon.MS         = fCurrentMS;

                    // 3. try to control for the precision
                    if(rstSection.State.Emoticon.MS > 2000.0 * fUnitMS * nFrameCount){
                        rstSection.State.Emoticon.MS -= (1000.0 * fUnitMS * nFrameCount);
                    }

                    break;
                }
            default:
                break;
        }
    }
}

int TokenBoard::SectionTypeCount(int nLine, int nSectionType)
{
    if(nLine < 0 || nLine >= (int)m_LineV.size()){ return -1; }

    auto fnCmp = [](int nType, int nArgType) -> bool {
        switch(nArgType){
            case SECTIONTYPE_EMOTICON:
            case SECTIONTYPE_PLAINTEXT:
            case SECTIONTYPE_EVENTTEXT:
                {
                    return nType == nArgType;
                }
            case SECTIONTYPE_TEXT:
                {
                    return nType == SECTIONTYPE_PLAINTEXT || nType == SECTIONTYPE_EVENTTEXT;
                }
            case SECTIONTYPE_ALL:
                {
                    return true;
                }
            default:
                {
                    return false;
                }
        }
    };

    int nCount = 0;
    if(nSectionType == SECTIONTYPE_ALL){
        return m_LineV[nLine].size();
    }

    for(const auto &rstTokenBox: m_LineV[nLine]){
        auto p = m_SectionV.find(rstTokenBox.Section);
        if(p != m_SectionV.end()){
            if(fnCmp(p->second.Info.Type, nSectionType)){
                nCount++;
            }
        }else{
            // oooops
            extern Log *g_Log;
            g_Log->AddLog(LOGTYPE_INFO, "section id can't be find");
        }
    }
    return nCount;
}

// padding to set the current line width to be m_PW
//  1. W1/W2 is prepared for m_WordSpace only, we need to increase it
//  2. only do padding, won't calculate StartX/Y for simplity
//
//      ++----------+------------+
//      |+        + |
//      ||        | |
//      |+        + |
//      +-----------+------------+
//
//  nDWidth      : width should be made by space
//  nSectionType : specify SectionType for padding
//
// return should always be non-negative
// meaning we can't be out of the boundary
//
// I even forget  logics in this funciton
// update it when error occurs
int TokenBoard::DoLinePadding(int nLine, int nDWidth, int nSectionType)
{
    if(nLine < 0 || nLine > (int)m_LineV.size()){ return -1; }
    if(nDWidth < 0){ return -1; }

    int nCount = SectionTypeCount(nLine, nSectionType);
    if(false
            || m_LineV[nLine].size() < 2
            || nCount == 0){
        // can't padding
        return nDWidth;
    }

    int nPaddingSpace = nDWidth / nCount;

    // first round
    // first token may reserve for no padding
    //
    // WTF
    // I forget the reason why I need to check [0].type != [1].type
    if(TokenBoxType(m_CurrentLine[0]) != TokenBoxType(m_CurrentLine[1])){
        if(nSectionType == 0 || TokenBoxType(m_CurrentLine[0]) & nSectionType){
            m_CurrentLine[0].State.W2 += (nPaddingSpace / 2);
            nDWidth -= nPaddingSpace / 2;
            if(nDWidth == 0){ return 0; }
        }
    }

    for(size_t nIndex = 1; nIndex < m_CurrentLine.size() - 1; ++nIndex){
        if(nSectionType == 0 || TokenBoxType(m_CurrentLine[nIndex]) & nSectionType){
            m_CurrentLine[nIndex].State.W1 += (nPaddingSpace / 2);
            m_CurrentLine[nIndex].State.W2 += (nPaddingSpace - nPaddingSpace / 2);

            nDWidth -= nPaddingSpace;
            if(nDWidth == 0){ return 0; }
        }
    }

    // why here I won't check type? forget
    if(nSectionType == 0 || TokenBoxType(m_CurrentLine.back()) & nSectionType){
        m_CurrentLine.back().State.W1 += (nPaddingSpace / 2);
        nDWidth -= nPaddingSpace / 2;
        if(nDWidth == 0){ return 0; }
    }

    // second round, small update
    if(TokenBoxType(m_CurrentLine[0]) != TokenBoxType(m_CurrentLine[1])){
        if(nSectionType == 0 || TokenBoxType(m_CurrentLine[0]) & nSectionType){
            m_CurrentLine[0].State.W2++;;
            nDWidth--;
            if(nDWidth == 0){ return 0; }
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
            if(nDWidth == 0){ return 0; }
        }
    }

    if(nSectionType == 0 || TokenBoxType(m_CurrentLine.back()) & nSectionType){
        m_CurrentLine.back().State.W1++;
        nDWidth--;
        if(nDWidth == 0){ return 0; }
    }

    return nDWidth;
}

void TokenBoard::SetTokenBoxStartY(int nLine, int nBaseLineY)
{
    if(nLine < 0 || nLine >= (int)m_LineV.size()){ return; }
    for(auto &rstTokenBox: m_LineV[nLine]){
        rstTokenBox.Cache.StartY = nBaseLineY - rstTokenBox.Cache.H1;
    }
}

void TokenBoard::SetTokenBoxStartX(int nLine)
{
    if(nLine < 0 || nLine >= (int)m_LineV.size()){ return; }

    int nCurrentX = m_Margin[3];
    for(auto &rstTokenBox: m_LineV[nLine]){
        nCurrentX += rstTokenBox.State.W1;
        rstTokenBox.Cache.StartX = nCurrentX;
        nCurrentX += rstTokenBox.Cache.W;
        nCurrentX += rstTokenBox.State.W2;
    }
}

int TokenBoard::LineFullWidth(int nLine)
{
    if(nLine < 0 || nLine >= (int)m_LineV.size()){ return -1; }

    int nWidth = 0;
    for(auto &rstTB: m_LineV[nLine]){
        nWidth += (rstTB.Cache.W + rstTB.State.W1 + rstTB.State.W2);
    }
    return nWidth;
}

int TokenBoard::LineRawWidth(int nLine, bool bWithWordSpace)
{
    if(nLine < 0 || nLine >= (int)m_LineV.size()){ return -1; }

    switch(m_LineV[nLine].size()){
        case 0:
            {
                // empty line, can only happen at the last line
                return 0;
            }
        case 1:
            {
                // only one token, so word space is not counted
                return m_LineV[nLine][0].Cache.W;
            }
        default:
            {
                int nWidth = 0;
                for(auto &rstTB: m_LineV[nLine]){
                    nWidth += rstTB.Cache.W;
                }

                if(bWithWordSpace){
                    nWidth += m_WordSpace * ((int)m_LineV[nLine].size() - 1);
                }
                return nWidth;
            }
    }
}

int TokenBoard::SetTokenBoxWordSpace(int nLine)
{
    if(nLine < 0 || nLine >= (int)m_LineV.size()){ return -1; }

    int nW1 = m_WordSpace / 2;
    int nW2 = m_WordSpace - nW1;

    for(auto &rstTB: m_LineV[nLine]){
        rstTB.State.W1 = nW1;
        rstTB.State.W2 = nW2;
    }

    if(!m_LineV[nLine].empty()){
        m_LineV[nLine][0].State.W1     = 0;
        m_LineV[nLine].back().State.W2 = 0;
    }

    return LineRawWidth(nLine, true);
}


// padding with space to make the line be exactly of width m_PW
// for input:
//      1. all tokens in current line has W specified
//      2. W1/W2 is not-inited
// output:
//      1. W1/W2 inited
//      2. return real width after operation of this function
int TokenBoard::LinePadding(int nLine)
{
    if(nLine < 0 || nLine >= (int)m_LineV.size()){ return -1; }

    int nWidth = SetTokenBoxWordSpace(nLine);
    if(nWidth < 0){ return -1; }

    // if we don't need space padding, that's all
    if(m_PW <= 0){ return nWidth; }

    // we need word space padding and extra padding
    int nDWidth = m_PW - nWidth;

    if(nDWidth > 0){
        // round-1: try to padding by emoticons
        int nNewWidth = DoLinePadding(nLine, nDWidth, SECTIONTYPE_EMOTICON);
        if(nNewWidth < 0 || nNewWidth == nDWidth){
            // doesn't work, padding by all boxes
            nNewWidth = DoLinePadding(nLine, nDWidth, 0);
            if(nNewWidth == 0){
                return m_PW;
            }
            return -1;
        }
    }
    return nWidth;
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
    //  but token in (n + 1)-th line only count for the interval,
    //  W1/W2 won't count for here, as I denoted

    int nMaxH2 = -1;
    for(auto &rstTokenBox: m_LineV[nthLine]){
        int nX  = rstTokenBox.Cache.StartX;
        int nW  = rstTokenBox.Cache.W;
        int nH2 = rstTokenBox.Cache.H2;
        int nW1 = rstTokenBox.State.W1;
        int nW2 = rstTokenBox.State.W2;

        if(IntervalOverlap(nX, nW1 + nW + nW2, nIntervalStartX, nIntervalWidth)){
            nMaxH2 = (std::max)(nMaxH2, nH2);
        }
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

// get StartY of current Line
// assume:
//      1. 0 ~ (nthLine - 1) are well-prepared with StartX, StartY, W/W1/W2/H1/H2 etc
//      2. nthLine is padded already, StartX, W/W1/W2 are OK now
int TokenBoard::GetNthNewLineStartY(int nLine)
{
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
    // for nthLine we use W only, but for nthLine - 1, we use W1 + W + W2
    // reason is clear since W1/2 of nthLine won't show, then use them are
    // wasteful for space
    //
    //          +---+-----+--+---+-------+
    //          |   |     |  |   |       |
    //          | 1 |  W  |2 | 1 |   W   |...
    //          |   |     |  |   |       |
    //          +---+-----+--+---+-------+
    //
    //                   +--------+
    //                   |        |
    //                   |   W    |
    //                   |        |
    //

    if(nLine < 0 || nLine >= (int)m_LineV.size()){ return -1; }

    int nCurrentY = -1;
    for(auto &rstTokenBox: m_LineV[nLine]){
        int nX  = rstTokenBox.Cache.StartX;
        int nW  = rstTokenBox.Cache.W;
        int nH1 = rstTokenBox.Cache.H1;

        nCurrentY = (std::max)(nCurrentY, GetNthLineTokenBoxStartY(nLine, nX, nW, nH1));
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

    // we need to check it since for each tokenbox, overlap checking is expensive
    bool bCheckInside = !(nSrcX == 0 && nSrcY == 0 && nSrcW == W() && nSrcH == H());

    for(int nLine = 0; nLine < (int)m_LineV.size(); ++nLine){
        for(auto &rstTokenBox: m_LineV[nLine]){

            // get tokenbox size info, this is uniform for Text/Emoticon
            int nX, nY, nW, nH;
            nX = rstTokenBox.Cache.StartX;
            nY = rstTokenBox.Cache.StartY;
            nW = rstTokenBox.Cache.W - rstTokenBox.State.W1 - rstTokenBox.State.W2;
            nH = rstTokenBox.Cache.H;

            bool bCheckOverlap = bCheckInside && !RectangleInside(0, 0, W(), H(), nX, nY, nW, nH);

            if(bCheckOverlap && !RectangleOverlapRegion(
                        nSrcX, nSrcY, nSrcW, nSrcH, &nX, &nY, &nW, &nH)){
                // need to check overlap and it's outside
                continue;
            }

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

                        int nEvent = m_SectionV[rstTokenBox.Section].State.Text.Event;
                        auto &rstColor = m_SectionV[nEvent].Info.Text.Color[nEvent];

                        extern SDLDevice *g_SDLDevice;
                        extern FontexDBN *g_FontexDBN;
                        auto pTexture = g_FontexDBN->Retrieve(rstTokenBox.UTF8CharBox.Cache.Key);

                        if(pTexture){
                            SDL_SetTextureColorMod(pTexture, rstColor.r, rstColor.g, rstColor.b);
                            int nDX = nX - rstTokenBox.Cache.StartX;
                            int nDY = nY - rstTokenBox.Cache.StartY;
                            g_SDLDevice->DrawTexture(pTexture,
                                    nX + nDstX, nY + nDstY, nDX, nDY, nW, nH);
                        }else{
                            // TODO
                            // draw a box here to indicate errors
                        }

                        break;
                    }

                case SECTIONTYPE_EMOTICON:
                    {
                        int nFrameIndex = m_SectionV[rstTokenBox.Section].State.Emoticon.FrameIndex;
                        extern EmoticonDBN *g_EmoticonDBN;
                        extern SDLDevice   *g_SDLDevice;

                        int nXOnTex, nYOnTex;
                        auto pTexture = g_EmoticonDBN->Retrieve(
                                rstTokenBox.EmoticonBox.Cache.Key + nFrameIndex,
                                &nXOnTex, &nYOnTex,
                                nullptr, nullptr, nullptr, nullptr, nullptr);

                        if(pTexture){
                            int nDX = nX - rstTokenBox.Cache.StartX;
                            int nDY = nY - rstTokenBox.Cache.StartY;
                            g_SDLDevice->DrawTexture(pTexture,
                                    nX + nDstX, nY + nDstY, nXOnTex + nDX, nYOnTex + nDY, nW, nH);
                        }else{
                            // TODO
                            // draw a box to indicate errors
                        }

                        break;
                    }
                default:
                    break;
            }
        }
    }
}

// calculate all need cache for nLine
//
// TBD & TODO & Assume:
//      1. all needed tokens are already in current line
//
//      2. m_EndWithReturn[nLine] specified
//
//      3. all other lines (except nLine) are well-prepared
//         so we can call GetNthNewLineStartY() safely
//
//      4. helper containers such as m_LineStartY are prepared for space
//         the value of it may be invalid
//
void TokenBoard::ResetLine(int nLine)
{
    if(nLine < 0 || nLine >= (int)m_LineV.size()){ return; }

    if(!m_EndWithReturn[nLine]){
        LinePadding(nLine);
    }

    SetTokenBoxStartX(nLine);

    int nWidth = LineFullWidth(nLine);
    if(m_W < nWidth){
        m_W = nWidth;
    }else{
        // the current width may get from nLine
        // but nLine resets now, we need to recompute it
        m_W = -1;
        for(int nIndex = 0; nIndex < (int)m_LineV.size(); ++nIndex){
            m_W = (std::max)(m_W, LineFullWidth(nLine));
        }
    }

    // without StartX we can't calculate StartY
    //      1. for Loading function, this will only be one round
    //      2. for Insert function, this execute many times to
    //    re-calculate the StartY.
    //    
    // howto
    // 1. reset nLine, anyway this is needed
    m_LineStartY[nLine] = GetNthNewLineStartY(nLine);
    SetTokenBoxStartY(nLine, m_LineStartY[nLine]);

    // 2. reset rest lines, use a trick here
    //      if last line is full length, we only need to add a delta form now
    //      otherwise we continue to compute all StartY
    //
    int nRestLine  = nLine + 1;
    int nTrickOn   = 0;
    int nDStartY   = 0;
    // TODO
    // think about if m_PW < 0 can I save some thing
    while(nRestLine < (int)m_LineV.size()){
        if(nTrickOn){
            m_LineStartY[nRestLine] += nDStartY;
        }else{
            int nOldStartY = m_LineStartY[nRestLine];
            m_LineStartY[nRestLine] = GetNthNewLineStartY(nRestLine);
            if(LineFullWidth(nRestLine) == m_W){
                nTrickOn = 1;
                nDStartY = m_LineStartY[nRestLine] - nOldStartY;
            }
        }
        SetTokenBoxStartY(nRestLine, m_LineStartY[nRestLine]);
    }

    m_H = m_LineStartY.back() + GetNthLineIntervalMaxH2(m_LineV.size() - 1, 0, m_W) + 1;
}

void TokenBoard::TokenBoxGetMouseButtonUp(int nX, int nY, bool bFirstHalf)
{
    // 1. invalid position
    if(TokenBoxValid(nX, nY)){ return; }

    // 2. invalid section id
    int nSection = m_LineV[nY][nX].Section;
    auto pSectionInst = m_SectionV.find(nSection);
    if(pSectionInst == m_SectionV.end()){
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_INFO, "section id can't find: %d", nSection);
        return;
    }

    auto &rstSection = pSectionInst->second;

    switch(rstSection.Info.Type){
        case SECTIONTYPE_PLAINTEXT:
        case SECTIONTYPE_EMOTICON:
            {
                // for plain text and emoticon, we only need to think about selecting
                // and it can't accept other kind of events
                if(m_SelectState == 1){
                    // 1. stop the dragging
                    m_SelectState = 2;
                    // 2. record the end offset of dragging
                    m_SelectLoc[1].first  = nX - (bFirstHalf ? 1 : 0);
                    m_SelectLoc[1].second = nY;
                }else{
                    // this should be click, need to do nothing
                }
                break;
            }
        case SECTIONTYPE_EVENTTEXT:
            {
                // 1. may trigger event
                // 2. may be the end of dragging
                switch(rstSection.State.Text.Event){
                    case 0:
                        {
                            // impossible generally, only possibility comes with
                            // user are moving mouse *very* fast. Then pointer can
                            // jump inside one tokenbox and emit button up event.
                            //
                            // do nothing
                            break;
                        }
                    case 1:
                        {
                            // over state and then get release, this only can be dragging
                            if(m_SelectState == 1){
                                // 1. stop the dragging
                                m_SelectState = 2;
                                // 2. record the end offset of dragging
                                m_SelectLoc[1].first  = nX - (bFirstHalf ? 1 : 0);
                                m_SelectLoc[1].second = nY;
                            }else{
                                // impossible
                            }
                            break;
                        }
                    case 2:
                        {
                            // pressed state, and get released event, need tirgger
                            // 1. make as state ``over"
                            rstSection.State.Text.Event = 1;
                            // 2. trigger registered event handler
                            auto pFuncInst = m_IDFuncV.find(nSection);
                            if(pFuncInst != m_IDFuncV.end() && pFuncInst->second){
                                (pFuncInst->second)();
                            }
                            break;
                        }
                    default:
                        {
                            break;
                        }

                }

                break;
            }
    }
}

void TokenBoard::TokenBoxGetMouseButtonDown(int nX, int nY, bool bFirstHalf)
{
    // 1. invalid position
    if(TokenBoxValid(nX, nY)){ return; }

    // 2. invalid section id
    int nSection = m_LineV[nY][nX].Section;
    auto pSectionInst = m_SectionV.find(nSection);
    if(pSectionInst == m_SectionV.end()){
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_INFO, "section id can't find: %d", nSection);
        return;
    }

    auto &rstSection = pSectionInst->second;
    switch(rstSection.Info.Type){
        case SECTIONTYPE_PLAINTEXT:
        case SECTIONTYPE_EMOTICON:
            {
                // 1. always drop the selected result off
                m_SelectState = 0;
                std::pair<int, int> stCursorLoc = { nY, nX - (bFirstHalf ? 1 : 0) };

                // 2. mark for the preparing of next selection
                if(m_Selectable){ m_SelectLoc[0]  = stCursorLoc; }
                if(m_WithCursor){ m_CursorLoc     = stCursorLoc; }

                break;
            }
        case SECTIONTYPE_EVENTTEXT:
            {
                // set to be pressed directly
                rstSection.State.Text.Event = 2;
                break;
            }
    }
}

void TokenBoard::TokenBoxGetMouseMotion(int nX, int nY, bool bFirstHalf)
{
    if(m_Selectable){
        m_SelectState  = 1;
        m_SelectLoc[1] = {nY, nX - (bFirstHalf ? 1 : 0)};
    }
}

void TokenBoard::ProcessEventMouseButtonUp(int nEventDX, int nEventDY)
{

    if(!In(nEventDX, nEventDY)){ return; }

    if(LastTokenBoxValid()){
        auto &rstLastTB = m_LineV[m_LastTokenBoxLoc.second][m_LastTokenBoxLoc.first];
        // W1 and W2 should also count in
        // otherwise mouse can escape from the flaw of two contiguous tokens
        // means when move mouse horizontally, event text will turn on and off
        //
        int nX = rstLastTB.Cache.StartX - rstLastTB.State.W1;
        int nY = rstLastTB.Cache.StartY;
        int nW = rstLastTB.Cache.W;
        int nH = rstLastTB.Cache.H;

        if(PointInRectangle(nEventDX, nEventDY, nX, nY, nW, nH)){
            TokenBoxGetMouseButtonUp(
                    m_LastTokenBoxLoc.first, m_LastTokenBoxLoc.second, nEventDX < nX + nW / 2 );
        }
    }else{
        int nGridX = nEventDX / m_Resolution;
        int nGridY = nEventDY / m_Resolution;

        // only put EventText Section in Bitmap
        // emoticon won't accept events
        //
        for(const auto &stLoc: m_TokenBoxBitmap[nGridX][nGridY]){
            //
            // for each possible tokenbox in the grid
            //
            // this is enough since cover of all tokenboxs in one gird
            // is much larger than the grid itself
            if(!TokenBoxValid(stLoc.first, stLoc.second)){ continue; }

            const auto &rstTokenBox = m_LineV[stLoc.second][stLoc.first];

            int nStartX = rstTokenBox.Cache.StartX - rstTokenBox.State.W1;
            int nStartY = rstTokenBox.Cache.StartY;
            int nW      = rstTokenBox.Cache.W;
            int nH      = rstTokenBox.Cache.H;

            if(PointInRectangle(nEventDX, nEventDY, nStartX, nStartY, nW, nH)){
                TokenBoxGetMouseButtonUp(stLoc.first, stLoc.second, nEventDX < nStartX + nW / 2);
            }
        }
    }
}

bool TokenBoard::ProcessEvent(const SDL_Event &rstEvent, bool *bValid)
{
    // don't need to handle event or event has been consumed
    if(m_SkipEvent || (bValid && !(*bValid))){ return false; }

    int nEventDX, nEventDY;
    switch(rstEvent.type){
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
            {
                nEventDX = rstEvent.button.x - X();
                nEventDY = rstEvent.button.y - Y();
                break;
            }
        case SDL_MOUSEMOTION:
            {
                nEventDX = rstEvent.motion.x - X();
                nEventDY = rstEvent.motion.y - Y();
                break;
            }
        default:
            {
                // TBD
                // for all other event, won't handle, just return
                // maybe key up/down event maybe handled in the future
                return false;
            }
    }

    // now we get event type and event location
    //
    // 1. inside the board or not
    if(!In(nEventDX, nEventDY)){ return false; }

    auto fnTokenBoxEventHandle = [this](uint32_t nEventType, int nLocX, int nLocY, bool bFirstHalf){
        switch(nEventType){
            case SDL_MOUSEMOTION:
                TokenBoxGetMouseMotion(nLocX, nLocY, bFirstHalf);
                break;
            case SDL_MOUSEBUTTONUP:
                TokenBoxGetMouseButtonUp(nLocX, nLocY, bFirstHalf);
                break;
            case SDL_MOUSEBUTTONDOWN:
                TokenBoxGetMouseButtonDown(nLocX, nLocY, bFirstHalf);
                break;
            default:
                break;
        }
    };

    // 2. try among all boxes
    int  nTBLocX    = -1;
    int  nTBLocY    = -1;
    bool bFirstHalf = false;

    if(LastTokenBoxValid()){
        auto &rstLastTB = m_LineV[m_LastTokenBoxLoc.second][m_LastTokenBoxLoc.first];
        // W1 and W2 should also count in
        // otherwise mouse can escape from the flaw of two contiguous tokens
        // means when move mouse horizontally, event text will turn on and off
        //
        int nX = rstLastTB.Cache.StartX - rstLastTB.State.W1;
        int nY = rstLastTB.Cache.StartY;
        int nW = rstLastTB.Cache.W;
        int nH = rstLastTB.Cache.H;

        if(PointInRectangle(nEventDX, nEventDY, nX, nY, nW, nH)){
            nTBLocX    = m_LastTokenBoxLoc.first;
            nTBLocY    = m_LastTokenBoxLoc.second;
            bFirstHalf = (nEventDX < nX + nW / 2);
        }
    }else{
        int nGridX = nEventDX / m_Resolution;
        int nGridY = nEventDY / m_Resolution;

        // only put EventText Section in Bitmap
        // emoticon won't accept events
        //
        for(const auto &stLoc: m_TokenBoxBitmap[nGridX][nGridY]){
            //
            // for each possible tokenbox in the grid
            //
            // this is enough since cover of all tokenboxs in one gird
            // is much larger than the grid itself
            if(TokenBoxValid(stLoc.first, stLoc.second)){
                // we may need to re-build event bitmap
                const auto &rstTokenBox = m_LineV[stLoc.second][stLoc.first];

                int nX = rstTokenBox.Cache.StartX - rstTokenBox.State.W1;
                int nY = rstTokenBox.Cache.StartY;
                int nW = rstTokenBox.Cache.W;
                int nH = rstTokenBox.Cache.H;

                if(PointInRectangle(nEventDX, nEventDY, nX, nY, nW, nH)){
                    nTBLocX    = stLoc.first;
                    nTBLocY    = stLoc.second;
                    bFirstHalf = (nEventDX < nX + nW / 2);
                    break;
                }
            }
        }
    }

    if(TokenBoxValid(nTBLocX, nTBLocY)){
        // we do get some tokenbox
        fnTokenBoxEventHandle(rstEvent.type, nTBLocX, nTBLocY, bFirstHalf);
        if(bValid){ *bValid = false; }
        return true;
    }

    // whether we need to handle these events not in any boxes
    bool bNeedOutTB = false;
    if(m_WithCursor || (m_Selectable && m_SelectState == 1)){
        bNeedOutTB = true;
    }

    // then no box can handle this event, need to check line by line
    if(bNeedOutTB){
        // consume this event anyway
        if(bValid){ *bValid = false; }

        // 1. decide which line it's inside
        int nRowIn = -1;
        if(!m_LineStartY.empty() && nEventDY > m_LineStartY.back()){
            nRowIn = (int)m_LineStartY.size() - 1;
        }else{
            for(int nRow = 0; nRow < (int)m_LineStartY.size(); ++nRow){
                if(m_LineStartY[nRow] >= nEventDY){
                    // this may cause a problem:
                    //
                    //
                    // +-----------+ +----+
                    // |           | |    |
                    // +-----------+ |    | --------------- Y
                    //              *|    |
                    //     +-------+ |    |
                    //     |       | +----+
                    //     |       | +--+
                    //     |       | |  |
                    //     +-------+ +--+   --------------- Y
                    //
                    // now if click is at "*", it will give cursor at the second
                    // line, not the first line

                    nRowIn = nRow;
                    break;
                }
            }
        }

        // can't capture this event, most-likely error occurs
        if(nRowIn < 0){ return false; }

        // now try to find nTBLocX for tokenbox binding
        //
        int nTokenBoxBind = -1;
        int nLastCenterX  = -1;
        for(int nXCnt = 0; nXCnt < (int)m_LineV[nRowIn].size(); ++nXCnt){
            int nX  = m_LineV[nRowIn][nXCnt].Cache.StartX;
            int nW  = m_LineV[nRowIn][nXCnt].Cache.W;
            int nCX = nX + nW / 2;

            if(PointInSegment(nEventDX, nLastCenterX, nCX - nLastCenterX + 1)){
                nTokenBoxBind = nXCnt;
                break;
            }

            nLastCenterX = nCX;
        }

        if(nTokenBoxBind < 0){
            // if we can't bind, bind it to the last box
            // this may cause problem:
            //
            // +-----------+ +----+
            // |           | |    |
            // +-----------+ |    | --------------- Y
            //               |    |
            //     +-------+ |    |
            //     |       | |    |
            //     |       | |    |
            //     |       | |    |
            //     |       | |    | *
            //     |       | +----+
            //     +-------+ ---------------------- Y
            //
            // when clicking at "*", we get cursor bind
            // at the second line
            //
            nTokenBoxBind = m_LineV[nRowIn].size();
        }

        fnTokenBoxEventHandle(rstEvent.type, nTokenBoxBind, nRowIn, false);
        return false;
    }

    return true;
}

int TokenBoard::TokenBoxType(const TOKENBOX &rstTokenBox)
{
    return m_SectionV[rstTokenBox.Section].Info.Type;
}

void TokenBoard::MakeTokenBoxEventBitmap()
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

    for(int nY = 0; nY < (int)m_LineV.size(); ++nY){
        for(int nX = 0; nX < (int)m_LineV[nY].size(); ++nX){
            MarkTokenBoxEventBitmap(nX, nY);
        }
    }
}

void TokenBoard::MarkTokenBoxEventBitmap(int nLocX, int nLocY)
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
    if(!TokenBoxValid(nLocX, nLocY)){ return; }

    auto &rstTokenBox = m_LineV[nLocY][nLocX];

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
            m_TokenBoxBitmap[nX][nY].emplace_back(nX, nY);
        }
    }
}

int TokenBoard::GuessResoltion()
{
    // TODO
    // make this function more reasonable and functional
    return m_DefaultSize / 2;
}

bool TokenBoard::ParseXML(const char *szText)
{
    UNUSED(szText);
    return true;
}

std::string TokenBoard::GetXML(bool bSelectedOnly)
{
    if(bSelectedOnly){
        if(m_SelectState == 1 || m_SelectState == 2){
            // selecting or selected
            return InnGetXML(m_SelectLoc[0].first,
                    m_SelectLoc[0].second, m_SelectLoc[1].first, m_SelectLoc[1].second);
        }else{
            return "<root></root>";
        }
    }else{
        return InnGetXML(0, 0, m_LineV.size() - 1, m_LineV.back().size() - 1);
    }
}

std::string TokenBoard::InnGetXML(int nX0, int nY0, int nX1, int nY1)
{
    std::string szXML;
    if((nY0 > nY1) || (nY0 == nY1 && nX0 > nX1)){ return szXML; }

    szXML = "<root>";
    int nX = nX0;
    int nY = nY0;

    int nLastSection = -1;
    while(!(nX == nX1 && nY == nY1)){
        const auto &rstTB = m_LineV[nY][nX];
        const auto &rstSN = m_SectionV[rstTB.Section];

        if(nLastSection >= 0){
            szXML += "</object>";
        }

        switch(rstSN.Info.Type){
            case SECTIONTYPE_EMOTICON:
                {
                    szXML += "<object type=emoticon set=";
                    szXML += std::to_string(rstSN.Info.Emoticon.Set);
                    szXML += " index=";
                    szXML += std::to_string(rstSN.Info.Emoticon.Index);
                    szXML += ">";
                    break;
                }
            case SECTIONTYPE_EVENTTEXT:
                {
                    if(rstTB.Section != nLastSection){
                        // TODO & TBD
                        // 1. we won't parse ID here
                        // 2. didn't parse style yet
                        // 3. didn't parse color set yet
                        //
                        szXML += "<object type=eventtext font=";
                        szXML += std::to_string(rstSN.Info.Text.Font);
                        szXML += " size=";
                        szXML += std::to_string(rstSN.Info.Text.Size);
                        szXML += ">";
                    }
                    // truncate to get last 32 bits
                    uint32_t nUTF8Code = rstTB.UTF8CharBox.Cache.Key;
                    szXML += (const char *)(&nUTF8Code);
                    break;
                }
            case SECTIONTYPE_PLAINTEXT:
                {
                    if(rstTB.Section != nLastSection){
                        szXML += "<object type=plaintext font=";
                        szXML += std::to_string(rstSN.Info.Text.Font);
                        szXML += " size=";
                        szXML += std::to_string(rstSN.Info.Text.Size);

                        char szColor[16];
                        std::sprintf(szColor, "0x%08x", Color2U32ARGB(rstSN.Info.Text.Color[0]));
                        szXML += " color=";
                        szXML += szColor;
                        szXML += ">";
                    }
                    // truncate to get last 32 bits
                    uint32_t nUTF8Code = rstTB.UTF8CharBox.Cache.Key;
                    szXML += (const char *)(&nUTF8Code);
                    break;
                }
            default:
                {
                    break;
                }
        }

        nLastSection = rstTB.Section;

        nX++;
        if(nX == (int)m_LineV[nY].size()){
            nX = 0;
            nY++;
        }
    }
}

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

// Insert a bunch of tokenboxes at any position (nX, nY) inside the text
// block, this function won't introduce any ``return"
//
// Assume:
//      1. the tokenboard is well-prepared
//      2. we won't allow empty line since don't know how to set height
//      3. the section info has already been set
//
// HowTo:
//      1. if outside the text block, insert at the beginning or append
//         at the very end
//      2. if current line can hold the new box, just insert it
//      3. if can't
//          1. if current line ends with return, create a new line ends
//             with return and contain all needed tokens, insert this 
//             line next to current line. make current line and the new
//             line both end with return
//          2. if not, move token at the end of current line and insert
//             it at the beginning of next line, until current line has
//             enough space to hold the new token
//  return:
//      1. true most likely
//      2. false when current token is tooooooo wide that a whole line
//         even can't hold it
bool TokenBoard::AddTokenBoxV(int nX, int nY, const std::vector<TOKENBOX> & rstTBV)
{
    if(rstTBV.empty()){ return true; }

    if(nY < 0){
        nX = 0;
        nY = 0;
    }else if(nY >= (int)m_LineV.size()){
        nX = m_LineV.back().size();
        nY = m_LineV.size() - 1;
    }

    nX = std::max(std::min(nX, (int)m_LineV[nY].size()), 0);

    // now (nX, nY) are well-defined
    m_LineV[nY].insert(m_LineV[nY].begin() + nX, rstTBV.begin(), rstTBV.end());

    if(m_PW <= 0){
        // we don't have to wrap, easy case
        ResetLine(nY);
        return true;
    }

    // we need to wrap the text
    // count the tokens, to put proper tokens in current line
    int nCount = -1;
    int nWidth =  0;
    for(int nIndex = 0; nIndex < (int)m_LineV[nY].size(); ++nIndex){
        nWidth += m_LineV[nY][nIndex].Cache.W;

        if(nWidth + nIndex * m_WordSpace > m_PW){
            nCount = nIndex;
            break;
        }
    }

    if(nCount == 0){
        // the tokenbox is toooo wide, just fail and return false
        return false;
    }

    if(nCount == -1){
        // nCount didn't set, mean it can hold the whole v
        ResetLine(nY);
        return true;
    }

    std::vector<TOKENBOX> stRestTBV {
        m_LineV[nY].begin() + nCount, m_LineV[nY].end()};

    m_LineV[nY].resize(nCount);
    ResetLine(nY);

    // now the tokenboard is valid again
    //
    if(m_EndWithReturn[nY]){
        m_EndWithReturn[nY] = false;
        m_LineV.insert(m_LineV.begin() + nY + 1, {});
        m_EndWithReturn.insert(m_EndWithReturn.begin() + nY + 1, true);
    }

    // afer this, the tokenboard is valid again
    return AddTokenBoxV(nY + 1, 0, stRestTBV);
}

// get the location and shape info of tokenbox
bool TokenBoard::GetTokenBoxInfo(int nX, int nY,
        int *pType, int *pX, int *pY, int *pW, int *pH, int *pW1, int *pW2)
{
    if(nY < 0 || nY >= (int)m_LineV.size()){ return false; }
    if(nX < 0 || nX >= (int)m_LineV[nY].size()){ return false; }

    int  nSection = m_LineV[nY][nX].Section;
    auto p = m_SectionV.find(nSection);
    if(p == m_SectionV.end()){ return false; }

    if(pType){ *pType = p->second.Info.Type;          }
    if(pX   ){ *pX    = m_LineV[nY][nX].Cache.StartX; }
    if(pY   ){ *pY    = m_LineV[nY][nX].Cache.StartY; }
    if(pW   ){ *pW    = m_LineV[nY][nX].Cache.W;      }
    if(pH   ){ *pH    = m_LineV[nY][nX].Cache.H;      }
    if(pW1  ){ *pW1   = m_LineV[nY][nX].State.W1;     }
    if(pW2  ){ *pW2   = m_LineV[nY][nX].State.W2;     }

    return true;
}

// remove tokens, for bSelectedOnly
//    1:  only shadow will be deleted, for CTRL-X
//    0:  if there is shadow, remove shadow, else remove, current cursor bound tokenbox
void TokenBoard::Delete(bool bSelectedOnly)
{
    int nX1, nY1, nX2, nY2;
    if(m_SelectState == 2){
        nX1 = m_SelectLoc[0].first;
        nY1 = m_SelectLoc[0].second;
        nX2 = m_SelectLoc[1].first;
        nY2 = m_SelectLoc[1].second;
        DeleteTokenBox(nX1, nY1, nX2, nY2);
    }else{
        // so when you selecting and hold the left mouse button
        // and then press backspace, it won't be deleted
        if(!bSelectedOnly){
            nX1 = nX2 = m_CursorLoc.first;
            nY1 = nY2 = m_CursorLoc.second;
            DeleteTokenBox(nX1, nY1, nX2, nY2);
        }
    }
}

// for delete, (x, y) are location of tokens, not cursor
// this funciton is for internal use only
//
// outer user can only call Delete(bool) to delete by cursor
// or by selected region
bool TokenBoard::DeleteTokenBox(int nX0, int nY0, int nX1, int nY1)
{
    // TODO & TBD
    // think about this boundary check
    //
    // nothing to delete
    if(m_LineV.empty()){ return true; }

    // do have something...
    if(nY0 < 0){ nX0 = 0; nY0 = 0; }

    if(nY1 >= (int)m_LineV.size()){
        nX1 = m_LineV.back().size();
        nY1 = m_LineV.size() - 1;
    }

    if((nY0 > nY1) || (nY0 == nY1 && nX0 > nX1)){ return false; }

    nY0 = std::max(0, std::min(nY0, (int)m_LineV.size()));
    nY1 = std::max(0, std::min(nY1, (int)m_LineV.size()));
    nX0 = std::max(0, std::min(nX0, (int)m_LineV[nY0].size()));
    nX1 = std::max(0, std::min(nX1, (int)m_LineV[nY1].size()));

    // now (x0, y0, x1, y1) are well-prepared

    // TBD & TODO
    // will delete on line N affect line (n - 1)? YES:
    //      1. current line ends with <CR>
    //      2. last line doesn't ends with <CR>
    //      3. delete all of current line
    // Then we need to put a <CR> at last line and padding it if necessary
    //

    if(m_Spacing &&  m_PW > 0                        // system asks for padding

            &&  nY0 >= 1                             // nY0 is not the first line
            &&  nX0 == 0                             // delete start from the very beginning
            &&  nX1 == (int)m_LineV[nY1].size()      // delete ends at the very end
            &&  m_EndWithReturn[nY1]                 // current line ends with <CR>
            && !m_EndWithReturn[nY0 - 1]){           // last line is not ends with <CR>

        m_EndWithReturn[nY0 - 1] = true;
        ResetOneLine(nY0 - 1);
    }

    // now (0 ~ (nY0 -1)) are well-prepared
    // 1. delete whole lines, only need to reset startY
    // 2. line nY1 ends without <CR>:
    //      1. take pieces from line nY0 and line nY1 and store in one TBV
    //      2. delete line (nY0 ~ nY1), then line nY1 + 1 becomes nY0
    //      3. reset line nY0 (only startY) to make current board valid
    //      4. insert the stored TBV at (0, nY0)
    // 3. line nY1 ends with <CR>
    //      1. store pieces from line nY1 in one TBV
    //      2. delete line (nY0 + 1) ~ nY1
    //      3. resize line nY0
    //      4. change line nY0 ends with <CR>
    //      5. reset line nY0 (all) to make board valid
    //      6. insert TBV at the end of nY0
    //
    // we delete whole lines from nY0 ~ nY1

    auto fnDeleteLine = [this](int nLine0, int nLine1){
        m_LineV.erase(
                m_LineV.begin() + nLine0, m_LineV.begin() + nLine1 + 1);
        m_EndWithReturn.erase(
                m_EndWithReturn.begin() + nLine0, m_EndWithReturn.begin() + nLine1 + 1);
    };

    // case 1
    if(nX0 == 0 && nX1 == (int)m_LineV[nY1].size()){
        // delete all, just erase this line
        fnDeleteLine(nY0, nY1);
        ResetLineStartY(nY0);
        return true;
    }

    // case 2
    if(!m_EndWithReturn[nY1]){
        std::vector<TOKENBOX> stTBV;
        stTBV.insert(stTBV.end(), m_LineV[nY0].begin(), m_LineV[nY0].begin() + nX0);
        stTBV.insert(stTBV.end(), m_LineV[nY1].begin() + nX1 + 1, m_LineV[nY1].end());

        fnDeleteLine(nY0, nY1);
        ResetLineStartY(nY0);
        return AddTokenBoxV(0, nY0, stTBV);
    }else{
        std::vector<TOKENBOX> stTBV(m_LineV[nY1].begin() + nX1 + 1, m_LineV[nY1].end());
        fnDeleteLine(nY0 + 1, nY1);

        m_LineV[nY0].resize(nX0);
        m_EndWithReturn[nY0] = true;

        ResetLine(nY0);
        return AddTokenBoxV(m_LineV[nY0].size(), nY0, stTBV);
    }
}

// put current board in empty but valid state
// 1. clear all containers
// 2. reset all variables
// 3. etc
//
void TokenBoard::Clear()
{
    m_LineV.clear();
    m_IDFuncV.clear();
    m_SectionV.clear();
    m_LineStartY.clear();
    m_TokenBoxBitmap.clear();

    m_MaxH1            = 0;
    m_MaxH2            = 0;
    m_W                = 0;
    m_H                = 0;
    m_SelectState      = 0;
    m_CurrentWidth     = 0;
    m_CurrentLineMaxH2 = 0;
    m_SkipEvent        = false;
    m_LastTokenBox     = nullptr;
}

// add a <CR> before the current cursor, since we defined the concept of ``default font",
// let's support ``empty line".
//
// HOWTO:
//      1. 
//
// assmuption:
//      1. the current board is valid
//      2. there is valid buffer for the insertion
//      3. cursor position is well-defined
//
bool TokenBoard::ParseReturnObject()
{
    int nX = m_CursorLoc.first;
    int nY = m_CursorLoc.second;

    if(!(nY >= 0 && nY < (int)m_LineV.size() && nX >= 0 && nX <= (int)m_LineV[nY].size())){
        // invalid cursor location, bye
        return false;
    }

    // anyway we need to handle it case by case
    // so do it in detail

    // 1. decide whether last line will be in affect
    if(nX == 0 && nY > 0 && !m_EndWithReturn[nY - 1]){
        // 1. cursor it at the beginning
        // 2. current is not the first line
        // 3. last line doesn't end with <CR>
        m_EndWithReturn[nY - 1] = true;
        ResetOneLine(nY - 1);
    }

    // get the content after the cursor, maybe empty
    std::vector<TOKENBOX> stTBV(m_LineV[nY].begin() + nX, m_LineV[nY].end());
    m_LineV[nY].resize(nX);

    // backup current line end state
    bool bEndWithReturn = m_EndWithReturn[nY];

    // 2. reset current line
    m_EndWithReturn[nY] = true;
    ResetOneLine(nY);

    // 3. handle following line
    if(bEndWithReturn){
        // the original line ends with return, then we insert a new line
        // actually if the cursor is at the beginning, then we even don't have
        // to re-padding it
        //
        // since only one line, it's OK
        m_LineV.insert(m_LineV.begin() + nY, stTBV);
        m_EndWithReturn.insert(m_EndWithReturn.begin() + nY, true);
        ResetOneLine(nY + 1);

        // reset all startY for the rest
        ResetLineStartY(nY + 2);
    }else{
        AddTokenBoxV(0, nY + 1, stTBV);
    }
}

// reset one line for
// 1. do padding if needed
// 2. calculate the cache.startx
// 3. calculate the cache.starty
//
// assmuption:
//      1. Line 0 ~ (nLine - 1) are valid
//      2.
//
// return:
//      1. Line 0 ~ nLine are valid
void TokenBoard::ResetOneLine(int nLine)
{
    if(nLine < 0 || nLine >= (int)m_LineV.size()){
        // invalid line, just return
        return;
    }

    if(!m_EndWithReturn[nLine]){
        LinePadding(nLine);
    }
    SetTokenBoxStartX(nLine);

    m_LineStartY[nLine] = GetNthNewLineStartY(nLine);
    SetTokenBoxStartY(nLine, m_LineStartY[nLine]);
}

// reset the StartY from nStartLine to the end
//
// why define this function, because most of the time we update (nStartLine - 1), reset its StartX/Y
// and then the first half [0, nStartLine - 1] of the board is valid, but we need to adjust the sec-
// ond part of the board, which only needs recalculate for StartY. We can calculate StartY one line
// by one line but this is not good though, we can find the longest line of the rest, say line N,
// then for line [nStartLine, N] we do it one by one, but for [N + 1, end()) we do simplily add nDY
// for each line and it's good enough.
//
// assumption:
//      1. Line 0 ~ (nStartLine - 1) are valid 
//      2. if take 0 ~ (nStartLine - 1) out, and reset StartY for nStartLine ~ end() as a new
//         board, then this new board is also ``valid".
//
// return:
//      1. a valid board
//
void TokenBoard::ResetLineStartY(int nStartLine)
{
    if(nStartLine < 0 || nStartLine >= (int)m_LineV.size()){
        // invalid line number, just return
        return;
    }

    int nLongestLine  = m_LineV.size() - 1;
    int nLongestLineW = -1;

    // get the longest line, by assumption we can directly use cache info
    for(int nIndex = nStartLine; nIndex < (int)m_LineV.size(); ++nIndex){
        if(!m_LineV[nIndex].empty()){
            const auto &rstTB = m_LineV[nIndex].back();
            // actually back().State.W2 is always 0, put it here for nothing
            int nCurrentWidth = rstTB.Cache.StartX + rstTB.Cache.W + rstTB.State.W2;
            if(nCurrentWidth > nLongestLineW){
                nLongestLineW = nCurrentWidth;
                nLongestLine  = nIndex;
            }
        }
    }

    int nOldLongestLineStartY = m_LineStartY[nLongestLine];
    for(int nIndex = nStartLine; nIndex < (int)m_LineV.size() && nIndex <= nLongestLine; ++nIndex){
        m_LineStartY[nIndex] = GetNthNewLineStartY(nIndex);
        SetTokenBoxStartY(nIndex, m_LineStartY[nIndex]);
    }

    int nDStartY = m_LineStartY[nIndex] - nOldLongestLineStartY;
    for(int nIndex = nLongestLine + 1; nIndex < (int)m_LineV.size(); ++nIndex){
        m_LineStartY[nIndex] += nDStartY;
    }
}

int TokenBoard::CreateSection(const SECTION &rstSection, const std::function<void()> &fnCB)
{
    // TODO
    // make it more efficient
    int nID = 0;
    while(nID <= std::numeric_limits<int>::max()){
        if(m_SectionV.find(nID) == m_SectionV.end()){
            // good
            m_SectionV[nID] = rstSection;
            m_IDFuncV[nID]  = fnCB;
            return nID;
        }
        nID++;
    }
    return -1;
}

// insert a utf-8 char box
// assume:
//      1. valid tokenboard
bool TokenBoard::AddUTF8Code(uint32_t)
{
    TOKENBOX stTB;
    std::memset(&stTB, 0, sizeof(stTB));
}

void TokenBoard::GetDefaultFontInfo(uint8_t *pFont, uint8_t *pFontSize, uint8_t *pFontStyle)
{
    if(pFont     ){ *pFont      = m_DefaultFont; }
    if(pFontSize ){ *pFontSize  = m_DefaultFontSize; }
    if(pFontStyle){ *pFontStyle = m_DefaultFontStyle; }
}
