/*
 * =====================================================================================
 *
 *       Filename: xmltypeset.cpp
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

#include <cinttypes>
#include "log.hpp"
#include "lalign.hpp"
#include "toll.hpp"
#include "fflerror.hpp"
#include "xmltypeset.hpp"
#include "utf8func.hpp"
#include "mathfunc.hpp"
#include "fontexdb.hpp"
#include "sdldevice.hpp"
#include "emoticondb.hpp"

extern Log *g_Log;
extern FontexDB *g_FontexDB;
extern SDLDevice *g_SDLDevice;
extern EmoticonDB *g_EmoticonDB;

void XMLTypeset::SetTokenBoxWordSpace(int nLine)
{
    if(!LineValid(nLine)){
        throw std::invalid_argument(str_fflprintf(": Invalid line: %d", nLine));
    }

    int nW1 = m_WordSpace / 2;
    int nW2 = m_WordSpace - nW1;

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
int XMLTypeset::LineFullWidth(int nLine) const
{
    if(!LineValid(nLine)){
        throw std::invalid_argument(str_fflprintf(": Invalid line: %d", nLine));
    }

    int nWidth = 0;
    for(int nIndex = 0; nIndex < LineTokenCount(nLine); ++nIndex){
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
int XMLTypeset::LineRawWidth(int nLine, bool bWithWordSpace) const
{
    if(!LineValid(nLine)){
        throw std::invalid_argument(str_fflprintf(": Invalid line: %d", nLine));
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

                int nWidth = 0;
                for(int nX = 0; nX < LineTokenCount(nLine); ++nX){
                    nWidth += GetToken(nX, nLine)->Box.Info.W;
                    if(bWithWordSpace){
                        nWidth += GetTokenWordSpace(nX, nLine);
                    }
                }

                if(bWithWordSpace){
                    int nWordSpaceFirst = GetTokenWordSpace(0, nLine);
                    int nWordSpaceLast  = GetTokenWordSpace(LineTokenCount(nLine) - 1, nLine);

                    nWidth -= (nWordSpaceFirst / 2);
                    nWidth -= (nWordSpaceLast - nWordSpaceLast / 2);
                }

                return nWidth;
            }
    }
}

bool XMLTypeset::addRawToken(int nLine, const TOKEN &rstToken)
{
    if(!LineValid(nLine)){
        throw std::invalid_argument(str_fflprintf(": Invalid line: %d", nLine));
    }

    if(m_MaxLineWidth == 0){
        m_LineList[nLine].Content.push_back(rstToken);
        return true;
    }

    // if we have a defined width but too small
    // need to accept but give warnings

    if(m_MaxLineWidth < rstToken.Box.Info.W && LineTokenCount(nLine) == 0){
        g_Log->AddLog(LOGTYPE_WARNING, "XMLTypeset width is too small to hold the token: (%d < %d)", (int)(m_MaxLineWidth), (int)(rstToken.Box.Info.W));
        m_LineList[nLine].Content.push_back(rstToken);
        return true;
    }

    if((LineRawWidth(nLine, true) + rstToken.Box.Info.W) > m_MaxLineWidth){
        return false;
    }

    m_LineList[nLine].Content.push_back(rstToken);
    return true;
}

void XMLTypeset::LinePadding(int nLine)
{
    if(!LineValid(nLine)){
        throw std::invalid_argument(str_fflprintf(": Invalid line: %d", nLine));
    }
}

void XMLTypeset::LineDistributedPadding(int)
{
}

// do justify padding for current line
// assume:
//      1. alreayd put all tokens into current line
//      2. token has Box.W ready
void XMLTypeset::LineJustifyPadding(int nLine)
{
    if(!LineValid(nLine)){
        throw std::invalid_argument(str_fflprintf(": Invalid line: %d", nLine));
    }

    if(LAlign() != LALIGN_JUSTIFY){
        throw std::invalid_argument(str_fflprintf(": Do line justify-padding while board align is configured as: %d", LAlign()));
    }

    switch(LineTokenCount(nLine)){
        case 0:
            {
                throw std::runtime_error(str_fflprintf(": Empty line detected: %d", nLine));
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
        throw std::invalid_argument(str_fflprintf(": Line raw width exceeds the fixed max line width: %d", MaxLineWidth()));
    }

    auto fnLeafTypePadding = [this, nLine](int nLeafTypeMask, double fPaddingRatio) -> int
    {
        while(LineFullWidth(nLine) < MaxLineWidth()){
            int nCurrDWidth = MaxLineWidth() - LineFullWidth(nLine);
            int nDoneDWidth = nCurrDWidth;
            for(int nIndex = 0; nIndex < LineTokenCount(nLine); ++nIndex){
                auto pToken = GetToken(nIndex, nLine);
                if(m_paragraph.leafRef(pToken->Leaf).Type() & nLeafTypeMask){
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

    throw std::runtime_error(str_fflprintf(": Can't do padding to width: %d", MaxLineWidth()));
}

void XMLTypeset::ResetOneLine(int nLine, bool bCREnd)
{
    if(!LineValid(nLine)){
        throw std::invalid_argument(str_fflprintf(": Invalid line: %d", nLine));
    }

    // some tokens may have non-zero W1/W2 when reach here
    // need to reset them all
    for(int nIndex = 0; nIndex < LineTokenCount(nLine); ++nIndex){
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

void XMLTypeset::SetLineTokenStartX(int nLine)
{
    if(!LineValid(nLine)){
        throw std::invalid_argument(str_fflprintf(": Invalid line: %d", nLine));
    }

    int nLineStartX = 0;
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

    int nCurrX = nLineStartX;
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
int XMLTypeset::LineIntervalMaxH2(int nLine, int nIntervalStartX, int nIntervalWidth) const
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
        throw std::invalid_argument(str_fflprintf(": Invalid line: %d", nLine));
    }

    if(nIntervalWidth == 0){
        throw std::invalid_argument(str_fflprintf(": Zero-length interval provided"));
    }

    int nMaxH2 = -1;
    for(int nIndex = 0; nIndex < LineTokenCount(nLine); ++nIndex){
        auto pToken = GetToken(nIndex, nLine);
        int nW  = pToken->Box.Info .W;
        int nX  = pToken->Box.State.X;
        int nW1 = pToken->Box.State.W1;
        int nW2 = pToken->Box.State.W2;
        int nH2 = pToken->Box.State.H2;

        if(MathFunc::IntervalOverlap<int>(nX - nW1, nW1 + nW + nW2, nIntervalStartX, nIntervalWidth)){
            nMaxH2 = (std::max<int>)(nMaxH2, nH2);
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
int XMLTypeset::LineTokenBestY(int nLine, int nTokenX, int nTokenWidth, int nTokenHeight) const
{
    if(!LineValid(nLine)){
        throw std::invalid_argument(str_fflprintf(": Invalid line: %d", nLine));
    }

    if(nTokenWidth <= 0 || nTokenHeight <= 0){
        throw std::invalid_argument(str_fflprintf(": Invalid token size: width = %d, height = %d", nTokenWidth, nTokenHeight));
    }

    if(nLine == 0){
        return nTokenHeight - 1;
    }

    if(int nMaxH2 = LineIntervalMaxH2(nLine - 1, nTokenX, nTokenWidth); nMaxH2 >= 0){
        return m_LineList[nLine - 1].StartY + (int)(nMaxH2) + m_LineSpace + nTokenHeight;
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
int XMLTypeset::LineNewStartY(int nLine)
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
        throw std::invalid_argument(str_fflprintf(": Invalid line: %d", nLine));
    }

    if(nLine == 0){
        return LineMaxHk(0, 1);
    }

    if(!CanThrough()){
        return LineReachMaxY(nLine - 1) + m_LineSpace + LineMaxHk(nLine, 1);
    }

    if(LineTokenCount(nLine) == 0){
        throw std::runtime_error(str_fflprintf(": Empty line detected: %d", nLine));
    }

    int nCurrentY = -1;
    for(int nIndex = 0; nIndex < LineTokenCount(nLine); ++nIndex){
        auto pToken = GetToken(nIndex, nLine);
        int nX  = pToken->Box.State.X;
        int nW  = pToken->Box.Info .W;
        int nH1 = pToken->Box.State.H1;

        // LineTokenBestY() already take m_LineSpace into consideration
        nCurrentY = (std::max<int>)(nCurrentY, LineTokenBestY(nLine, nX, nW, nH1));
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

    return (int)((std::max<int>)(nCurrentY, LineReachMaxY(nLine - 1) + 1));
}

void XMLTypeset::SetLineTokenStartY(int nLine)
{
    if(!LineValid(nLine)){
        throw std::invalid_argument(str_fflprintf(": Invalid line: %d", nLine));
    }

    m_LineList[nLine].StartY = (int)(LineNewStartY(nLine));
    for(int nIndex = 0; nIndex < LineTokenCount(nLine); ++nIndex){
        auto pToken = GetToken(nIndex, nLine);
        pToken->Box.State.Y = m_LineList[nLine].StartY - pToken->Box.State.H1;
    }
}

void XMLTypeset::checkDefaultFont() const
{
    const uint64_t u64key = UTF8Func::BuildU64Key(m_font, m_fontSize, 0, UTF8Func::PeekUTF8Code("0"));
    if(!g_FontexDB->Retrieve(u64key)){
        throw fflerror("invalid default font: font = %d, fontsize = %d", (int)(m_font), (int)(m_fontSize));
    }
}

TOKEN XMLTypeset::BuildUTF8Token(int nLeaf, uint8_t nFont, uint8_t nFontSize, uint8_t nFontStyle, uint32_t nUTF8Code) const
{
    TOKEN stToken;
    std::memset(&(stToken), 0, sizeof(stToken));
    auto nU64Key = UTF8Func::BuildU64Key(nFont, nFontSize, nFontStyle, nUTF8Code);

    stToken.Leaf = nLeaf;
    if(auto pTexture = g_FontexDB->Retrieve(nU64Key)){
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

    nU64Key = UTF8Func::BuildU64Key(m_font, m_fontSize, 0, nUTF8Code);
    if(g_FontexDB->Retrieve(nU64Key)){
        throw std::invalid_argument(str_fflprintf(": Can't find texture for UTF8: %" PRIX32, nUTF8Code));
    }

    nU64Key = UTF8Func::BuildU64Key(m_font, m_fontSize, nFontStyle, UTF8Func::PeekUTF8Code("0"));
    if(g_FontexDB->Retrieve(nU64Key)){
        throw std::invalid_argument(str_fflprintf(": Invalid font style: %" PRIX8, nFontStyle));
    }

    // invalid font given
    // use system default font, don't fail it

    nU64Key = UTF8Func::BuildU64Key(m_font, m_fontSize, nFontStyle, nUTF8Code);
    if(auto pTexture = g_FontexDB->Retrieve(nU64Key)){
        int nBoxW = -1;
        int nBoxH = -1;
        if(!SDL_QueryTexture(pTexture, nullptr, nullptr, &nBoxW, &nBoxH)){
            g_Log->AddLog(LOGTYPE_WARNING, "Fallback to default font: font: %d -> %d, fontsize: %d -> %d", (int)(nFont), (int)(m_font), (int)(nFontSize), (int)(m_fontSize));
            stToken.Box.Info.W      = nBoxW;
            stToken.Box.Info.H      = nBoxH;
            stToken.Box.State.H1    = stToken.Box.Info.H;
            stToken.Box.State.H2    = 0;
            stToken.UTF8Char.U64Key = nU64Key;
            return stToken;
        }
        throw std::runtime_error(str_fflprintf(": SDL_QueryTexture(%p) failed", pTexture));
    }
    throw std::runtime_error(str_fflprintf(": Fallback to default font failed: font: %d -> %d, fontsize: %d -> %d", (int)(nFont), (int)(m_font), (int)(nFontSize), (int)(m_fontSize)));
}

TOKEN XMLTypeset::buildEmojiToken(int leaf, uint32_t emoji) const
{
    TOKEN token;
    std::memset(&(token), 0, sizeof(token));
    token.Leaf = leaf;

    int tokenW     = -1;
    int tokenH     = -1;
    int h1         = -1;
    int fps        = -1;
    int frameCount = -1;

    if(g_EmoticonDB->Retrieve(emoji, 0, 0, &tokenW, &tokenH, &h1, &fps, &frameCount)){
        token.Box.Info.W       = tokenW;
        token.Box.Info.H       = tokenH;
        token.Box.State.H1     = h1;
        token.Box.State.H2     = tokenH - h1;
        token.Emoji.U32Key     = emoji;
        token.Emoji.FPS        = fps;
        token.Emoji.FrameCount = frameCount;
    }
    return token;
}

TOKEN XMLTypeset::CreateToken(int nLeaf, int nLeafOff) const
{
    switch(auto &rstLeaf = m_paragraph.leafRef(nLeaf); rstLeaf.Type()){
        case LEAF_UTF8GROUP:
            {
                auto nFont      = rstLeaf.Font()     .value_or(m_font);
                auto nFontSize  = rstLeaf.FontSize() .value_or(m_fontSize);
                auto nFontStyle = rstLeaf.FontStyle().value_or(m_fontStyle);
                return BuildUTF8Token(nLeaf, nFont, nFontSize, nFontStyle, rstLeaf.PeekUTF8Code(nLeafOff));
            }
        case LEAF_EMOJI:
            {
                return buildEmojiToken(nLeaf, rstLeaf.emojiU32Key());
            }
        case LEAF_IMAGE:
            {
                throw fflerror("not supported yet");
            }
        default:
            {
                throw fflerror("invalid type: %d", rstLeaf.Type());
            }
    }
}

// rebuild the board from token (nX, nY)
// assumen:
//      (0, 0) ~ PrevTokenLoc(nX, nY) are valid
// output:
//      valid board
// notice:
// this function may get called after any XML update, the (nX, nY) in XMLTypeset may be
// invalid but as long as it's valid in XMLParagraph it's well-defined
void XMLTypeset::BuildBoard(int nX, int nY)
{
    for(int nLine = 0; nLine < nY; ++nLine){
        if(m_LineList[nLine].Content.empty()){
            throw std::invalid_argument(str_fflprintf(": Line %d is empty", nLine));
        }
    }

    int nLeaf    = 0;
    int nLeafOff = 0;

    if(nX || nY){
        auto [nPrevX,    nPrevY      ] = PrevTokenLoc(nX, nY);
        auto [nPrevLeaf, nPrevLeafOff] = TokenLocInXMLParagraph(nPrevX, nPrevY);

        int nAdvanced = 0;
        std::tie(nLeaf, nLeafOff, nAdvanced) = m_paragraph.NextLeafOff(nPrevLeaf, nPrevLeafOff, 1);

        if(nAdvanced == 0){
            ResetOneLine(nPrevY, true);
            ResetBoardPixelRegion();
            return;
        }
    }

    m_leaf2TokenLoc.resize(nLeaf);

    m_LineList.resize(nY + 1);
    m_LineList[nY].StartY = 0;
    m_LineList[nY].Content.resize(nX);

    int nAdvanced = 1;
    int nCurrLine = nY;

    for(; nAdvanced; std::tie(nLeaf, nLeafOff, nAdvanced) = m_paragraph.NextLeafOff(nLeaf, nLeafOff, 1)){
        TOKEN stToken = CreateToken(nLeaf, nLeafOff);
        if(addRawToken(nCurrLine, stToken)){
            if(nLeafOff == 0){
                m_leaf2TokenLoc.push_back({m_LineList[nCurrLine].Content.size() - 1, nCurrLine});
            }
            continue;
        }

        ResetOneLine(nCurrLine, false);

        nCurrLine++;
        m_LineList.resize(nCurrLine + 1);

        if(!addRawToken(nCurrLine, stToken)){
            throw std::runtime_error(str_fflprintf(": Insert token to a new line failed: line = %d", (int)(nCurrLine)));
        }

        if(nLeafOff == 0){
            m_leaf2TokenLoc.push_back({m_LineList[nCurrLine].Content.size() - 1, nCurrLine});
        }
    }

    ResetOneLine(nCurrLine, true);
    ResetBoardPixelRegion();
}

std::tuple<int, int> XMLTypeset::TokenLocInXMLParagraph(int nX, int nY) const
{
    int nStartX = 0;
    int nStartY = 0;

    int nStartLeaf = GetToken(nX, nY)->Leaf;
    for(int nLine = nY; LineValid(nLine); --nLine){
        if(LineTokenCount(nLine) == 0){
            throw std::runtime_error(str_fflprintf(": Invalid empty line: %d", nLine));
        }

        if(GetToken(0, nLine)->Leaf != nStartLeaf){
            nStartY = nLine;
            break;
        }
    }

    for(int nCurrX = 0; nCurrX < LineTokenCount(nStartY); ++nCurrX){
        if(GetToken(nCurrX, nStartY)->Leaf == nStartLeaf){
            nStartX = nCurrX;
            break;
        }
    }

    if(nStartY == nY){
        return {nStartLeaf, nX - nStartX};
    }

    int nLeafOff = (LineTokenCount(nStartY) - nStartX) + nX;
    for(int nCurrLine = nStartY + 1; nCurrLine < nY; ++nCurrLine){
        nLeafOff += LineTokenCount(nCurrLine);
    }
    return {nStartLeaf, nLeafOff};
}

void XMLTypeset::ResetBoardPixelRegion()
{
    if(!lineCount()){
        m_PX = 0;
        m_PY = 0;
        m_PW = 0;
        m_PH = 0;
        return;
    }

    int nMaxPX = 0;
    int nMaxPY = 0;
    int nMinPX = std::numeric_limits<int>::max();
    int nMinPY = std::numeric_limits<int>::max();

    for(int nLine = 0; LineValid(nLine); ++nLine){
        if(!LineTokenCount(nLine)){
            throw std::runtime_error(str_fflprintf(": Found empty line in XMLTypeset: line = %d", nLine));
        }

        nMaxPX = (std::max<int>)(nMaxPX, LineReachMaxX(nLine));
        nMaxPY = (std::max<int>)(nMaxPY, LineReachMaxY(nLine));
        nMinPX = (std::min<int>)(nMinPX, LineReachMinX(nLine));
        nMinPY = (std::min<int>)(nMinPY, LineReachMinY(nLine));
    }

    m_PX = nMinPX;
    m_PY = nMinPY;
    m_PW = nMaxPX + 1 - nMinPX;
    m_PH = nMaxPY + 1 - nMinPY;
}

std::tuple<int, int> XMLTypeset::PrevTokenLoc(int nX, int nY) const
{
    if(!TokenLocValid(nX, nY)){
        throw std::invalid_argument(str_fflprintf(": Invalid token location: (%d, %d)", nX, nY));
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

std::tuple<int, int> XMLTypeset::NextTokenLoc(int nX, int nY) const
{
    if(!TokenLocValid(nX, nY)){
        throw std::invalid_argument(str_fflprintf(": Invalid token location: (%d, %d)", nX, nY));
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

void XMLTypeset::Delete(int nX, int nY, int nTokenCount)
{
    if(!TokenLocValid(nX, nY)){
        throw std::invalid_argument(str_fflprintf(": Invalid location: (X = %d, Y = %d)", nX, nY));
    }

    if(nTokenCount == 0){
        return;
    }

    auto [nLeaf, nLeafOff] = TokenLocInXMLParagraph(nX, nY);

    int nAdvanced = 0;
    std::tie(std::ignore, std::ignore, nAdvanced) = m_paragraph.NextLeafOff(nLeaf, nLeafOff, nTokenCount);

    m_paragraph.Delete(nLeaf, nLeafOff, nAdvanced);
    BuildBoard(nX, nY);
}

void XMLTypeset::InsertText(int nX, int nY, const char *szText)
{
    if(!TokenLocValid(nX, nY)){
        throw std::invalid_argument(str_fflprintf(": Invalid location: (X = %d, Y = %d)", nX, nY));
    }

    auto [nLeaf, nLeafOff] = TokenLocInXMLParagraph(nX, nY);
    if(m_paragraph.leafRef(nLeaf).Type() == LEAF_UTF8GROUP){
        m_paragraph.InsertUTF8Char(nLeaf, nLeafOff, szText);
    }
}

void XMLTypeset::drawEx(int nDstX, int nDstY, int nSrcX, int nSrcY, int nSrcW, int nSrcH) const
{
    if(!MathFunc::RectangleOverlap<int>(nSrcX, nSrcY, nSrcW, nSrcH, PX(), PY(), PW(), PH())){
        return;
    }

    int nDstDX = nDstX - nSrcX;
    int nDstDY = nDstY - nSrcY;

    uint32_t nColor   = 0;
    uint32_t nBGColor = 0;

    int nLastLeaf = -1;
    for(int nLine = 0; nLine < lineCount(); ++nLine){
        for(int nToken = 0; nToken < LineTokenCount(nLine); ++nToken){
            auto pToken = GetToken(nToken, nLine);

            int nX = pToken->Box.State.X;
            int nY = pToken->Box.State.Y;
            int nW = pToken->Box.Info.W;
            int nH = pToken->Box.Info.H;

            if(!MathFunc::RectangleOverlapRegion(nSrcX, nSrcY, nSrcW, nSrcH, &nX, &nY, &nW, &nH)){
                continue;
            }

            const int nDX = nX - pToken->Box.State.X;
            const int nDY = nY - pToken->Box.State.Y;

            auto &stLeaf = m_paragraph.leafRef(pToken->Leaf);
            if(nLastLeaf != pToken->Leaf){
                nColor    = stLeaf.  Color().value_or(  Color());
                nBGColor  = stLeaf.BGColor().value_or(BGColor());
                nLastLeaf = pToken->Leaf;
            }

            SDLDevice::EnableDrawBlendMode stEnableDrawBlendMode(SDL_BLENDMODE_BLEND);
            g_SDLDevice->FillRectangle(nBGColor, nX + nDstX, nY + nDstY, nW, nH);

            switch(stLeaf.Type()){
                case LEAF_UTF8GROUP:
                    {
                        auto pTexture = g_FontexDB->Retrieve(pToken->UTF8Char.U64Key);
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
                        int xOnTex = 0;
                        int yOnTex = 0;

                        const uint32_t emojiKey = [pToken]() -> uint8_t
                        {
                            if(pToken->Emoji.FrameCount && pToken->Emoji.FPS){
                                return (pToken->Emoji.U32Key & 0XFFFFFF00) + pToken->Emoji.Frame % pToken->Emoji.FrameCount;
                            }
                            return pToken->Emoji.U32Key & 0XFFFFFF00;
                        }();

                        if(auto ptex = g_EmoticonDB->Retrieve(emojiKey, &xOnTex, &yOnTex, 0, 0, 0, 0, 0)){
                            g_SDLDevice->DrawTexture(ptex, nX + nDstDX, nY + nDstDY, xOnTex + nDX, yOnTex + nDY, nW, nH);
                        }
                        else{
                            g_SDLDevice->DrawRectangle(ColorFunc::CompColor(nBGColor), nX + nDstDX, nY + nDstDY, nW, nH);
                        }
                        break;
                    }
            }

        }
    }
}

void XMLTypeset::update(double fMS)
{
    for(int leaf = 0; leaf < LeafCount(); ++leaf){
        if(m_paragraph.leafRef(leaf).Type() == LEAF_EMOJI){
            const auto [x, y] = leafTokenLoc(leaf);
            if(auto pToken= GetToken(x, y); pToken->Emoji.FPS != 0){
                double fPeroidMS = 1000.0 / pToken->Emoji.FPS;
                double fCurrTick = fMS + pToken->Emoji.Tick;

                auto nAdvancedFrame  = (uint8_t)(fCurrTick / fPeroidMS);
                pToken->Emoji.Tick   = (uint8_t)(fCurrTick - nAdvancedFrame * fPeroidMS);
                pToken->Emoji.Frame += nAdvancedFrame;
            }
        }
    }
}

std::string XMLTypeset::GetText(bool bTextOnly) const
{
    std::string szPlainString;
    for(int nIndex = 0; nIndex < m_paragraph.LeafCount(); ++nIndex){
        switch(auto nType = m_paragraph.leafRef(nIndex).Type()){
            case LEAF_UTF8GROUP:
                {
                    szPlainString += m_paragraph.leafRef(nIndex).UTF8Text();
                    break;
                }
            case LEAF_IMAGE:
                {
                    if(!bTextOnly){
                        szPlainString += str_printf("\\image{0x%016" PRIx64 "}", m_paragraph.leafRef(nIndex).ImageU64Key());
                    }
                    break;
                }
            case LEAF_EMOJI:
                {
                    if(!bTextOnly){
                        szPlainString += str_printf("\\emoji{0x%016" PRIu64 "}", (uint64_t)(m_paragraph.leafRef(nIndex).emojiU32Key()));
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

int XMLTypeset::GetTokenWordSpace(int nX, int nY) const
{
    if(!TokenLocValid(nX, nY)){
        throw std::invalid_argument(str_fflprintf(": Invalid token location: (%d, %d)", nX, nY));
    }
    return m_WordSpace;
}

int XMLTypeset::LineReachMaxX(int nLine) const
{
    if(!LineValid(nLine)){
        throw std::invalid_argument(str_fflprintf(": Invalid line: %d", nLine));
    }

    if(LineTokenCount(nLine) == 0){
        throw std::runtime_error(str_fflprintf(": Invalie empty line: %d", nLine));
    }

    auto pToken = GetLineBackToken(nLine);
    return pToken->Box.State.X + pToken->Box.Info.W;
}

int XMLTypeset::LineReachMaxY(int nLine) const
{
    if(!LineValid(nLine)){
        throw std::invalid_argument(str_fflprintf(": Invalid line: %d", nLine));
    }
    return m_LineList[nLine].StartY + LineMaxHk(nLine, 2);
}

int XMLTypeset::LineReachMinX(int nLine) const
{
    if(!LineValid(nLine)){
        throw std::invalid_argument(str_fflprintf(": Invalid line: %d", nLine));
    }

    if(LineTokenCount(nLine) == 0){
        throw std::runtime_error(str_fflprintf(": Invalie empty line: %d", nLine));
    }

    return GetToken(0, nLine)->Box.State.X;
}

int XMLTypeset::LineReachMinY(int nLine) const
{
    if(!LineValid(nLine)){
        throw std::invalid_argument(str_fflprintf(": Invalid line: %d", nLine));
    }
    return m_LineList[nLine].StartY - LineMaxHk(nLine, 1) + 1;
}

int XMLTypeset::LineMaxHk(int nLine, int k) const
{
    // nHk = 1: get MaxH1
    // nHk = 2: get MaxH2

    if(!LineValid(nLine)){
        throw std::invalid_argument(str_fflprintf(": Invalid line: %d", nLine));
    }

    if(k != 1 && k != 2){
        throw std::invalid_argument(str_fflprintf(": Invalid argument k: %d", k));
    }

    int nCurrMaxHk = 0;
    for(int nIndex = 0; nIndex < LineTokenCount(nLine); ++nIndex){
        nCurrMaxHk = (std::max<int>)(nCurrMaxHk, (k == 1) ? GetToken(nIndex, nLine)->Box.State.H1 : GetToken(nIndex, nLine)->Box.State.H2);
    }
    return nCurrMaxHk;
}

int XMLTypeset::LAlign() const
{
    if(MaxLineWidth() == 0){
        return LALIGN_LEFT;
    }
    return m_LAlign;
}
