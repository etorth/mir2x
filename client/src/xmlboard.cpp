/*
 * =====================================================================================
 *
 *       Filename: xmlboard.cpp
 *        Created: 12/11/2018 04:44:07
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
#include "lalign.hpp"
#include "xmlboard.hpp"
#include "utf8func.hpp"
#include "mathfunc.hpp"
#include "fontexdbn.hpp"
#include "sdldevice.hpp"

extern Log *g_Log;
extern SDLDevice *g_SDLDevice;
extern FontexDBN *g_FontexDBN;

void XMLBoard::ResetLine(size_t)
{
}

void XMLBoard::SetTokenBoxWordSpace(size_t nLine)
{
    if(!LineValid(nLine)){
        throw std::invalid_argument(str_fflprintf(": Invalid line: %zu", nLine));
    }

    size_t nW1 = m_WordSpace / 2;
    size_t nW2 = m_WordSpace - nW1;

    for(auto &rstToken: m_LineList[nLine].Content){
        rstToken.Box.State.W1 = nW1;
        rstToken.Box.State.W2 = nW2;
    }

    if(!m_LineList[nLine].Content.empty()){
        m_LineList[nLine].Content[0]    .Box.State.W1 = 0;
        m_LineList[nLine].Content.back().Box.State.W2 = 0;
    }
}

// get line full width, count everything inside
// assume:
//      token W/W1/W2 has been set
// return:
//      full line width
size_t XMLBoard::LineFullWidth(size_t nLine) const
{
    if(!LineValid(nLine)){
        throw std::invalid_argument(str_fflprintf(": Invalid line: %zu", nLine));
    }

    size_t nWidth = 0;
    for(size_t nIndex = 0; nIndex < LineTokenCount(nLine); ++nIndex){
        auto pToken = GetToken(nIndex, nLine);
        nWidth += (pToken->Box.State.W1 + pToken->Box.Info.W + pToken->Box.State.W2);
    }
    return nWidth;
}

// calculate line width without W1/W2
// assume:
//      tokens are in current line
//      tokens have W initialized
// return:
//      total token width, plus word space optionally
//      negative if error
// this function is used before we padding all tokens, to estimate how many pixels we need for current line
size_t XMLBoard::LineRawWidth(size_t nLine, bool bWithWordSpace) const
{
    if(!LineValid(nLine)){
        throw std::invalid_argument(str_fflprintf(": Invalid line: %zu", nLine));
    }

    switch(LineTokenCount(nLine)){
        case 0:
            {
                return 0;
            }
        case 1:
            {
                return GetToken(0, nLine)->Box.Info.W;
            }
        default:
            {
                // for more than one tokens
                // we need to check word spacing

                size_t nWidth = 0;
                for(size_t nX = 0; nX < LineTokenCount(nLine); ++nX){
                    nWidth += GetToken(nX, nLine)->Box.Info.W;
                    if(bWithWordSpace){
                        nWidth += GetTokenWordSpace(nX, nLine);
                    }
                }

                if(bWithWordSpace){
                    size_t nWordSpaceFirst = GetTokenWordSpace(0, nLine);
                    size_t nWordSpaceLast  = GetTokenWordSpace(LineTokenCount(nLine) - 1, nLine);

                    nWidth -= (nWordSpaceFirst / 2);
                    nWidth -= (nWordSpaceLast - nWordSpaceLast / 2);
                }

                return nWidth;
            }
    }
}

bool XMLBoard::AddRawToken(size_t nLine, const TOKEN &rstToken)
{
    if(!LineValid(nLine)){
        throw std::invalid_argument(str_fflprintf(": Invalid line: %zu", nLine));
    }

    if(m_MaxLineWidth == 0){
        m_LineList[nLine].Content.push_back(rstToken);
        return true;
    }

    // if we have a defined width but too small
    // need to accept but give warnings

    if(m_MaxLineWidth < rstToken.Box.Info.W && LineTokenCount(nLine) == 0){
        g_Log->AddLog(LOGTYPE_WARNING, "XMLBoard width is too small to hold the token: (%d < %d)", (int)(m_MaxLineWidth), (int)(rstToken.Box.Info.W));
        m_LineList[nLine].Content.push_back(rstToken);
        return true;
    }

    if((LineRawWidth(nLine, true) + rstToken.Box.Info.W) > m_MaxLineWidth){
        return false;
    }

    m_LineList[nLine].Content.push_back(rstToken);
    return true;
}

void XMLBoard::LinePadding(size_t nLine)
{
    if(!LineValid(nLine)){
        throw std::invalid_argument(str_fflprintf(": Invalid line: %zu", nLine));
    }
}

void XMLBoard::LineDistributedPadding(size_t)
{
}

// do justify padding for current line
// assume:
//      1. alreayd put all tokens into current line
//      2. token has Box.W ready
void XMLBoard::LineJustifyPadding(size_t nLine)
{
    if(!LineValid(nLine)){
        throw std::invalid_argument(str_fflprintf(": Invalid line: %zu", nLine));
    }

    if(LAlign() != LALIGN_JUSTIFY){
        throw std::invalid_argument(str_fflprintf(": Do line justify-padding while board align is configured as: %d", LAlign()));
    }

    switch(LineTokenCount(nLine)){
        case 0:
            {
                throw std::runtime_error(str_fflprintf(": Empty line detected: %zu", nLine));
            }
        case 1:
            {
                return;
            }
        default:
            {
                break;
            }
    }

    if(MaxLineWidth() == 0){
        throw std::invalid_argument(str_fflprintf(": Do line justify-padding while board is configured as infinite single line mode"));
    }

    // we allow to exceeds the line limitation..
    // when there is a huge token inderted to current line, but only for this exception

    if((LineRawWidth(nLine, true) > MaxLineWidth()) && (LineTokenCount(nLine) > 1)){
        throw std::invalid_argument(str_fflprintf(": Line raw width exceeds the fixed max line width: %zu", MaxLineWidth()));
    }

    auto fnLeafTypePadding = [this, nLine](int nLeafTypeMask, double fPaddingRatio) -> size_t
    {
        while(LineFullWidth(nLine) < MaxLineWidth()){
            size_t nCurrDWidth = MaxLineWidth() - LineFullWidth(nLine);
            size_t nDoneDWidth = nCurrDWidth;
            for(size_t nIndex = 0; nIndex < LineTokenCount(nLine); ++nIndex){
                auto pToken = GetToken(nIndex, nLine);
                if(m_Paragraph.LeafRef(pToken->Leaf).Type() & nLeafTypeMask){
                    if((fPaddingRatio >= 0.0) && (pToken->Box.State.W1 + pToken->Box.State.W2) >= (int)(std::lround(pToken->Box.Info.W * fPaddingRatio))){
                        continue;
                    }

                    if(pToken->Box.State.W1 <= pToken->Box.State.W2){
                        if(nIndex != 0){
                            pToken->Box.State.W1++;
                            nDoneDWidth--;

                            if(nDoneDWidth == 0){
                                return MaxLineWidth();
                            }
                        }
                    }else{
                        if(nIndex != LineTokenCount(nLine) - 1){
                            pToken->Box.State.W2++;
                            nDoneDWidth--;

                            if(nDoneDWidth == 0){
                                return MaxLineWidth();
                            }
                        }
                    }
                }
            }

            // can't help
            // we go through the whole line but no width changed
            if(nCurrDWidth == nDoneDWidth){
                return LineFullWidth(nLine);
            }
        }

        return MaxLineWidth();
    };

    if(fnLeafTypePadding(LEAF_IMAGE | LEAF_EMOJI, 0.2)){
        return;
    }

    if(fnLeafTypePadding(LEAF_UTF8GROUP, -1.0) == MaxLineWidth()){
        return;
    }

    throw std::runtime_error(str_fflprintf(": Can't do padding to width: %zu", MaxLineWidth()));
}

void XMLBoard::ResetOneLine(size_t nLine, bool bCREnd)
{
    if(!LineValid(nLine)){
        throw std::invalid_argument(str_fflprintf(": Invalid line: %zu", nLine));
    }

    // some tokens may have non-zero W1/W2 when reach here
    // need to reset them all
    for(size_t nIndex = 0; nIndex < LineTokenCount(nLine); ++nIndex){
        GetToken(nIndex, nLine)->Box.State.W1 = 0;
        GetToken(nIndex, nLine)->Box.State.W2 = 0;
    }

    switch(LAlign()){
        case LALIGN_JUSTIFY:
            {
                if(bCREnd){
                    LineJustifyPadding(nLine);
                }
                break;
            }
        case LALIGN_DISTRIBUTED:
            {
                LineDistributedPadding(nLine);
                break;
            }
    }

    SetLineTokenStartX(nLine);
    SetLineTokenStartY(nLine);
}

void XMLBoard::SetLineTokenStartX(size_t nLine)
{
    if(!LineValid(nLine)){
        throw std::invalid_argument(str_fflprintf(": Invalid line: %zu", nLine));
    }

    size_t nLineStartX = 0;
    switch(LAlign()){
        case LALIGN_RIGHT:
            {
                nLineStartX = MaxLineWidth() - LineFullWidth(nLine);
                break;
            }
        case LALIGN_CENTER:
            {
                nLineStartX = (MaxLineWidth() - LineFullWidth(nLine)) / 2;
                break;
            }
    }

    size_t nCurrX = nLineStartX;
    for(auto &rstToken: m_LineList[nLine].Content){
        nCurrX += rstToken.Box.State.W1;
        rstToken.Box.State.X = nCurrX;

        nCurrX += rstToken.Box.Info.W;
        nCurrX += rstToken.Box.State.W2;
    }
}

// get max H2 for an interval
// assume:
//      W1/W/W2/X of tokens in nLine is initialized
// return:
//      max H2, or negative if there is no tokens for that interval
int XMLBoard::LineIntervalMaxH2(size_t nLine, size_t nIntervalStartX, size_t nIntervalWidth) const
{
    //    This function only take care of nLine
    //    has nothing to do with (n-1)th or (n+1)th Line
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
    //  Here we take the padding space as part of the token.
    //  If not, token in (n + 1)-th line may go through this space
    //
    //  but token in (n + 1)-th line only count for the interval
    //  W1/W2 won't count for here, as shown

    if(!LineValid(nLine)){
        throw std::invalid_argument(str_fflprintf(": Invalid line: %zu", nLine));
    }

    if(nIntervalWidth == 0){
        throw std::invalid_argument(str_fflprintf(": Zero-length interval provided"));
    }

    int nMaxH2 = -1;
    for(size_t nIndex = 0; nIndex < LineTokenCount(nLine); ++nIndex){
        auto pToken = GetToken(nIndex, nLine);
        int nW  = pToken->Box.Info .W;
        int nX  = pToken->Box.State.X;
        int nW1 = pToken->Box.State.W1;
        int nW2 = pToken->Box.State.W2;
        int nH2 = pToken->Box.State.H2;

        if(MathFunc::IntervalOverlap<int>(nX - nW1, nW1 + nW + nW2, nIntervalStartX, nIntervalWidth)){
            nMaxH2 = std::max(nMaxH2, nH2);
        }
    }

    // maybe the line is not long enough to cover interval
    // in this case we return -1
    return nMaxH2;
}

// get the minmal (compact) possible start Y of a token in nth line
// assume:
//      (n-1)th line is valid if exists
// return:
//      possible minimal Y for current box
// 
// for nth line we try to calculate possible minmal Y for each token then
// take the max-min as the line start Y, here we don't permit tokens in nth
// line go through upper than (n - 1)th baseLine
size_t XMLBoard::LineTokenBestY(size_t nLine, size_t nTokenX, size_t nTokenWidth, size_t nTokenHeight) const
{
    if(!LineValid(nLine)){
        throw std::invalid_argument(str_fflprintf(": Invalid line: %zu", nLine));
    }

    if(nTokenWidth <= 0 || nTokenHeight <= 0){
        throw std::invalid_argument(str_fflprintf(": Invalid token size: width = %zu, height = %zu", nTokenWidth, nTokenHeight));
    }

    if(nLine == 0){
        return nTokenHeight - 1;
    }

    if(int nMaxH2 = LineIntervalMaxH2(nLine - 1, nTokenX, nTokenWidth); nMaxH2 >= 0){
        return m_LineList[nLine - 1].StartY + (size_t)(nMaxH2) + m_LineSpace + nTokenHeight;
    }

    // negative nMaxH2 means the (n - 1)th line is not long enough
    // directly let the token touch the (n - 1)th baseline
    //
    //           +----------+                  
    //           |          |                  
    // +-------+ |          |                   
    // | Words | |   Emoji  |  Words             
    // +-------+-+----------+------------------
    //           |          |   +----------+  
    //           +----------+   |          |
    //                          |   TB0    |                  
    //                +-------+ |          |                   
    //                | Words | |  Emoji   |  Words             
    //                +-------+-+----------+-------           
    //                          |          |                   
    //                          +----------+                  
    return m_LineList[nLine - 1].StartY + m_LineSpace + nTokenHeight;
}

// get start Y of current line
// assume:
//      1. (nth - 1) line is valid
//      2. nLine is padded already, StartX, W/W1/W2 are OK now
size_t XMLBoard::LineNewStartY(size_t nLine)
{
    //           +----------+                  
    //           |          |                  
    // +-------+ |          |                   
    // | Words | |   Emoji  |  Words             
    // +-------+-+----------+------------------
    //           |          |   +----------+  
    //           +----------+   |          |
    //                          |          |                  
    //                +-------+ |          |                   
    //                | Words | |   Emoji  |  Words             
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

    if(!LineValid(nLine)){
        throw std::invalid_argument(str_fflprintf(": Invalid line: %zu", nLine));
    }

    if(nLine == 0){
        return LineMaxHk(0, 1);
    }

    if(!CanThrough()){
        return LineReachMaxY(nLine - 1) + m_LineSpace + LineMaxHk(nLine, 1);
    }

    if(LineTokenCount(nLine) == 0){
        throw std::runtime_error(str_fflprintf(": Empty line detected: %zu", nLine));
    }

    int nCurrentY = -1;
    for(size_t nIndex = 0; nIndex < LineTokenCount(nLine); ++nIndex){
        auto pToken = GetToken(nIndex, nLine);
        int nX  = pToken->Box.State.X;
        int nW  = pToken->Box.Info .W;
        int nH1 = pToken->Box.State.H1;

        // LineTokenBestY() already take m_LineSpace into consideration
        nCurrentY = std::max<int>(nCurrentY, LineTokenBestY(nLine, nX, nW, nH1));
    }

    // not done yet, think about the following situation
    // if we only check boxes in nth line we may make (n - 1)th line go cross
    //
    // +-----------+ +----+
    // |           | |    |
    // +-----------+-+    + ------ (n - 1)th
    //        +----+ |    |
    //        |    | |    |
    //  +--+  |    | |    |
    //  |  |  |    | |    |
    // -+--+--+----+ |    | ------ nth
    //               +----+

    return (size_t)(std::max<int>(nCurrentY, LineReachMaxY(nLine - 1) + 1));
}

void XMLBoard::SetLineTokenStartY(size_t nLine)
{
    if(!LineValid(nLine)){
        throw std::invalid_argument(str_fflprintf(": Invalid line: %zu", nLine));
    }

    m_LineList[nLine].StartY = (size_t)(LineNewStartY(nLine));
    for(size_t nIndex = 0; nIndex < LineTokenCount(nLine); ++nIndex){
        auto pToken = GetToken(nIndex, nLine);
        pToken->Box.State.Y = m_LineList[nLine].StartY - pToken->Box.State.H1;
    }
}

void XMLBoard::CheckDefaultFont() const
{
    uint64_t nU64Key = UTF8Func::BuildU64Key(m_DefaultFont, m_DefaultFontSize, 0, UTF8Func::PeekUTF8Code("0"));
    if(!g_FontexDBN->Retrieve(nU64Key)){
        throw std::runtime_error(str_fflprintf(": Invalid default font: font = %d, fontsize = %d", (int)(m_DefaultFont), (int)(m_DefaultFontSize)));
    }
}

TOKEN XMLBoard::BuildUTF8Token(uint8_t nFont, uint8_t nFontSize, uint8_t nFontStyle, uint32_t nUTF8Code) const
{
    TOKEN stToken;
    std::memset(&(stToken), 0, sizeof(stToken));
    auto nU64Key = UTF8Func::BuildU64Key(nFont, nFontSize, nFontStyle, nUTF8Code);

    if(auto pTexture = g_FontexDBN->Retrieve(nU64Key)){
        int nBoxW = -1;
        int nBoxH = -1;
        if(!SDL_QueryTexture(pTexture, nullptr, nullptr, &nBoxW, &nBoxH)){
            stToken.Box.Info.W      = nBoxW;
            stToken.Box.Info.H      = nBoxH;
            stToken.Box.State.H1    = stToken.Box.Info.H;
            stToken.Box.State.H2    = 0;
            stToken.UTF8Char.U64Key = nU64Key;
            return stToken;
        }
        throw std::runtime_error(str_fflprintf(": SDL_QueryTexture(%p) failed", pTexture));
    }

    nU64Key = UTF8Func::BuildU64Key(m_DefaultFont, m_DefaultFontSize, 0, nUTF8Code);
    if(g_FontexDBN->Retrieve(nU64Key)){
        throw std::invalid_argument(str_fflprintf(": Can't find texture for UTF8: %" PRIX32, nUTF8Code));
    }

    nU64Key = UTF8Func::BuildU64Key(m_DefaultFont, m_DefaultFontSize, nFontStyle, UTF8Func::PeekUTF8Code("0"));
    if(g_FontexDBN->Retrieve(nU64Key)){
        throw std::invalid_argument(str_fflprintf(": Invalid font style: %" PRIX8, nFontStyle));
    }

    // invalid font given
    // use system default font, don't fail it

    nU64Key = UTF8Func::BuildU64Key(m_DefaultFont, m_DefaultFontSize, nFontStyle, nUTF8Code);
    if(auto pTexture = g_FontexDBN->Retrieve(nU64Key)){
        int nBoxW = -1;
        int nBoxH = -1;
        if(!SDL_QueryTexture(pTexture, nullptr, nullptr, &nBoxW, &nBoxH)){
            g_Log->AddLog(LOGTYPE_WARNING, "Fallback to default font: font: %d -> %d, fontsize: %d -> %d", (int)(nFont), (int)(m_DefaultFont), (int)(nFontSize), (int)(m_DefaultFontSize));
            stToken.Box.Info.W      = nBoxW;
            stToken.Box.Info.H      = nBoxH;
            stToken.Box.State.H1    = stToken.Box.Info.H;
            stToken.Box.State.H2    = 0;
            stToken.UTF8Char.U64Key = nU64Key;
            return stToken;
        }
        throw std::runtime_error(str_fflprintf(": SDL_QueryTexture(%p) failed", pTexture));
    }
    throw std::runtime_error(str_fflprintf(": Fallback to default font failed: font: %d -> %d, fontsize: %d -> %d", (int)(nFont), (int)(m_DefaultFont), (int)(nFontSize), (int)(m_DefaultFontSize)));
}

TOKEN XMLBoard::CreateToken(size_t nLeaf, size_t nLeafOff) const
{
    switch(auto &rstLeaf = m_Paragraph.LeafRef(nLeaf); rstLeaf.Type()){
        case LEAF_UTF8GROUP:
            {
                return BuildUTF8Token(0, 10, 0, rstLeaf.PeekUTF8Code(nLeafOff));
            }
        case LEAF_EMOJI:
        case LEAF_IMAGE:
            {
                throw std::runtime_error(str_fflprintf(": Not supported yet"));
            }
        default:
            {
                throw std::runtime_error(str_fflprintf(": Invalid type: %d", rstLeaf.Type()));
            }
    }
}

// rebuild the board from token (nX, nY)
// assumen:
//      (0, 0) ~ PrevTokenLoc(nX, nY) are valid
// output:
//      valid board
// notice:
// this function may get called after any XML update, the (nX, nY) in XMLBoard may be
// invalid but as long as it's valid in XMLParagraph it's well-defined
void XMLBoard::BuildBoard(size_t nX, size_t nY)
{
    for(size_t nLine = 0; nLine < nY; ++nLine){
        if(m_LineList[nLine].Content.empty()){
            throw std::invalid_argument(str_fflprintf(": Line %zu is empty", nLine));
        }
    }

    size_t nLeaf    = 0;
    size_t nLeafOff = 0;

    if(nX || nY){
        auto [nPrevX,    nPrevY      ] = PrevTokenLoc(nX, nY);
        auto [nPrevLeaf, nPrevLeafOff] = TokenLocInXMLParagraph(nPrevX, nPrevY);

        size_t nAdvanced = 0;
        std::tie(nLeaf, nLeafOff, nAdvanced) = m_Paragraph.NextLeafOff(nPrevLeaf, nPrevLeafOff, 1);

        if(nAdvanced == 0){
            ResetOneLine(nPrevY, true);
            ResetBoardPixelRegion();
            return;
        }
    }

    m_LineList.resize(nY + 1);
    m_LineList[nY].StartY = 0;
    m_LineList[nY].Content.resize(nX);

    size_t nAdvanced = 1;
    size_t nCurrLine = nY;

    for(; nAdvanced; std::tie(nLeaf, nLeafOff, nAdvanced) = m_Paragraph.NextLeafOff(nLeaf, nLeafOff, 1)){
        TOKEN stToken = CreateToken(nLeaf, nLeafOff);
        if(AddRawToken(nCurrLine, stToken)){
            continue;
        }

        ResetOneLine(nCurrLine, false);

        nCurrLine++;
        m_LineList.resize(nCurrLine + 1);

        if(!AddRawToken(nCurrLine, stToken)){
            throw std::runtime_error(str_fflprintf(": Insert token to a new line failed: line = %d", (int)(nCurrLine)));
        }
    }

    ResetOneLine(nCurrLine, true);
    ResetBoardPixelRegion();
}

std::tuple<size_t, size_t> XMLBoard::TokenLocInXMLParagraph(size_t nX, size_t nY) const
{
    size_t nStartX = 0;
    size_t nStartY = 0;

    size_t nStartLeaf = GetToken(nX, nY)->Leaf;
    for(size_t nLine = nY; LineValid(nLine); --nLine){
        if(LineTokenCount(nLine) == 0){
            throw std::runtime_error(str_fflprintf(": Invalid empty line: %zu", nLine));
        }

        if(GetToken(0, nLine)->Leaf != nStartLeaf){
            nStartY = nLine;
            break;
        }
    }

    for(size_t nCurrX = 0; nCurrX < LineTokenCount(nStartY); ++nCurrX){
        if(GetToken(nCurrX, nStartY)->Leaf == nStartLeaf){
            nStartX = nCurrX;
            break;
        }
    }

    if(nStartY == nY){
        return {nStartLeaf, nX - nStartX};
    }

    size_t nLeafOff = (LineTokenCount(nStartY) - nStartX) + nX;
    for(size_t nCurrLine = nStartY + 1; nCurrLine < nY; ++nCurrLine){
        nLeafOff += LineTokenCount(nCurrLine);
    }
    return {nStartLeaf, nLeafOff};
}

void XMLBoard::ResetBoardPixelRegion()
{
    if(!LineCount()){
        m_PX = 0;
        m_PY = 0;
        m_PW = 0;
        m_PH = 0;
        return;
    }

    size_t nMaxPX = 0;
    size_t nMaxPY = 0;
    size_t nMinPX = std::numeric_limits<size_t>::max();
    size_t nMinPY = std::numeric_limits<size_t>::max();

    for(size_t nLine = 0; LineValid(nLine); ++nLine){
        if(!LineTokenCount(nLine)){
            throw std::runtime_error(str_fflprintf(": Found empty line in XMLBoard: line = %zu", nLine));
        }

        nMaxPX = std::max<size_t>(nMaxPX, LineReachMaxX(nLine));
        nMaxPY = std::max<size_t>(nMaxPY, LineReachMaxY(nLine));
        nMinPX = std::min<size_t>(nMinPX, LineReachMinX(nLine));
        nMinPY = std::min<size_t>(nMinPY, LineReachMinY(nLine));
    }

    m_PX = nMinPX;
    m_PY = nMinPY;
    m_PW = nMaxPX + 1 - nMinPX;
    m_PH = nMaxPY + 1 - nMinPY;
}

std::tuple<size_t, size_t, size_t> XMLBoard::PrevTokenLoc(size_t nX, size_t nY, size_t) const
{
    if(!TokenLocValid(nX, nY)){
        throw std::invalid_argument(str_fflprintf(": Invalid token location: (%zu, %zu)", nX, nY));
    }

    return {0, 0, 0};
}

std::tuple<size_t, size_t, size_t> XMLBoard::NextTokenLoc(size_t nX, size_t nY, size_t) const
{
    if(!TokenLocValid(nX, nY)){
        throw std::invalid_argument(str_fflprintf(": Invalid token location: (%zu, %zu)", nX, nY));
    }
    return {0, 0, 0};
}

std::tuple<size_t, size_t> XMLBoard::PrevTokenLoc(size_t nX, size_t nY) const
{
    if(!TokenLocValid(nX, nY)){
        throw std::invalid_argument(str_fflprintf(": Invalid token location: (%zu, %zu)", nX, nY));
    }

    if(nX == 0 && nY == 0){
        throw std::invalid_argument(str_fflprintf(": Try find token before (0, 0)"));
    }

    if(nX){
        return {nX - 1, nY};
    }else{
        return {LineTokenCount(nY - 1), nY - 1};
    }
}

std::tuple<size_t, size_t> XMLBoard::NextTokenLoc(size_t nX, size_t nY) const
{
    if(!TokenLocValid(nX, nY)){
        throw std::invalid_argument(str_fflprintf(": Invalid token location: (%zu, %zu)", nX, nY));
    }

    if(nX == 0 && nY == 0){
        throw std::invalid_argument(str_fflprintf(": Try find token location before (0, 0)"));
    }

    if(nX){
        return {nX - 1, nY};
    }else{
        return {LineTokenCount(nY - 1), nY - 1};
    }
}

void XMLBoard::Delete(size_t nX, size_t nY, size_t nTokenCount)
{
    if(!TokenLocValid(nX, nY)){
        throw std::invalid_argument(str_fflprintf(": Invalid location: (X = %zu, Y = %zu)", nX, nY));
    }

    if(nTokenCount == 0){
        return;
    }

    auto [nLeaf, nLeafOff] = TokenLocInXMLParagraph(nX, nY);

    size_t nAdvanced = 0;
    std::tie(std::ignore, std::ignore, nAdvanced) = m_Paragraph.NextLeafOff(nLeaf, nLeafOff, nTokenCount);

    m_Paragraph.Delete(nLeaf, nLeafOff, nAdvanced);
    BuildBoard(nX, nY);
}

void XMLBoard::InsertText(size_t nX, size_t nY, const char *szText)
{
    if(!TokenLocValid(nX, nY)){
        throw std::invalid_argument(str_fflprintf(": Invalid location: (X = %zu, Y = %zu)", nX, nY));
    }

    auto [nLeaf, nLeafOff] = TokenLocInXMLParagraph(nX, nY);
    if(m_Paragraph.LeafRef(nLeaf).Type() == LEAF_UTF8GROUP){
        m_Paragraph.InsertUTF8Char(nLeaf, nLeafOff, szText);
    }
}

void XMLBoard::DrawEx(int nDstX, int nDstY, int nSrcX, int nSrcY, int nSrcW, int nSrcH) const
{
    if(!MathFunc::RectangleOverlap<int>(nSrcX, nSrcY, nSrcW, nSrcH, PX(), PY(), PW(), PH())){
        return;
    }

    int nDstDX = nDstX - nSrcX;
    int nDstDY = nDstY - nSrcY;

    for(size_t nLine = 0; nLine < LineCount(); ++nLine){
        for(size_t nToken = 0; nToken < LineTokenCount(nLine); ++nToken){
            auto pToken = GetToken(nToken, nLine);

            int nX = pToken->Box.State.X;
            int nY = pToken->Box.State.Y;
            int nW = pToken->Box.Info.W;
            int nH = pToken->Box.Info.H;

            if(!MathFunc::RectangleOverlapRegion(nSrcX, nSrcY, nSrcW, nSrcH, &nX, &nY, &nW, &nH)){
                continue;
            }

            auto &stLeaf = m_Paragraph.LeafRef(pToken->Leaf);
            g_SDLDevice->FillRectangle(stLeaf.BGColor().value_or(BGColor()), nX + nDstX, nY + nDstY, nW, nH);

            auto nColor = ColorFunc::GREEN;
            auto nBGColor = stLeaf.BGColor().value_or(BGColor());
            switch(stLeaf.Type()){
                case LEAF_UTF8GROUP:
                    {
                        int nDX = nX - pToken->Box.State.X;
                        int nDY = nY - pToken->Box.State.Y;

                        auto pTexture = g_FontexDBN->Retrieve(pToken->UTF8Char.U64Key);
                        if(pTexture){
                            SDL_SetTextureColorMod(pTexture, ColorFunc::R(nColor), ColorFunc::G(nColor), ColorFunc::B(nColor));
                            g_SDLDevice->DrawTexture(pTexture, nX + nDstDX, nY + nDstDY, nDX, nDY, nW, nH);
                        }else{
                            g_SDLDevice->DrawRectangle(ColorFunc::CompColor(nBGColor), nX + nDstDX, nY + nDstDY, nW, nH);
                        }
                        break;
                    }
                case LEAF_IMAGE:
                    {
                        break;
                    }
                case LEAF_EMOJI:
                    {
                        // int nXOnTex = 0;
                        // int nYOnTex = 0;
                        // if(auto pTexture = g_EmojiDBN->Retrieve(stLeaf.Emoji().U32Key(), stLeaf.Emoji().Frame(), &nXOnTex, &nYOnTex, nullptr, nullptr, nullptr, nullptr, nullptr)){
                        //     g_SDLDevice->DrawTexture(pTexture, nX + nDstDX, nY + nDstDY, nXOnTex + nDX, nYOnTex + nDY, nW, nH);
                        // }else{
                        //     g_SDLDevice->DrawRectangle(ColorFunc::CompColor(nBGColor));
                        // }
                        break;
                    }
            }

        }
    }
}

void XMLBoard::Update(double fMS)
{
    size_t nLeaf     = 0;
    size_t nX        = 0;
    size_t nY        = 0;
    size_t nAdvanced = 1;

    while(nAdvanced && (nLeaf < m_Paragraph.LeafCount())){
        if(m_Paragraph.LeafRef(nLeaf).Type() == LEAF_EMOJI){
            if(auto pToken= GetToken(nX, nY); pToken->Emoji.FPS != 0){
                double fPeroidMS = 1000.0 / pToken->Emoji.FPS;
                double fCurrTick = fMS + pToken->Emoji.Tick;

                auto nAdvancedFrame  = (uint8_t)(fCurrTick / fPeroidMS);
                pToken->Emoji.Tick   = (uint8_t)(fCurrTick - nAdvancedFrame * fPeroidMS);
                pToken->Emoji.Frame += nAdvancedFrame;
            }
        }

        std::tie(nX, nY, nAdvanced) = NextTokenLoc(nX, nY, m_Paragraph.LeafRef(nLeaf).UTF8CharOffRef().size());
        ++nLeaf;
    }
}

std::string XMLBoard::GetText(bool bTextOnly) const
{
    std::string szPlainString;
    for(size_t nIndex = 0; nIndex < m_Paragraph.LeafCount(); ++nIndex){
        switch(auto nType = m_Paragraph.LeafRef(nIndex).Type()){
            case LEAF_UTF8GROUP:
                {
                    szPlainString += m_Paragraph.LeafRef(nIndex).UTF8Text();
                    break;
                }
            case LEAF_IMAGE:
                {
                    if(!bTextOnly){
                        szPlainString += str_printf("\\image{0x%016" PRIx64 "}", m_Paragraph.LeafRef(nIndex).ImageU64Key());
                    }
                    break;
                }
            case LEAF_EMOJI:
                {
                    if(!bTextOnly){
                        szPlainString += str_printf("\\emoji{0x%016" PRIu64 "}", m_Paragraph.LeafRef(nIndex).EmojiU64Key());
                    }
                    break;
                }
            default:
                {
                    throw std::runtime_error(str_fflprintf(": Invalid leaf type: %d", nType));
                }
        }
    }
    return szPlainString;
}

size_t XMLBoard::GetTokenWordSpace(size_t nX, size_t nY) const
{
    if(!TokenLocValid(nX, nY)){
        throw std::invalid_argument(str_fflprintf(": Invalid token location: (%zu, %zu)", nX, nY));
    }
    return m_WordSpace;
}

size_t XMLBoard::LineReachMaxX(size_t nLine) const
{
    if(!LineValid(nLine)){
        throw std::invalid_argument(str_fflprintf(": Invalid line: %zu", nLine));
    }

    if(LineTokenCount(nLine) == 0){
        throw std::runtime_error(str_fflprintf(": Invalie empty line: %zu", nLine));
    }

    auto pToken = GetLineBackToken(nLine);
    return pToken->Box.State.X + pToken->Box.Info.W;
}

size_t XMLBoard::LineReachMaxY(size_t nLine) const
{
    if(!LineValid(nLine)){
        throw std::invalid_argument(str_fflprintf(": Invalid line: %zu", nLine));
    }
    return m_LineList[nLine].StartY + LineMaxHk(nLine, 2);
}

size_t XMLBoard::LineReachMinX(size_t nLine) const
{
    if(!LineValid(nLine)){
        throw std::invalid_argument(str_fflprintf(": Invalid line: %zu", nLine));
    }

    if(LineTokenCount(nLine) == 0){
        throw std::runtime_error(str_fflprintf(": Invalie empty line: %zu", nLine));
    }

    return GetToken(0, nLine)->Box.State.X;
}

size_t XMLBoard::LineReachMinY(size_t nLine) const
{
    if(!LineValid(nLine)){
        throw std::invalid_argument(str_fflprintf(": Invalid line: %zu", nLine));
    }
    return m_LineList[nLine].StartY - LineMaxHk(nLine, 1) + 1;
}

size_t XMLBoard::LineMaxHk(size_t nLine, size_t k) const
{
    // nHk = 1: get MaxH1
    // nHk = 2: get MaxH2

    if(!LineValid(nLine)){
        throw std::invalid_argument(str_fflprintf(": Invalid line: %zu", nLine));
    }

    if(k != 1 && k != 2){
        throw std::invalid_argument(str_fflprintf(": Invalid argument k: %zu", k));
    }

    size_t nCurrMaxHk = 0;
    for(size_t nIndex = 0; nIndex < LineTokenCount(nLine); ++nIndex){
        nCurrMaxHk = std::max<int>(nCurrMaxHk, (k == 1) ? GetToken(nIndex, nLine)->Box.State.H1 : GetToken(nIndex, nLine)->Box.State.H2);
    }
    return nCurrMaxHk;
}

int XMLBoard::LAlign() const
{
    if(MaxLineWidth() == 0){
        return LALIGN_LEFT;
    }
    return m_LAlign;
}
