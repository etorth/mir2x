/*
 * =====================================================================================
 *
 *       Filename: tokenboard.cpp
 *        Created: 06/17/2015 10:24:27
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

#include <map>
#include <utf8.h>
#include <string>
#include <algorithm>
#include <functional>
#include <tinyxml2.h>
#include <SDL2/SDL.h>

#include "log.hpp"
#include "section.hpp"
#include "emoticon.hpp"
#include "tokenbox.hpp"
#include "utf8char.hpp"
#include "mathfunc.hpp"
#include "fontexdb.hpp"
#include "condcheck.hpp"
#include "colorfunc.hpp"
#include "tokenboard.hpp"
#include "emoticondb.hpp"
#include "xmlobjectlist.hpp"

extern Log *g_Log;
extern FontexDB *g_FontexDB;
extern SDLDevice *g_SDLDevice;
extern EmoticonDB *g_EmoticonDB;

// insert an XML file to current board
// assumption: current board is well-prepared
bool TokenBoard::InnInsert(const XMLObjectList &rstXMLObjectList, const IDHandlerMap &rstIDHandleMap)
{
    // if XMLObjectList is empty, this is OK and we return true
    // if unknown section type detected we give a warning and won't abort parsing

    bool bRes = true;
    for(auto pObject = rstXMLObjectList.FirstElement(); pObject && bRes; pObject = pObject->NextSiblingElement()){
        switch(auto nObjectType = XMLObject::ObjectType(*pObject)){
            case OBJECTTYPE_RETURN:
                {
                    bRes = ParseReturnObject();
                    break;
                }
            case OBJECTTYPE_EMOTICON:
                {
                    bRes = ParseEmoticonObject(*pObject);
                    break;
                }
            case OBJECTTYPE_EVENTTEXT:
            case OBJECTTYPE_PLAINTEXT:
                {
                    bRes = ParseTextObject(*pObject, nObjectType, rstIDHandleMap);
                    break;
                }
            default:
                {
                    g_Log->AddLog(LOGTYPE_WARNING, "Detected known object type, ignored it");

                    bRes = true;
                    break;
                }
        }
    }
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
        try{
            auto stColor = ColorFunc::RGBA2Color(ColorFunc::String2RGBA(pText));
            if(pOutColor){
                *pOutColor = stColor;
            }
            return true;
        }catch(...){}
    }

    if(pOutColor){
        *pOutColor = rstDefaultColor;
    }
    return false;
}

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

    // no matter succeed or not, we need to set it if non-null pointer
    if(pOut){ *pOut = nOut; }
    return bRes;
}

// insert an emoticon to current board
// assumption: current board is well-prepared
bool TokenBoard::ParseEmoticonObject(const tinyxml2::XMLElement &rstCurrentObject)
{
    SECTION  stSection;
    TOKENBOX stTokenBox;
    std::memset(&stSection , 0, sizeof(stSection) );
    std::memset(&stTokenBox, 0, sizeof(stTokenBox));

    stSection.Info.Type                 = SECTIONTYPE_EMOTICON;
    stSection.State.Emoticon.FrameIndex = 0;
    stSection.State.Emoticon.MS         = 0.0;

    GetAttributeAtoi(&(stSection.Info.Emoticon.Set),   0, rstCurrentObject, {"SET", "Set", "set"});
    GetAttributeAtoi(&(stSection.Info.Emoticon.Index), 0, rstCurrentObject, {"INDEX", "Index", "index"});

    auto pTexture = g_EmoticonDB->Retrieve(
            stSection.Info.Emoticon.Set,          // emoticon set
            stSection.Info.Emoticon.Index,        // emoticon index
            0,                                    // first frame of the emoticon
            nullptr,                              // don't need the start info here
            nullptr,                              // don't need the satrt info here
            nullptr,
            nullptr,
            &stSection.Info.Emoticon.FPS,         // o
            nullptr,                              // o
            &stSection.Info.Emoticon.FrameCount); // o

    if(pTexture){
        stSection.State.Emoticon.Valid = 1;
    }

    int nSectionID = CreateSection(stSection);
    if(nSectionID >= 0){

        // we can use a uint32_t to indicate the emoticon, but it's uncessary
        // since it already contained in the section info, so just pass the key as zero

        if(MakeTokenBox(nSectionID, 0, &stTokenBox)){
            return AddTokenBoxLine({stTokenBox});
        }

        // failed to make the token
        // we should remove the section record
        m_SectionRecord.erase(nSectionID);
    }
    return false;
}

// insert an plain/event text to current board
// board will assign a new section to host and manage it
// assumption: current board is well-prepared
bool TokenBoard::ParseTextObject(const tinyxml2::XMLElement &rstCurrentObject, int nObjectType, const IDHandlerMap &rstIDHandleMap)
{
    switch(nObjectType){
        case OBJECTTYPE_EVENTTEXT:
        case OBJECTTYPE_PLAINTEXT:
            {
                break;
            }
        default:
            {
                g_Log->AddLog(LOGTYPE_WARNING, "Invalid object type: %d", nObjectType);
                return false;
            }
    }

    SECTION stSection;
    std::memset(&stSection, 0, sizeof(stSection));

    if(nObjectType == OBJECTTYPE_PLAINTEXT){
        stSection.Info.Type = SECTIONTYPE_PLAINTEXT;
    }else{
        stSection.Info.Type = SECTIONTYPE_EVENTTEXT;
    }

    int nFontCode = 0;
    GetAttributeAtoi(&nFontCode, m_DefaultFont, rstCurrentObject, {"FONT", "Font", "font"});
    stSection.Info.Text.Font = (uint8_t)(nFontCode);

    // TODO: need to support it
    // GetAttributeAtoi(&(stSection.Info.Text.Style), 0, rstCurrentObject, {"STYLE", "Style", "style"});

    int nFontSize = 0;
    GetAttributeAtoi(&nFontSize, m_DefaultSize, rstCurrentObject, {"SIZE", "Size", "size"});
    stSection.Info.Text.Size = (uint8_t)nFontSize;

    std::function<void()> fnCallback;
    if(auto szID = rstCurrentObject.Attribute("ID")){
        auto pCBRecord = rstIDHandleMap.find(std::string(szID));
        if(pCBRecord != rstIDHandleMap.end()){
            fnCallback = pCBRecord->second;
        }
    }

    GetAttributeColor(stSection.Info.Text.BackColor + 0, {0X00, 0X00, 0X00, 0X00}, rstCurrentObject, {"BACKCOLOR", "BackColor", "backcolor"});
    if(nObjectType == OBJECTTYPE_PLAINTEXT){
        GetAttributeColor(stSection.Info.Text.Color + 0, {0XFF, 0XFF, 0XFF, 0XFF}, rstCurrentObject, {"COLOR", "Color", "color"});
    }else{
        GetAttributeColor(stSection.Info.Text.Color + 0, {0XFF, 0XFF, 0X00, 0XFF}, rstCurrentObject, {"OFF",  "Off",  "off"});
        GetAttributeColor(stSection.Info.Text.Color + 1, {0X00, 0XFF, 0X00, 0XFF}, rstCurrentObject, {"OVER", "Over", "over"});
        GetAttributeColor(stSection.Info.Text.Color + 2, {0XFF, 0X00, 0X00, 0XFF}, rstCurrentObject, {"DOWN", "Down", "down"});
    }

    stSection.State.Text.Event = 0;

    int nSectionID = CreateSection(stSection, fnCallback);
    if(nSectionID < 0){ return false; }

    // then we parse event text content
    auto pStart = rstCurrentObject.GetText();
    auto pEnd   = pStart;

    // when get inside this funciton
    // the section structure has been well-prepared

    std::vector<TOKENBOX> stTBV;
    while(*pEnd != '\0'){
        pStart = pEnd;
        utf8::unchecked::advance(pEnd, 1);

        // should be true
        condcheck(pEnd - pStart <= 4);

        uint32_t nUTF8Key = 0;
        std::memcpy(&nUTF8Key, pStart, pEnd - pStart);

        // fully set the token box unit
        TOKENBOX stTokenBox;
        if(MakeTokenBox(nSectionID, nUTF8Key, &stTokenBox)){
            stTBV.push_back(stTokenBox);
        }else{
            m_SectionRecord.erase(nSectionID);
            return false;
        }
    }

    return AddTokenBoxLine(stTBV);
}

void TokenBoard::Update(double fMS)
{
    if(true
            &&  fMS > 0.0
            && !m_SkipUpdate){

        // if not valid parameter
        // if current board dosn't have emotion we set m_SkipUpdate

        for(auto &rstInst: m_SectionRecord){
            auto &rstSection = rstInst.second.Section;
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
}

int TokenBoard::SectionTypeCount(int nLine, int nSectionType)
{
    if(LineValid(nLine)){
        auto fnCmp = [](int nType, int nArgType) -> bool
        {
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

        switch(nSectionType){
            case SECTIONTYPE_ALL:
                {
                    return (int)(m_LineV[nLine].Content.size());
                }
            default:
                {
                    int nCount = 0;
                    for(const auto &rstTokenBox: m_LineV[nLine].Content){
                        if(SectionValid(rstTokenBox.Section)){
                            nCount += (fnCmp(m_SectionRecord[rstTokenBox.Section].Section.Info.Type, nSectionType) ? 1 : 0);
                        }else{
                            g_Log->AddLog(LOGTYPE_INFO, "Invalid section ID: %d", rstTokenBox.Section);
                            return -1;
                        }
                    }
                    return nCount;
                }
        }
    }
    return -1;
}

// padding to set the current line width to be m_MaxLineWidth
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
int TokenBoard::DoLinePadding(int nLine, int nDWidth, int nSectionType)
{
    if(!LineValid(nLine)){ return -1; }
    if(nDWidth <  0){ return -1; }
    if(nDWidth == 0){ return  0; }

    int nCount = SectionTypeCount(nLine, nSectionType);
    if(false
            || nCount == 0
            || m_LineV[nLine].Content.size() == 0
            || m_LineV[nLine].Content.size() == 1){

        // for cases we can't do padding
        // 1. no specified section type
        // 2. have only 0 or 1 box in the line
        return nDWidth;
    }

    auto &stCurrLine = m_LineV[nLine];
    auto nPaddingSpace = nDWidth / nCount;

    // first round
    // first token may reserve for no padding

    if(TokenBoxType(stCurrLine.Content[0]) != TokenBoxType(stCurrLine.Content[1])){
        if(nSectionType == 0 || TokenBoxType(stCurrLine.Content[0]) & nSectionType){
            stCurrLine.Content[0].State.W2 += (nPaddingSpace / 2);
            nDWidth -= nPaddingSpace / 2;
            if(nDWidth == 0){ return 0; }
        }
    }

    for(size_t nIndex = 1; nIndex < stCurrLine.Content.size() - 1; ++nIndex){
        if(nSectionType == 0 || TokenBoxType(stCurrLine.Content[nIndex]) & nSectionType){
            stCurrLine.Content[nIndex].State.W1 += (nPaddingSpace / 2);
            stCurrLine.Content[nIndex].State.W2 += (nPaddingSpace - nPaddingSpace / 2);

            nDWidth -= nPaddingSpace;
            if(nDWidth == 0){ return 0; }
        }
    }

    if(nSectionType == 0 || TokenBoxType(stCurrLine.Content.back()) & nSectionType){
        stCurrLine.Content.back().State.W1 += (nPaddingSpace / 2);
        nDWidth -= nPaddingSpace / 2;
        if(nDWidth == 0){ return 0; }
    }

    // second round, small update
    // every update only add 1 pixel

    if(TokenBoxType(stCurrLine.Content[0]) != TokenBoxType(stCurrLine.Content[1])){
        if(nSectionType == 0 || TokenBoxType(stCurrLine.Content[0]) & nSectionType){
            stCurrLine.Content[0].State.W2++;;
            nDWidth--;
            if(nDWidth == 0){ return 0; }
        }
    }

    for(size_t nIndex = 1; nIndex < stCurrLine.Content.size() - 1; ++nIndex){
        if(nSectionType == 0 || TokenBoxType(stCurrLine.Content[nIndex]) & nSectionType){
            if(stCurrLine.Content[nIndex].State.W1 <=  stCurrLine.Content[nIndex].State.W2){
                stCurrLine.Content[nIndex].State.W1++;
            }else{
                stCurrLine.Content[nIndex].State.W2++;
            }
            nDWidth--;
            if(nDWidth == 0){ return 0; }
        }
    }

    if(nSectionType == 0 || TokenBoxType(stCurrLine.Content.back()) & nSectionType){
        stCurrLine.Content.back().State.W1++;
        nDWidth--;
        if(nDWidth == 0){ return 0; }
    }

    return nDWidth;
}

void TokenBoard::SetTokenBoxStartY(int nLine, int nBaseLineY)
{
    if(LineValid(nLine)){
        for(auto &rstTokenBox: m_LineV[nLine].Content){
            rstTokenBox.Cache.StartY = nBaseLineY - rstTokenBox.Cache.H1;
        }
    }
}

void TokenBoard::SetTokenBoxStartX(int nLine)
{
    if(LineValid(nLine)){
        int nCurrentX = m_Margin[3];
        for(auto &rstTokenBox: m_LineV[nLine].Content){
            nCurrentX += rstTokenBox.State.W1;
            rstTokenBox.Cache.StartX = nCurrentX;

            nCurrentX += rstTokenBox.Cache.W;
            nCurrentX += rstTokenBox.State.W2;
        }
    }
}

int TokenBoard::LineFullWidth(int nLine) const
{
    if(LineValid(nLine)){
        int nWidth = 0;
        for(auto &rstTB: m_LineV[nLine].Content){
            nWidth += (rstTB.Cache.W + rstTB.State.W1 + rstTB.State.W2);
        }
        return nWidth;
    }
    return -1;
}

int TokenBoard::LineRawWidth(int nLine, bool bWithWordSpace)
{
    if(LineValid(nLine)){
        switch(m_LineV[nLine].Content.size()){
            case 0:
                {
                    return 0;
                }
            case 1:
                {
                    return m_LineV[nLine].Content[0].Cache.W;
                }
            default:
                {
                    int nWidth = 0;
                    for(auto &rstTB: m_LineV[nLine].Content){
                        nWidth += rstTB.Cache.W;
                    }

                    if(bWithWordSpace){
                        nWidth += m_WordSpace * ((int)(m_LineV[nLine].Content.size()) - 1);
                    }
                    return nWidth;
                }
        }
    }
    return -1;
}

int TokenBoard::SetTokenBoxWordSpace(int nLine)
{
    if(LineValid(nLine)){
        int nW1 = m_WordSpace / 2;
        int nW2 = m_WordSpace - nW1;

        for(auto &rstTB: m_LineV[nLine].Content){
            rstTB.State.W1 = nW1;
            rstTB.State.W2 = nW2;
        }

        if(!m_LineV[nLine].Content.empty()){
            m_LineV[nLine].Content[0]    .State.W1 = 0;
            m_LineV[nLine].Content.back().State.W2 = 0;
        }

        return LineRawWidth(nLine, true);
    }
    return -1;
}

// padding with space to make the line be exactly of width m_MaxLineWidth
// for input:
//      1. all tokens in current line has W specified
//      2. W1/W2 is not-inited
// output:
//      1. W1/W2 inited
//      2. return real width after operation of this function
int TokenBoard::LinePadding(int nLine)
{
    if(LineValid(nLine)){
        int nWidth = SetTokenBoxWordSpace(nLine);
        if(nWidth < 0){ return -1; }

        // if we don't need space padding, that's all
        if(m_MaxLineWidth <= 0){ return nWidth; }

        // we need word space padding and extra padding
        int nDWidth = m_MaxLineWidth - nWidth;

        if(nDWidth > 0){
            // round-1: try to padding by emoticons
            int nNewDWidth = DoLinePadding(nLine, nDWidth, SECTIONTYPE_EMOTICON);
            if(nNewDWidth < 0 || nNewDWidth == nDWidth){
                // doesn't work, padding by all boxes
                nNewDWidth = DoLinePadding(nLine, nDWidth, 0);
                if(nNewDWidth == 0){
                    return m_MaxLineWidth;
                }
                // report as error
                // even padding all doesn't work
                return -1;
            }
        }
        return nWidth;

    }
    return -1;

}

int TokenBoard::GetLineIntervalMaxH2(int nLine, int nIntervalStartX, int nIntervalWidth)
{
    //    This function only take care of nLine
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
    for(auto &rstTokenBox: m_LineV[nLine].Content){
        int nX  = rstTokenBox.Cache.StartX;
        int nW  = rstTokenBox.Cache.W;
        int nH2 = rstTokenBox.Cache.H2;
        int nW1 = rstTokenBox.State.W1;
        int nW2 = rstTokenBox.State.W2;

        if(MathFunc::IntervalOverlap(nX, nW1 + nW + nW2, nIntervalStartX, nIntervalWidth)){
            nMaxH2 = (std::max<int>)(nMaxH2, nH2);
        }
    }
    // maybe the line is not long enough to cover Interval
    // in this case we return -1
    return nMaxH2;
}

int TokenBoard::GetLineTokenBoxStartY(int nLine, int nStartX, int nBoxWidth, int nBoxHeight)
{
    while(nLine - 1 >= 0){
        int nMaxH2 = GetLineIntervalMaxH2(nLine - 1, nStartX, nBoxWidth);
        if(nMaxH2 >= 0){
            return m_LineV[nLine - 1].StartY + nMaxH2 + m_LineSpace + nBoxHeight;
        }else{
            // OK current line is not long enough
            // check whether we permit the token box go through
            //           +----------+                  
            //           |          |                  
            // +-------+ |          |                   
            // | Words | | Emoticon |  Words             
            // +-------+-+----------+------------------
            //           |          |   +----------+  
            //           +----------+   |          |
            //                          |   TB0    |                  
            //                +-------+ |          |                   
            //                | Words | | Emoticon |  Words             
            //                +-------+-+----------+-------           
            //                          |          |                   
            //                          +----------+                  
            //
            //  if bCanThrough is true, then we let TB0 go through last line
            //  else we just return, for bCanThrough == false, we always do
            //  one test and return
            //
            if(!m_CanThrough){
                return m_LineV[nLine - 1].StartY + m_LineSpace + nBoxHeight;
            }

            // else we'll go through last line
        }
        nLine--;
    }
    // there is no line upside
    return m_Margin[0] + m_LineSpace / 2 + nBoxHeight;
}

// get StartY of current Line
// assume:
//      1. 0 ~ (nLine - 1) are well-prepared with StartX, StartY, W/W1/W2/H1/H2 etc
//      2. nLine is padded already, StartX, W/W1/W2 are OK now
int TokenBoard::GetNewLineStartY(int nLine)
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
    // for nLine we use W only, but for nLine - 1, we use W1 + W + W2
    // reason is clear since W1/2 of nLine won't show, then use them are
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

    // 1. parameters check
    if(!LineValid(nLine)){ return -1; }

    // 2. we allow empty line for logic consistency
    if(m_LineV[nLine].Content.empty()){
        int nBlankLineH = GetBlankLineHeight();
        if(nLine == 0){
            return m_Margin[0] + m_LineSpace / 2 + nBlankLineH;
        }else{
            return m_LineV[nLine - 1].StartY + m_LineSpace + nBlankLineH;
        }
    }

    // 3. get the best start point
    //    there is a in-consistency here, when we set bCanThrough as true but
    //    we also permit blank line, then (nLine + 1) will cover nLine if
    //    nLine is empty, this means we can't draw a proper blank line
    //
    //    so if nLine is emtpy, we always temporarily disable (nLine + 1) to
    //    go through nLine, to reserve the blank line space

    // now nLine is not empty
    // last line is empty, then we always disable the bCanThrough
    if(LineValid(nLine - 1) && m_LineV[nLine - 1].Content.empty()){
        int nCurrMaxH1 = GetLineMaxH1(nLine);
        return m_LineV[nLine - 1].StartY + m_LineSpace + nCurrMaxH1;
    }

    // ok current line is not empty, last line is not empty
    int nCurrentY = -1;
    for(auto &rstTokenBox: m_LineV[nLine].Content){
        int nX  = rstTokenBox.Cache.StartX;
        int nW  = rstTokenBox.Cache.W;
        int nH1 = rstTokenBox.Cache.H1;

        // GetLineTokenBoxStartY() already take m_LineSpace into consideration
        nCurrentY = (std::max<int>)(nCurrentY, GetLineTokenBoxStartY(nLine, nX, nW, nH1));
    }

    return nCurrentY;
}

// everything dynamic and can be retrieve is in Cache
//            dynamic but can not be retrieved is in State
//            static is in Info
// we assume all token box are well-prepared here!
// means cache, state, info are all valid now when invoke this function
void TokenBoard::DrawEx(
        int nDstX, int nDstY, // start position of drawing on the screen
        int nSrcX, int nSrcY, // region to draw, a cropped region on the token board
        int nSrcW, int nSrcH)
{
    // 1. if no overlapping at all then directly return
    if(!MathFunc::RectangleOverlap(nSrcX, nSrcY, nSrcW, nSrcH, 0, 0, W(), H())){ return; }

    // get the coordinate of the top-left corner point on the dst
    int nDstDX = nDstX - nSrcX;
    int nDstDY = nDstY - nSrcY;

    // 2. check tokenbox one by one, this is expensive
    for(auto rstLine: m_LineV){
        for(auto &rstTokenBox: rstLine.Content){
            int nX = rstTokenBox.Cache.StartX;
            int nY = rstTokenBox.Cache.StartY;
            int nW = rstTokenBox.Cache.W;
            int nH = rstTokenBox.Cache.H;

            // try to get the clipped region of the tokenbox
            if(!MathFunc::RectangleOverlapRegion(nSrcX, nSrcY, nSrcW, nSrcH, &nX, &nY, &nW, &nH)){ continue; }
            if(!SectionValid(rstTokenBox.Section, true)){
                g_Log->AddLog(LOGTYPE_INFO, "section id invalid: %d", rstTokenBox.Section);
                return;
            }

            switch(m_SectionRecord[rstTokenBox.Section].Section.Info.Type){
                case SECTIONTYPE_EVENTTEXT:
                case SECTIONTYPE_PLAINTEXT:
                    {
                        // the purpose of introducing FontexDB is for saving video memory, it's
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

                        int nEvent = m_SectionRecord[rstTokenBox.Section].Section.State.Text.Event;
                        auto &rstColor = m_SectionRecord[rstTokenBox.Section].Section.Info.Text.Color[nEvent];
                        auto &rstBackColor = m_SectionRecord[rstTokenBox.Section].Section.Info.Text.BackColor[0];

                        auto pTexture = g_FontexDB->Retrieve(rstTokenBox.UTF8CharBox.Cache.Key);

                        if(pTexture){
                            int nDX = nX - rstTokenBox.Cache.StartX;
                            int nDY = nY - rstTokenBox.Cache.StartY;
                            SDL_SetTextureColorMod(pTexture, rstColor.r, rstColor.g, rstColor.b);

                            g_SDLDevice->PushColor(rstBackColor.r, rstBackColor.g, rstBackColor.b, rstBackColor.a);
                            g_SDLDevice->PushBlendMode(SDL_BLENDMODE_BLEND);
                            g_SDLDevice->FillRectangle(nX + nDstDX, nY + nDstDY, nW, nH);
                            g_SDLDevice->PopBlendMode();
                            g_SDLDevice->PopColor();

                            g_SDLDevice->DrawTexture(pTexture, nX + nDstDX, nY + nDstDY, nDX, nDY, nW, nH);
                        }else{
                            // TODO
                            // draw a box here to indicate errors
                        }

                        break;
                    }

                case SECTIONTYPE_EMOTICON:
                    {
                        int nFrameIndex = m_SectionRecord[rstTokenBox.Section].Section.State.Emoticon.FrameIndex;

                        int nXOnTex, nYOnTex;
                        auto pTexture = g_EmoticonDB->Retrieve(rstTokenBox.EmoticonBox.Cache.Key + nFrameIndex, &nXOnTex, &nYOnTex, nullptr, nullptr, nullptr, nullptr, nullptr);

                        if(pTexture){
                            int nDX = nX - rstTokenBox.Cache.StartX;
                            int nDY = nY - rstTokenBox.Cache.StartY;
                            g_SDLDevice->DrawTexture(pTexture, nX + nDstDX, nY + nDstDY, nXOnTex + nDX, nYOnTex + nDY, nW, nH);
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
// assumption:
//      1. all needed tokens are already in current line
//      2. m_LineV[nLine].EndWithCR specified
//
//      3. if take lines [0, (nLine - 1)] out, and reset StartY for [nLine, end] as
//         a new board, then the new board is also valid
// return:
//      a valid board
//
// some times we alternate one line and we need to reset the board, this can be done
// by calling ResetOneLine() for [nLine, end]in principle, but we don't use that because
// from some line we can just add nDStartY to every line to validate it
void TokenBoard::ResetLine(int nLine)
{
    if(!LineValid(nLine)){ return; }

    // if the line ends with CR, do word space padding only
    // else do full padding: word space and W1 - W2
    if(m_LineV[nLine].EndWithCR){
        SetTokenBoxWordSpace(nLine);
    }else{
        LinePadding(nLine);
    }

    SetTokenBoxStartX(nLine);

    // may need to reset board width after reset current line
    // if line get longer, then easier
    // if shorter we recompute the board width

    int nWidth = LineFullWidth(nLine);
    if(m_W < nWidth + m_Margin[1] + m_Margin[3]){
        m_W = nWidth + m_Margin[1] + m_Margin[3];
    }else{
        m_W = 0;
        for(int nIndex = 0; nIndex < (int)(m_LineV.size()); ++nIndex){
            m_W = (std::max<int>)(m_W, LineFullWidth(nIndex));
        }
        m_W += (m_Margin[1] + m_Margin[3]);
    }

    // without StartX we can't calculate StartY
    //      1. for Loading function, this will only be one round
    //      2. for Insert function, this execute many times to re-calculate the StartY.
    //    
    // howto
    // 1. reset nLine, anyway this is needed
    //    actually we should report as an error here

    m_LineV[nLine].StartY = GetNewLineStartY(nLine);
    SetTokenBoxStartY(nLine, m_LineV[nLine].StartY);

    // 2. reset all rest lines, use a trick here
    //      if last line is full length, we only need to add a delta form now
    //      otherwise we continue to compute all StartY

    int nRestLine  = nLine + 1;
    int nTrickOn   = 0;
    int nDStartY   = 0;

    while(nRestLine < (int)(m_LineV.size())){
        if(nTrickOn){
            m_LineV[nRestLine].StartY += nDStartY;
        }else{
            int nOldStartY = m_LineV[nRestLine].StartY;
            m_LineV[nRestLine].StartY = GetNewLineStartY(nRestLine);
            if(LineFullWidth(nRestLine) + (m_Margin[1] + m_Margin[3]) == m_W){
                nTrickOn = 1;
                nDStartY = m_LineV[nRestLine].StartY - nOldStartY;
            }
        }

        SetTokenBoxStartY(nRestLine, m_LineV[nRestLine].StartY);
        nRestLine++;
    }

    m_H = GetNewHBasedOnLastLine();
}

void TokenBoard::TokenBoxGetMouseButtonUp(int nX, int nY, bool bFirstHalf)
{
    // 1. invalid position
    if(!TokenBoxValid(nX, nY)){ return; }

    // 2. invalid section id
    int nSection = m_LineV[nY].Content[nX].Section;
    if(!SectionValid(nSection)){
        g_Log->AddLog(LOGTYPE_WARNING, "Invalid section ID: %d", nSection);
        return;
    }

    auto &rstSection = m_SectionRecord[nSection].Section;
    switch(rstSection.Info.Type){
        case SECTIONTYPE_PLAINTEXT:
        case SECTIONTYPE_EMOTICON:
            {
                // emotion and plain text only takes selecting
                // all other event should be ignored
                switch(m_SelectState){
                    case SELECTTYPE_SELECTING:
                        {
                            m_SelectState = SELECTTYPE_DONE;
                            m_SelectLoc[1].X = nX - (bFirstHalf ? 1 : 0);
                            m_SelectLoc[1].Y = nY;
                            break;
                        }
                    default:
                        {
                            break;
                        }
                }
                break;
            }
        case SECTIONTYPE_EVENTTEXT:
            {
                // 1. may trigger event
                // 2. may be the end of dragging
                switch(rstSection.State.Text.Event){
                    case 0: // off
                        {
                            // impossible generally, only possibility comes with
                            // user are moving mouse *very* fast. Then pointer can
                            // jump inside one tokenbox and emit button up event.
                            //
                            // do nothing
                            break;
                        }
                    case 1: // over
                        {
                            // over state and then get release
                            // only possible for dragging to select the content
                            switch(m_SelectState){
                                case SELECTTYPE_SELECTING:
                                    {
                                        m_SelectState = SELECTTYPE_DONE;
                                        m_SelectLoc[1].X = nX - (bFirstHalf ? 1 : 0);
                                        m_SelectLoc[1].Y = nY;
                                        break;
                                    }
                                default:
                                    {
                                        break;
                                    }
                            }
                            break;
                        }
                    case 2: // pressed
                        {
                            // get released for a pressed one
                            // should trigger the event text callback

                            // 1. make as state ``over"
                            rstSection.State.Text.Event = 1;

                            // 2. trigger registered event handler
                            auto &rstCB = m_SectionRecord[nSection].Callback;
                            if(rstCB){ rstCB(); }

                            break;
                        }
                    default:
                        {
                            break;
                        }
                }
                break;
            }
        default:
            {
                break;
            }
    }
}

void TokenBoard::TokenBoxGetMouseButtonDown(int nX, int nY, bool bFirstHalf)
{
    // 1. invalid position
    if(TokenBoxValid(nX, nY)){ return; }

    // 2. invalid section id
    int nSection = m_LineV[nY].Content[nX].Section;
    if(!SectionValid(nSection, true)){
        g_Log->AddLog(LOGTYPE_INFO, "section id can't find: %d", nSection);
        return;
    }

    auto &rstSection = m_SectionRecord[nSection].Section;
    switch(rstSection.Info.Type){
        case SECTIONTYPE_PLAINTEXT:
        case SECTIONTYPE_EMOTICON:
            {
                // 1. always drop the selected result off
                m_SelectState = 0;
                TBLocation stCursorLoc = {nY, nX - (bFirstHalf ? 1 : 0)};

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

bool TokenBoard::ProcessEvent(const SDL_Event &rstEvent, bool *bValid)
{
    // don't need to handle event or event has been consumed
    if(m_SkipEvent || (bValid && !(*bValid))){ return false; }

    int nEventDX = -1;
    int nEventDY = -1;

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
                return false;
            }
    }

    // 1. inside the board or not
    if(!In(nEventDX, nEventDY)){ return false; }

    auto fnTokenBoxEventHandle = [this](uint32_t nEventType, int nLocX, int nLocY, bool bFirstHalf)
    {
        switch(nEventType){
            case SDL_MOUSEMOTION:
                {
                    TokenBoxGetMouseMotion(nLocX, nLocY, bFirstHalf);
                    break;
                }
            case SDL_MOUSEBUTTONUP:
                {
                    TokenBoxGetMouseButtonUp(nLocX, nLocY, bFirstHalf);
                    break;
                }
            case SDL_MOUSEBUTTONDOWN:
                {
                    TokenBoxGetMouseButtonDown(nLocX, nLocY, bFirstHalf);
                    break;
                }
            default:
                {
                    break;
                }
        }
    };

    // 2. try among all boxes
    int  nTBLocX    = -1;
    int  nTBLocY    = -1;
    bool bFirstHalf = false;

    if(LastTokenBoxValid()){
        auto &rstLastTB = m_LineV[m_LastTokenBoxLoc.Y].Content[m_LastTokenBoxLoc.X];
        // W1 and W2 should also count in
        // otherwise mouse can escape from the flaw of two contiguous tokens
        // means when move mouse horizontally, event text will turn on and off

        int nX = rstLastTB.Cache.StartX - rstLastTB.State.W1;
        int nY = rstLastTB.Cache.StartY;
        int nW = rstLastTB.Cache.W;
        int nH = rstLastTB.Cache.H;

        if(MathFunc::PointInRectangle(nEventDX, nEventDY, nX, nY, nW, nH)){
            nTBLocX    = m_LastTokenBoxLoc.X;
            nTBLocY    = m_LastTokenBoxLoc.Y;
            bFirstHalf = (nEventDX < nX + nW / 2);
        }
    }

    if(!TokenBoxValid(nTBLocX, nTBLocY)){
        int nGridX = nEventDX / m_Resolution;
        int nGridY = nEventDY / m_Resolution;

        // only put EventText Section in Bitmap
        // emoticon won't accept events

        for(const auto &stLoc: m_TokenBoxBitmap[nGridX][nGridY]){
            if(TokenBoxValid(stLoc.X, stLoc.Y)){
                const auto &rstTokenBox = m_LineV[stLoc.Y].Content[stLoc.X];

                // get the rectangle of the box
                // check if event location is inside it
                int nX = rstTokenBox.Cache.StartX - rstTokenBox.State.W1;
                int nY = rstTokenBox.Cache.StartY;
                int nW = rstTokenBox.Cache.W;
                int nH = rstTokenBox.Cache.H;

                if(MathFunc::PointInRectangle(nEventDX, nEventDY, nX, nY, nW, nH)){
                    nTBLocX    = stLoc.X;
                    nTBLocY    = stLoc.Y;
                    bFirstHalf = (nEventDX < nX + nW / 2);
                    break;
                }
            }
        }
    }

    if(TokenBoxValid(nTBLocX, nTBLocY)){
        // 1. trigger event
        // 2. update m_LastTokenBoxLoc
        fnTokenBoxEventHandle(rstEvent.type, nTBLocX, nTBLocY, bFirstHalf);
        m_LastTokenBoxLoc = {nTBLocX, nTBLocY};

        if(bValid){ *bValid = false; }
        return true;
    }

    // check if we need to handle the event
    // current event is inside the board but not in any boxes

    if(false
            || (m_WithCursor)
            || (m_Selectable && m_SelectState == SELECTTYPE_SELECTING)){

        // 1. can edit
        // 2. can select and is in progress

        // 0. anyway we need to consume it
        if(bValid){ *bValid = false; }

        // 1. decide which line it's inside
        int nRowIn = -1;
        if(!m_LineV.empty() && nEventDY > m_LineV.back().StartY){
            // at the very down part
            // we reset it into the last line
            nRowIn = (int)(m_LineV.size()) - 1;
        }else{
            for(int nRow = 0; nRow < (int)(m_LineV.size()); ++nRow){
                if(m_LineV[nRow].StartY >= nEventDY){
                    // this may cause a problem:
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

        // can't capture this event
        if(nRowIn < 0){ return false; }

        // now try to find nTBLocX for tokenbox binding
        int nTokenBoxBind = -1;
        int nLastCenterX  = -1;
        for(int nXCnt = 0; nXCnt < (int)(m_LineV[nRowIn].Content.size()); ++nXCnt){
            int nX  = m_LineV[nRowIn].Content[nXCnt].Cache.StartX;
            int nW  = m_LineV[nRowIn].Content[nXCnt].Cache.W;
            int nCX = nX + nW / 2;

            if(MathFunc::PointInSegment(nEventDX, nLastCenterX, nCX - nLastCenterX + 1)){
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
            nTokenBoxBind = m_LineV[nRowIn].Content.size();
        }

        fnTokenBoxEventHandle(rstEvent.type, nTokenBoxBind, nRowIn, false);
        return false;
    }
    return true;
}

int TokenBoard::TokenBoxType(const TOKENBOX &rstTokenBox) const
{
    if(SectionValid(rstTokenBox.Section)){
        return m_SectionRecord.at(rstTokenBox.Section).Section.Info.Type;
    }
    return SECTIONTYPE_NONE;
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

    for(int nY = 0; nY < (int)(m_LineV.size()); ++nY){
        for(int nX = 0; nX < (int)(m_LineV[nY].Content.size()); ++nX){
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

    auto &rstTokenBox = m_LineV[nLocY].Content[nLocX];

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

std::string TokenBoard::GetXML(bool bSelectedOnly)
{
    if(Empty(false)){
        return "<root></root>";
    }

    if(bSelectedOnly){
        switch(m_SelectState){
            case SELECTTYPE_DONE:
            case SELECTTYPE_SELECTING:
                {
                    return InnGetXML(m_SelectLoc[0].X, m_SelectLoc[0].Y, m_SelectLoc[1].X, m_SelectLoc[1].Y);
                }
            default:
                {
                    return "<root></root>";
                }
        }
    }else{
        return InnGetXML(0, 0, m_LineV.back().Content.size() - 1, m_LineV.size() - 1);
    }
}

std::string TokenBoard::InnGetXML(int nX0, int nY0, int nX1, int nY1)
{
    if(false
            || (nY0 > nY1)
            || (nY0 == nY1 && nX0 > nX1)

            || !TokenBoxValid(nX0, nY0)
            || !TokenBoxValid(nX1, nY1)){ return "<root></root>"; }

    int nX = nX0;
    int nY = nY0;
    std::string szXML = "<root>";

    int nLastSection = -1;
    while(!(nX == nX1 && nY == nY1)){
        const auto &rstTB = m_LineV[nY].Content[nX];
        const auto &rstSN = m_SectionRecord[rstTB.Section].Section;

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

                        // TODO
                        // 1. we won't parse ID here
                        // 2. didn't parse style yet
                        // 3. didn't parse color set yet

                        szXML += "<object type=eventtext font=";
                        szXML += std::to_string(rstSN.Info.Text.Font);
                        szXML += " size=";
                        szXML += std::to_string(rstSN.Info.Text.Size);
                        szXML += ">";
                    }

                    // truncate to get last 32 bits
                    uint32_t nUTF8Code[2] = {(uint32_t)(rstTB.UTF8CharBox.Cache.Key), 0};
                    szXML += (const char *)(nUTF8Code);
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
                        std::sprintf(szColor, "0x%08x", ColorFunc::RGBA2ARGB(ColorFunc::Color2RGBA(rstSN.Info.Text.Color[0])));
                        szXML += " color=";
                        szXML += szColor;
                        szXML += ">";
                    }

                    // truncate to get last 32 bits
                    uint32_t nUTF8Code[2] = {(uint32_t)(rstTB.UTF8CharBox.Cache.Key), 0};
                    szXML += (const char *)(nUTF8Code);
                    break;
                }
            default:
                {
                    break;
                }
        }

        nLastSection = rstTB.Section;

        nX++;
        if(nX == (int)(m_LineV[nY].Content.size())){
            nX = 0;
            nY++;
        }
    }
    return szXML;
}

// Insert a bunch of tokenboxes before the cursor, cursor will move to
// the end of the inserted content after this function. This function
// won't introduce any new <CR>
//
// Assume:
//      1. the tokenboard is valid
//      2. the section info has already been set, actually this function
//         always works as a second half of insertion
//
// HowTo:
//      1. if outside the text block, insert at the beginning or append
//         at the very end
//      2. if current line can hold the new box, just insert it
//      3. if can't
//          1. if current line ends with <CR>, create a new line ends
//             with return and contain all needed tokens, insert this 
//             line next to current line. make current line and the new
//             line both end with return
//          2. if not, move token at the end of current line and insert
//             it at the beginning of next line, until current line has
//             enough space to hold the new token
//  return:
//      1. true most likely
//      2. false with warning when current token is tooooooo wide that a
//         whole line even can't hold it
bool TokenBoard::AddTokenBoxLine(const std::vector<TOKENBOX> &rstTBV)
{
    if(rstTBV.empty()){ return true; }
    if(!CursorValid()){ Reset(); }

    int nX = m_CursorLoc.X;
    int nY = m_CursorLoc.Y;

    m_LineV[nY].Content.insert(m_LineV[nY].Content.begin() + nX, rstTBV.begin(), rstTBV.end());

    // easy case, no padding needed
    // only need to reset current line and all rest
    if(m_MaxLineWidth <= 0){
        ResetLine(nY);
        m_CursorLoc.X += (int)(rstTBV.size());
        return true;
    }

    // we need to wrap the text
    // count the tokens, to put proper tokens in current line
    int nCount = -1;
    int nWidth =  0;
    for(int nIndex = 0; nIndex < (int)(m_LineV[nY].Content.size()); ++nIndex){
        nWidth += m_LineV[nY].Content[nIndex].Cache.W;
        if(nWidth > m_MaxLineWidth){
            nCount = nIndex;
            break;
        }
        nWidth += m_WordSpace;
    }

    if(nCount == 0){
        g_Log->AddLog(LOGTYPE_WARNING, "Big token box for current board: Token::W = %d, PW = %d", m_LineV[nY].Content[0].Cache.W, m_MaxLineWidth);
        return false;
    }

    if(nCount == -1){
        // nCount didn't set, mean it can hold the whole v
        // don't need to break current line
        ResetLine(nY);
        m_CursorLoc.X += (int)(rstTBV.size());
        return true;
    }

    // can't hold so many tokens in current line
    // backup the token need to put into next line

    std::vector<TOKENBOX> stRestTBV {m_LineV[nY].Content.begin() + nCount, m_LineV[nY].Content.end()};
    m_LineV[nY].Content.resize(nCount);

    // no matter current ends up with CR or not
    // now we need to make current line not-ends-up-with-CR
    bool bCurrEndWithCR = m_LineV[nY].EndWithCR;
    m_LineV[nY].EndWithCR = false;

    // reset current line, do padding, align, etc.
    ResetLine(nY);

    // now the tokenboard is valid again, and we put the rest to the next line
    if(bCurrEndWithCR){
        // 1. when curent line ends up with CR, we put a new line next to it
        m_LineV.insert(m_LineV.begin() + nY + 1, {});
        m_LineV[nY + 1].StartY    = -1;
        m_LineV[nY + 1].EndWithCR =  true;
        m_LineV[nY + 1].Content   =  {};

        // now there is a new empty line
        // need to reset this line to make the board to be valid again
        ResetLine(nY + 1);
    }else{
        // 2. we need to make sure current line has a ``next line" since
        //    if current line is the last line, it should end with CR

        if((int)(m_LineV.size()) > (nY + 1)){
            // OK we have next line
            // we can just insert stRestTBV at the beginning of next line
        }else{
            // this is an internal error
            // this is the last line and it's not end with CR

            g_Log->AddLog(LOGTYPE_WARNING, "Last line doesn't end with CR");

            // repair by insert an empty line at the end
            m_LineV.emplace_back();
            m_LineV.back().StartY    = -1;
            m_LineV.back().EndWithCR =  true;
            m_LineV.back().Content   =  {};

            ResetLine(nY + 1);
        }
    }

    // the tokenboard is valid again
    // we prepared an empty line at line (nY + 1)
    m_CursorLoc = {0, nY + 1};

    return AddTokenBoxLine(stRestTBV);
}

bool TokenBoard::QueryTokenBox(int nX, int nY, int *pType, int *pX, int *pY, int *pW, int *pH, int *pW1, int *pW2)
{
    if(TokenBoxValid(nX, nY)){
        auto &rstTB = m_LineV[nY].Content[nX];
        if(SectionValid(rstTB.Section)){
            auto &rstSEC = m_SectionRecord[rstTB.Section].Section;
            if(pType){ *pType = rstSEC.Info.Type;   }
            if(pX   ){ *pX    = rstTB.Cache.StartX; }
            if(pY   ){ *pY    = rstTB.Cache.StartY; }
            if(pW   ){ *pW    = rstTB.Cache.W;      }
            if(pH   ){ *pH    = rstTB.Cache.H;      }
            if(pW1  ){ *pW1   = rstTB.State.W1;     }
            if(pW2  ){ *pW2   = rstTB.State.W2;     }

            return true;
        }
    }
    return false;
}

// remove tokens, for bSelectedOnly
//    1:  only shadow will be deleted, for CTRL-X
//    0:  if there is shadow, remove shadow, else remove, current cursor bound tokenbox
// assumption
//  1. valid board
//  2. after this the cursor with at the first locaiton of the deleted box
bool TokenBoard::Delete(bool bSelectedOnly)
{
    if(Empty(false)){ return true; }

    int nX0 = -1;
    int nY0 = -1;
    int nX1 = -1;
    int nY1 = -1;

    if(bSelectedOnly){
        switch(m_SelectState){
            case SELECTTYPE_NONE:
                {
                    return true;
                }
            case SELECTTYPE_DONE:
            case SELECTTYPE_SELECTING:
                {
                    nX0 = m_SelectLoc[0].X;
                    nY0 = m_SelectLoc[0].Y;
                    nX1 = m_SelectLoc[1].X;
                    nY1 = m_SelectLoc[1].Y;
                    break;
                }
            default:
                {
                    return false;
                }
        }
    }else{
        nX0 = nX1 = m_CursorLoc.X - 1;
        nY0 = nY1 = m_CursorLoc.Y;
    }

    // helper function
    // delete whole lines [nY0, nY1]
    auto fnDeleteLine = [this](int nLine0, int nLine1)
    {
        if(true
                && nLine0 <= nLine1

                && LineValid(nLine0)
                && LineValid(nLine1)){
            m_LineV.erase(m_LineV.begin() + nLine0, m_LineV.begin() + nLine1 + 1);
        }
    };

    // don't use TokenBoxValid()
    // if we are about to delete a blank line
    // then (nX0, nY0) and (nX1, nY1) are invalid

    if(true
            && (nX0 == -1 && LineValid(nY0))
            && (nX1 == -1 && LineValid(nY1))
            && (nY0 >= 1)
            && (nY0 == nY1)){

        // 1. delete line nY0
        fnDeleteLine(nY0, nY0);

        // 2. reset the rest line
        if(LineValid(nY0)){
            ResetLineStartY(nY0);
        }else{
            // ok there is no line nY0 any more
            // we should reset the height of the board
            m_H = GetNewHBasedOnLastLine();
        }

        // 3. reset the cursor and return
        m_CursorLoc = {(int)(m_LineV[nY0 - 1].Content.size()), nY0 - 1};
        return true;
    }

    // not a blank line deletion
    // now we can use TokenBoxValid()

    if(false
            || !TokenBoxValid(nX0, nY0)
            || !TokenBoxValid(nX1, nY1)){
        return false;
    }

    // will delete on line N affect line (N - 1)? YES:
    //    1. current line ends with <CR>
    //    2. last line doesn't ends with <CR>
    //    3. delete all of current line
    // Then we need to put a <CR> at last line and padding it if necessary

    // when delete all content in current line:
    // 1. if last line ends with <CR>, leave a blank line for current line, cursor
    //    is still in current line
    // 2. else, delete the whole current line and move cursor at the end of last line

    // modify line (nY0 - 1) if necessary to make lines [0, (nY0 - 1)] well-prepared
    bool bAboveLineAdjusted = false;
    if(true
            &&  m_SpacePadding                                //
            &&  m_MaxLineWidth > 0                                      // system asks for padding
            &&  nY0 >= 1                                      // nY0 is not the first line
            &&  nX0 == 0                                      // delete start from the very beginning
            &&  nX1 == (int)(m_LineV[nY1].Content.size()) - 1 // delete to the very end
            &&  m_LineV[nY1    ].EndWithCR                    // current line ends with <CR>
            && !m_LineV[nY0 - 1].EndWithCR){                  // last line is not ends with <CR>

        m_LineV[nY0 - 1].EndWithCR = true;
        ResetOneLine(nY0 - 1);

        // keep a record to decide do I need to keep the blank line
        bAboveLineAdjusted = true;
    }

    // now (0 ~ (nY0 - 1)) are well-prepared for this delete

    // 1. line nY1 ends with <CR>
    //      1. store pieces from line nY1 in one TBV
    //      2. delete line (nY0 + 1) ~ nY1
    //      3. resize line nY0
    //      4. change line nY0 ends with <CR>
    //      5. reset line nY0 (all) to make board valid
    //      6. insert TBV at the end of nY0
    // 2. line nY1 ends without <CR>:
    //      1. take pieces from line nY0 and line nY1 and store in one TBV
    //      2. delete line (nY0 ~ nY1), then line nY1 + 1 becomes nY0
    //      3. reset line nY0 (only startY) to make current board valid
    //      4. insert the stored TBV at (0, nY0)

    // so if we decide to remove all content for line (nY0, nY1), if it's
    // case 1, we got a blank line, we need to decide to keep it or not

    // delete all whole lines, erase the line and decide whether I should
    // keep a blank line here
    if(true
            && nX0 == 0
            && nX1 == (int)(m_LineV[nY1].Content.size() - 1)){

        if(bAboveLineAdjusted){
            // would NOT leave a blank line
            fnDeleteLine(nY0, nY1);

            // after we remove line nY0 ~ nY1, then there may or may not
            // be a new ``line nY0", if YES, we do reset the start Y for
            // nY0 to the end

            if(nY0 < (int)(m_LineV.size())){
                // this will reset the H
                // so don't call GetNewHBasedOnLastLine() here again
                ResetLineStartY(nY0);
            }else{
                // else don't for reset H
                m_H = GetNewHBasedOnLastLine();
            }

            // reset the cursor to the last line
            m_CursorLoc = {(int)(m_LineV[nY0 - 1].Content.size()), nY0 - 1};
        }else{
            // we should leave a blank line
            // this can be handled by case-1, but we do it here
            fnDeleteLine(nY0 + 1, nY1);
            m_LineV[nY0].Content.clear();

            ResetLine(nY0);
            m_CursorLoc = {0, nY0};
        }
        return true;
    }

    // delete part of line nY0 / nY1
    // have to call AddTokenBoxLine() to reform the board

    bool bRes = 0; 
    if(m_LineV[nY1].EndWithCR){
        std::vector<TOKENBOX> stTBV(m_LineV[nY1].Content.begin() + nX1 + 1, m_LineV[nY1].Content.end());
        fnDeleteLine(nY0 + 1, nY1);

        m_LineV[nY0].Content.resize(nX0);
        m_LineV[nY0].EndWithCR = true;

        ResetLine(nY0);
        m_CursorLoc = {nX0, nY0};
        bRes = AddTokenBoxLine(stTBV);
    }else{
        std::vector<TOKENBOX> stTBV;
        stTBV.insert(stTBV.end(), m_LineV[nY0].Content.begin(), m_LineV[nY0].Content.begin() + nX0);
        stTBV.insert(stTBV.end(), m_LineV[nY1].Content.begin() + nX1 + 1, m_LineV[nY1].Content.end());

        fnDeleteLine(nY0, nY1);
        ResetLineStartY(nY0);

        m_CursorLoc = {0, nY0};
        bRes = AddTokenBoxLine(stTBV);
    }

    // after insertion we reset the cursor
    m_CursorLoc = {nX0, nY0};
    return bRes;
}

// bring board into an empty and read state
// this function works if internal error occurred or for initialization
//
// 1. delete all previous content in current board
// 2. create line-0 as an empty line
//
// assumption: no
void TokenBoard::Reset()
{
    m_LineV         .clear();
    m_SectionRecord .clear();
    m_TokenBoxBitmap.clear();
    m_SelectRecord  .clear();

    if(m_MaxLineWidth > 0){
        m_W = m_Margin[1] + m_MaxLineWidth + m_Margin[3];
    }else{
        m_W = 0;
    }

    m_H           = 0;
    m_SelectState = 0;
    m_SkipEvent   = false;
    m_SkipUpdate  = false;
    m_CursorLoc   = {0, 0};

    m_SelectLoc[0] = {-1, -1};
    m_SelectLoc[1] = {-1, -1};

    // create the first line manually
    m_LineV.emplace_back();
    m_LineV.back().StartY    = 0;
    m_LineV.back().EndWithCR = true;
    m_LineV.back().Content   = {};

    ResetLine(0);
}

// add a <CR> before the cursor
// can create an empty line if cursor is at end of one line
// assmuption:
//   1. the current board is valid
//   2.
// return:
//   a valid board
bool TokenBoard::ParseReturnObject()
{
    if(!CursorValid()){
        // here we won't call Reset()
        // if found invalid we just skip this insertion
        return false;
    }

    int nX = m_CursorLoc.X;
    int nY = m_CursorLoc.Y;

    // 1. decide whether last line will be affected
    if(true
            &&  nX == 0
            &&  nY  > 0
            && !m_LineV[nY - 1]. EndWithCR){

        m_LineV[nY - 1].EndWithCR = true;
        ResetOneLine(nY - 1);
    }

    // now lines [0, (nY - 1)] are valid

    // get the content after the cursor, maybe empty
    std::vector<TOKENBOX> stTBV(m_LineV[nY].Content.begin() + nX, m_LineV[nY].Content.end());
    m_LineV[nY].Content.resize(nX);

    // backup current line end state
    bool bEndWithCR = m_LineV[nY].EndWithCR;

    // 2. reset current line
    //    current line now could be empty
    //
    // since we are inserting return object to current line
    // then we can always do ResetOneLine() for current line since it must get shorter
    m_LineV[nY].EndWithCR = true;
    ResetOneLine(nY);

    // 3. handle following line
    if(bEndWithCR){
        // the original line ends with return, then we insert a new line with CR ended
        // actually if the cursor is at the beginning, then we even don't have to re-padding it

        // insert an new line and copy stTBV into it
        m_LineV.insert(m_LineV.begin() + nY + 1, {{}});
        m_LineV[nY + 1].StartY    = -1;
        m_LineV[nY + 1].EndWithCR =  true;
        m_LineV[nY + 1].Content   =  stTBV;

        ResetOneLine(nY + 1);
        ResetLineStartY(nY + 2);

        m_CursorLoc = {0, nY + 1};
        return true;
    }else{
        m_CursorLoc = {0, nY + 1};
        return AddTokenBoxLine(stTBV);
    }
}

// reset one line for
// 1. do padding if needed
// 2. calculate the cache.startx
// 3. calculate the cache.starty
//
// assmuption:
//      1. Lines [0 ~ (nLine - 1)] are valid
//      2.
//
// return:
//      1. Line [0 ~ nLine] are valid
void TokenBoard::ResetOneLine(int nLine)
{
    if(LineValid(nLine)){
        if(m_LineV[nLine].EndWithCR){
            // ends with CR, do word space padding only
            SetTokenBoxWordSpace(nLine);
        }else{
            // else do full padding: word space and W1-W2 padding
            LinePadding(nLine);
        }

        SetTokenBoxStartX(nLine);

        m_LineV[nLine].StartY = GetNewLineStartY(nLine);
        SetTokenBoxStartY(nLine, m_LineV[nLine].StartY);
    }
}

// reset the StartY from nStartLine to the end
// assumption:
//      1. lines [0, (nStartLine - 1)] are valid 
//      2. if take lines [0, (nStartLine - 1)] out, and reset StartY for [nStartLine, end] as
//         a new board, then the new board is also valid
// return:
//      1. a valid board
//
// most of the time we updated line (nStartLine - 1), reset its StartX/Y and then the first half of
// the board: [0, nStartLine - 1] is valid, but we need to adjust the second part, and which
// only needs recalculate the rest for StartY.
void TokenBoard::ResetLineStartY(int nStartLine)
{
    if(!LineValid(nStartLine)){ return; }

    int nLongestLine  = m_LineV.size() - 1;
    int nLongestLineW = -1;

    // get the longest line
    // can use function LineFullWidth()
    // but by assumption we can directly use cache info
    for(int nIndex = nStartLine; nIndex < (int)(m_LineV.size()); ++nIndex){
        if(!m_LineV[nIndex].Content.empty()){
            const auto &rstTB = m_LineV[nIndex].Content.back();
            int nCurrentWidth = rstTB.Cache.StartX + rstTB.Cache.W + rstTB.State.W2;
            if(nCurrentWidth > nLongestLineW){
                nLongestLineW = nCurrentWidth;
                nLongestLine  = nIndex;
            }
        }
    }

    int nOldLongestLineStartY = m_LineV[nLongestLine].StartY;
    for(int nIndex = nStartLine; nIndex < (int)(m_LineV.size()) && nIndex <= nLongestLine; ++nIndex){
        m_LineV[nIndex].StartY = GetNewLineStartY(nIndex);
        SetTokenBoxStartY(nIndex, m_LineV[nIndex].StartY);
    }

    int nDStartY = m_LineV[nLongestLine].StartY - nOldLongestLineStartY;
    for(int nIndex = nLongestLine + 1; nIndex < (int)(m_LineV.size()); ++nIndex){
        m_LineV[nIndex].StartY += nDStartY;
    }

    // don't forget to update the height
    m_H = GetNewHBasedOnLastLine();
}

// insert an utf-8 char box to current section
// if current section is not event/plain text then create one
// assume:
//      1. valid tokenboard
bool TokenBoard::AddUTF8Code(uint32_t nUTF8Code)
{
    TOKENBOX stTB;
    std::memset(&stTB, 0, sizeof(stTB));

    if(!CursorValid()){ return false; }

    // 1. get the current section ID
    auto fnGetTextSection = [this](int nTBX, int nTBY) -> int
    {
        if(TokenBoxValid(nTBX, nTBY)){
            int nID = m_LineV[nTBY].Content[nTBX].Section;
            if(!SectionValid(nID, false)){
                g_Log->AddLog(LOGTYPE_WARNING, "Section ID %d not found for token box (%d, %d).", nID, nTBX, nTBY);
                return -1;
            }

            switch(m_SectionRecord[nID].Section.Info.Type){
                case SECTIONTYPE_EVENTTEXT:
                case SECTIONTYPE_PLAINTEXT:
                    {
                        return nID;
                    }
                default:
                    {
                        break;
                    }
            }
        }
        return -1;
    };

    // 1. try the left/right box's section ID
    int nSectionID = -1;
    if(!SectionValid(nSectionID)){ nSectionID = fnGetTextSection(m_CursorLoc.X - 1, m_CursorLoc.Y); }
    if(!SectionValid(nSectionID)){ nSectionID = fnGetTextSection(m_CursorLoc.X,     m_CursorLoc.Y); }

    // 2. left and right are neither valid
    //    need to create a new one
    //    we can call ParseTextObject() instead for this case
    if(nSectionID < 0){
        SECTION stSection;
        std::memset(&stSection, 0, sizeof(stSection));

        stSection.Info.Type          = SECTIONTYPE_PLAINTEXT;
        stSection.Info.Text.Font     = m_DefaultFont;
        stSection.Info.Text.Size     = m_DefaultSize;
        stSection.Info.Text.Style    = m_DefaultStyle;
        stSection.Info.Text.Color[0] = m_DefaultColor;

        nSectionID = CreateSection(stSection);
    }

    if(nSectionID < 0){
        // no hope, just give up
        g_Log->AddLog(LOGTYPE_WARNING, "Can't find a proper section for current insertion");
        return false;
    }

    TOKENBOX stTokenBox;
    if(MakeTokenBox(nSectionID, nUTF8Code, &stTokenBox)){
        return AddTokenBoxLine({stTokenBox});
    }

    return false;
}

// insert an utf-8 string to current section
// if current section is not event/plain text then create one
// assume:
//      1. valid tokenboard
bool TokenBoard::AddUTF8Text(const char *pText)
{
    if(pText){
        auto pStart = pText;
        auto pEnd   = pText;

        while(*pEnd != '\0'){
            pStart = pEnd;
            utf8::unchecked::advance(pEnd, 1);

            // should be true
            condcheck(pEnd - pStart <= 4);

            uint32_t nUTF8Key = 0;
            std::memcpy(&nUTF8Key, pStart, pEnd - pStart);

            AddUTF8Code(nUTF8Key);
        }
    }
    return false;
}

void TokenBoard::QueryDefaultFont(uint8_t *pFont, uint8_t *pFontSize, uint8_t *pFontStyle)
{
    if(pFont     ){ *pFont      = m_DefaultFont; }
    if(pFontSize ){ *pFontSize  = m_DefaultSize; }
    if(pFontStyle){ *pFontStyle = m_DefaultStyle; }
}

void TokenBoard::DeleteEmptyBottomLine()
{
    if(m_LineV.empty()){ return; }

    while(!m_LineV.empty()){
        if(m_LineV.back().Content.empty()){
            m_LineV.pop_back();
        }else{
            break;
        }
    }

    if(!m_LineV.empty()){
        m_LineV.back().EndWithCR = true;
        ResetLine(m_LineV.size() - 1);
    }
}

// make a token box based on the section id
// assume
//      1. a SECTION w.r.t nSectionID is already present in m_SectionRecord and well-inited
//      2. all content can be specified by a uint32_t
//      3. can be used to check nSectionID is valid or not
bool TokenBoard::MakeTokenBox(int nSectionID, uint32_t nKey, TOKENBOX *pTokenBox)
{
    if(!SectionValid(nSectionID)){ return false; }
    if(!pTokenBox){ return true; }

    std::memset(pTokenBox, 0, sizeof(*pTokenBox));
    const auto &rstSection = m_SectionRecord[nSectionID].Section;
    switch(rstSection.Info.Type){
        case SECTIONTYPE_PLAINTEXT:
        case SECTIONTYPE_EVENTTEXT:
            {
                // 1. set all attributes
                uint32_t nFontAttrKey = 0
                    + (((uint32_t)rstSection.Info.Text.Font)  << 16)
                    + (((uint32_t)rstSection.Info.Text.Size)  <<  8)
                    + (((uint32_t)rstSection.Info.Text.Style) <<  0);

                pTokenBox->Section = nSectionID;
                pTokenBox->UTF8CharBox.UTF8Code  = nKey;
                pTokenBox->UTF8CharBox.Cache.Key = (((uint64_t)nFontAttrKey << 32) + nKey);

                // 2. set size cache
                auto pTexture = g_FontexDB->Retrieve(pTokenBox->UTF8CharBox.Cache.Key);
                if(pTexture){
                    SDL_QueryTexture(pTexture, nullptr, nullptr, &(pTokenBox->Cache.W), &(pTokenBox->Cache.H));
                    pTokenBox->Cache.H1    = pTokenBox->Cache.H;
                    pTokenBox->Cache.H2    = 0;
                    pTokenBox->State.Valid = 1;
                }else{
                    // failed to retrieve, set a []
                    pTokenBox->Cache.W  = m_DefaultSize;
                    pTokenBox->Cache.H  = m_DefaultSize;
                    pTokenBox->Cache.H1 = m_DefaultSize;
                    pTokenBox->Cache.H2 = 0;
                }
                return true;
            }
        case SECTIONTYPE_EMOTICON:
            {
                pTokenBox->Section = nSectionID;
                auto pTexture = g_EmoticonDB->Retrieve(
                        rstSection.Info.Emoticon.Set,         // emoticon set
                        rstSection.Info.Emoticon.Index,       // emoticon index
                        0,                                    // first frame of the emoticon
                        nullptr,                              // don't need the start info here
                        nullptr,                              // don't need the satrt info here
                        &(pTokenBox->Cache.W),
                        &(pTokenBox->Cache.H),
                        nullptr,
                        &(pTokenBox->Cache.H1),
                        nullptr);

                if(pTexture){
                    // this emoticon is valid
                    pTokenBox->Cache.H2    = pTokenBox->Cache.H - pTokenBox->Cache.H1;
                    pTokenBox->State.Valid = 1;
                }
                return true;
            }
        default:
            {
                return false;
            }
    }
}

// create an XML to export
// should contain return object and emotion object
// assumption:
//      a valid board
// return:
//      an XML with plain/event text, emoticon, return
std::string TokenBoard::Print(bool bSelectOnly)
{
    XMLObjectList stObjectList;
    if(Empty(false)){
        return stObjectList.Print();
    }

    int nX0 = -1;
    int nY0 = -1;
    int nX1 = -1;
    int nY1 = -1;

    if(bSelectOnly){
        switch(m_SelectState){
            case SELECTTYPE_SELECTING:
            case SELECTTYPE_DONE:
                {
                    break;
                }
            case SELECTTYPE_NONE:
            default:
                {
                    return stObjectList.Print();
                }
        }

        nX0 = m_SelectLoc[0].X;
        nY0 = m_SelectLoc[0].Y;
        nX1 = m_SelectLoc[1].X;
        nY1 = m_SelectLoc[1].Y;
    }else{

        // we know it's not empty then .back() is good
        // we need to print the whole from the first to the last place

        nX0 = 0;
        nY0 = 0;
        nX1 = (int)(m_LineV.back().Content.size() + 0);
        nY1 = (int)(m_LineV               .size() - 1);
    }

    auto fnBeforeEnd = [nX1, nY1](int nX, int nY) -> bool
    {
        return (nY < nY1) || (nY == nY1 && nX <= nX1);
    };

    if(false
            || !fnBeforeEnd(nX0, nY0)
            || !CursorValid(nX0, nY0)
            || !CursorValid(nX1, nY1)){

        g_Log->AddLog(LOGTYPE_WARNING, "Invalid selection location: (%d, %d) -> (%d, %d)", nX0, nY0, nX1, nY1);
        return stObjectList.Print();
    }

    auto fnAddObject = [this](XMLObjectList *pList, const char *szObjectContent, int nObjectType, int nSectionID)
    {
        if(pList){
            switch(nObjectType){
                case OBJECTTYPE_RETURN:
                    {
                        // return type has no content
                        //             has no section id
                        pList->Add({{"Type", "Return"}}, nullptr);
                        break;
                    }
                default:
                    {
                        // for types not a return object
                        // we need to get detailed information from section header

                        if(SectionValid(nSectionID)){
                            const auto &rstSEC = m_SectionRecord[nSectionID].Section;
                            switch(rstSEC.Info.Type){
                                case SECTIONTYPE_EMOTICON:
                                    {
                                        // won't take szObjectContent
                                        // emoticon won't have content in XMLObjectList
                                        pList->Add({
                                                {"Type" , "Emoticon"                                },
                                                {"Set"  , std::to_string(rstSEC.Info.Emoticon.Set)  },
                                                {"Index", std::to_string(rstSEC.Info.Emoticon.Index)}
                                                }, nullptr);
                                        break;
                                    }
                                case SECTIONTYPE_PLAINTEXT:
                                    {
                                        if(true
                                                && szObjectContent
                                                && std::strlen(szObjectContent)){

                                            char szColor[16];
                                            std::sprintf(szColor, "0X%08X", ColorFunc::RGBA2ARGB(ColorFunc::Color2RGBA(rstSEC.Info.Text.Color[0])));
                                            pList->Add({
                                                    {"Type" , "PlainText"                           },
                                                    {"Font" , std::to_string(rstSEC.Info.Text.Font) },
                                                    {"Size" , std::to_string(rstSEC.Info.Text.Size) },
                                                    {"Style", std::to_string(rstSEC.Info.Text.Style)},
                                                    {"Color", std::string(szColor)                  }
                                                    }, szObjectContent);
                                        }
                                        break;
                                    }
                                case SECTIONTYPE_EVENTTEXT:
                                    {
                                        if(true
                                                && szObjectContent
                                                && std::strlen(szObjectContent)){

                                            char szColor[3][16];
                                            for(int nIndex = 0; nIndex < 3; ++nIndex){
                                                std::sprintf(szColor[nIndex], "0X%08X", ColorFunc::RGBA2ARGB(ColorFunc::Color2RGBA(rstSEC.Info.Text.Color[nIndex])));
                                            }
                                            pList->Add({
                                                    {"Type" , "EventText"                           },
                                                    {"Font" , std::to_string(rstSEC.Info.Text.Font) },
                                                    {"Size" , std::to_string(rstSEC.Info.Text.Size) },
                                                    {"Style", std::to_string(rstSEC.Info.Text.Style)},
                                                    {"Off"  , std::string(szColor[0])               },
                                                    {"Over" , std::string(szColor[1])               },
                                                    {"Down" , std::string(szColor[2])               }
                                                    }, szObjectContent);
                                        }
                                        break;
                                    }
                                default:
                                    {
                                        break;
                                    }
                            }
                        }
                        break;
                    }
            }
        }
    };


    int nCurrX = nX0;
    int nCurrY = nY0;

    while(fnBeforeEnd(nCurrX, nCurrY)){
        if(nCurrX == (int)(m_LineV[nCurrY].Content.size())){
            // reach the end
            // need to check if we should insert a return object
            if(m_LineV[nCurrY].EndWithCR){
                fnAddObject(&stObjectList, nullptr, OBJECTTYPE_RETURN, -1);
            }

            nCurrX  = 0;
            nCurrY += 1;
        }else{

            // is a valid token box 
            // every box has a valid section id

            const auto &rstTokenBox = m_LineV[nCurrY].Content[nCurrX];
            if(SectionValid(rstTokenBox.Section)){
                const auto &rstSection = m_SectionRecord[rstTokenBox.Section].Section;
                switch(rstSection.Info.Type){
                    case SECTIONTYPE_EMOTICON:
                        {
                            fnAddObject(&stObjectList, nullptr, OBJECTTYPE_NONE, rstTokenBox.Section);
                            nCurrX++;

                            break;
                        }
                    case SECTIONTYPE_PLAINTEXT:
                    case SECTIONTYPE_EVENTTEXT:
                        {
                            std::string szContent;
                            int nCurrSectionID = rstTokenBox.Section;

                            while(true
                                    && fnBeforeEnd(nCurrX, nCurrY)
                                    && TokenBoxValid(nCurrX, nCurrY)
                                    && m_LineV[nCurrY].Content[nCurrX].Section == nCurrSectionID){

                                // 1. append current utf8 char
                                uint32_t buf32UTF8Code[] = {m_LineV[nCurrY].Content[nCurrX].UTF8CharBox.UTF8Code, 0};
                                szContent += (char *)(buf32UTF8Code);

                                // 2. move to next
                                nCurrX++;

                                // 3. if we reach the end
                                //    we should check if we need to switch to next line
                                if(nCurrX == (int)(m_LineV[nCurrY].Content.size())){
                                    if(m_LineV[nCurrY].EndWithCR){
                                        // ends with CR
                                        // then we need to stop here
                                        // because text object won't contain return object
                                        fnAddObject(&stObjectList, szContent.c_str(), OBJECTTYPE_NONE, nCurrSectionID);
                                        break;
                                    }else{
                                        // not end
                                        // need to switch to next line

                                        nCurrX  = 0;
                                        nCurrY += 1;
                                    }
                                }
                            }

                            // when to break:
                            //   1. reach the end of the section
                            //   2. reach the end of the selection
                            //   3. reach the end of the line, and ends with CR

                            break;
                        }
                    default:
                        {
                            // can't be here
                            // we checked SectionValid()

                            break;
                        }
                }
            }else{

                // current section ID is invalid
                //    1. invalid id
                //    2. invalid type
                // skip this box and alert a warning

                g_Log->AddLog(LOGTYPE_WARNING, "Invalid section ID = %d for token (%d, %d)", rstTokenBox.Section, nCurrX, nCurrY);

                nCurrX++;
            }
        }
    }

    return stObjectList.Print();
}

int TokenBoard::GetLineStartY(int nLine)
{
    if(LineValid(nLine)){
        return m_LineV[nLine].StartY;
    }
    return -1;
}

int TokenBoard::GetBlankLineHeight()
{
    auto pTexture = g_FontexDB->Retrieve(m_DefaultFont, m_DefaultSize, m_DefaultStyle, (int)'H'); 

    int nDefaultH;
    SDL_QueryTexture(pTexture, nullptr, nullptr, nullptr, &nDefaultH);

    return nDefaultH;
}

int TokenBoard::GetNewHBasedOnLastLine()
{
    // actually this could not happen
    // since there is at least one blank line for the board
    if(m_LineV.empty()){
        return m_Margin[0] + m_Margin[2];
    }

    return m_LineV.back().StartY + 1 + (m_LineV.back().Content.empty() ? 0 : GetLineIntervalMaxH2(m_LineV.size() - 1, 0, m_W)) + m_Margin[2];
}

bool TokenBoard::BreakLine()
{
    return ParseReturnObject();
}

int TokenBoard::GetLineMaxH1(int nLine)
{
    if(LineValid(nLine)){
        int nCurrMaxH1 = 0;
        for(auto &rstTokenBox: m_LineV[nLine].Content){
            nCurrMaxH1 = (std::max<int>)(nCurrMaxH1, rstTokenBox.Cache.H1);
        }
    }
    return -1;
}

bool TokenBoard::ParseXML(const char *szXML, const std::map<std::string, std::function<void()>> &rstIDHandleMap)
{
    XMLObjectList stXMLObjectList;
    if(stXMLObjectList.Parse(szXML)){
        return InnInsert(stXMLObjectList, rstIDHandleMap);
    }
    return false;
}

bool TokenBoard::Append(const char *pText)
{
    if(pText){
        std::string szXML;

        szXML += "<root>";
        szXML += "<object type=plaintext>";
        szXML += pText;
        szXML += "</object></root>";

        return AppendXML(szXML.c_str(), {});
    }
    return false;
}

bool TokenBoard::AppendXML(const char *szXML, const std::map<std::string, std::function<void()>> &rstIDHandleMap)
{
    MoveCursorBack();
    return ParseXML(szXML, rstIDHandleMap);
}

bool TokenBoard::Empty(bool bTokenBoxOnly) const
{
    auto fnLineEmpty = [this, bTokenBoxOnly](int nLine) -> bool
    {
        if(LineValid(nLine)){
            return bTokenBoxOnly ? m_LineV[nLine].Content.empty() : false;
        }
        return true;
    };

    for(int nLine = 0; nLine < (int)(m_LineV.size()); ++nLine){
        if(!fnLineEmpty(nLine)){ return false; }
    }
    return true;
}

int TokenBoard::SelectBox(int nX0, int nY0, int nX1, int nY1, const SDL_Color &rstFontColor, const SDL_Color &rstBackColor)
{
    if(true
            && TokenBoxValid(nX0, nY0)
            && TokenBoxValid(nX1, nY1)

            // check location
            // reject invalid location or empty location
            && ((nY0 < nY1) || ((nY0 == nY1) && (nX0 < nX1)))){

        int nSelectID = 0;
        while(nSelectID <= std::numeric_limits<int>::max()){
            if(m_SelectRecord.find(nSelectID) == m_SelectRecord.end()){
                SelectRecord stRecord;
                stRecord.X0 = nX0;
                stRecord.Y0 = nY0;
                stRecord.X1 = nX1;
                stRecord.Y1 = nY1;

                stRecord.Color[0] = rstFontColor;
                stRecord.Color[1] = rstBackColor;

                m_SelectRecord[nSelectID] = stRecord;
                return nSelectID;
            }
            nSelectID++;
        }
    }
    return -1;
}

// remove whole lines, will adjust (startLine, lineCount)
// assumption
//      1. valid board
//      2. after this the cursor could be relocated
// return:
//      a valid board
// we can select lines to be selected and then call Delete()
// but I use this more direct way
void TokenBoard::RemoveLine(int nStartLine, int nLineCount)
{
    nStartLine = (std::max<int>)(nStartLine, 0);
    nLineCount = (std::min<int>)(nLineCount, GetLineCount() - nStartLine);

    m_LineV.erase(m_LineV.begin() + nStartLine, m_LineV.begin() + nStartLine + nLineCount);
    ResetLine(nStartLine);

    // adjust cursor
    if(m_CursorLoc.X < nStartLine){
        // do nothing
        // we keep the old cursor location
    }else if(m_CursorLoc.X >= nStartLine + nLineCount){
        m_CursorLoc.X -= nLineCount;
    }else{
        // since we assume it's a valid board
        // here cursor could only be inside the delete lines
        m_CursorLoc = {nStartLine, 0};
    }
}
