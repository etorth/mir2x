/*
 * =====================================================================================
 *
 *       Filename: xmltypeset.hpp
 *        Created: 12/11/2018 04:40:11
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

#pragma once
#include <tuple>
#include "token.hpp"
#include "lalign.hpp"
#include "xmlfunc.hpp"
#include "fflerror.hpp"
#include "colorfunc.hpp"
#include "xmlparagraph.hpp"

class XMLTypeset // means XMLParagraph typeset
{
    private:
        struct ContentLine
        {
            int StartY;
            std::vector<TOKEN> Content;
        };

    private:
        const int m_MaxLineWidth;

    private:
        const int m_LAlign;

    private:
        const bool m_CanThrough;

    private:
        int m_WordSpace;
        int m_LineSpace;

    private:
        uint8_t  m_font;
        uint8_t  m_fontSize;
        uint8_t  m_fontStyle;
        uint32_t m_fontColor;

    private:
        int m_PX;
        int m_PY;
        int m_PW;
        int m_PH;

    private:
        XMLParagraph m_paragraph;

    private:
        std::vector<ContentLine> m_LineList;

    private:
        std::vector<std::tuple<int, int>> m_leaf2TokenLoc;

    public:
        XMLTypeset(
                int      nMaxLineWidth,
                int      nLAlign           =  LALIGN_LEFT,
                bool     bCanThrough       =  true,
                uint8_t  nDefaultFont      =  0,
                uint8_t  nDefaultFontSize  = 10,
                uint8_t  nDefaultFontStyle =  0,
                uint32_t nDefaultFontColor =  ColorFunc::WHITE + 255,
                int      nLineSpace        =  0,
                int      nWordSpace        =  0)
            : m_MaxLineWidth(nMaxLineWidth)
            , m_LAlign(nLAlign)
            , m_CanThrough(bCanThrough)
            , m_WordSpace(nWordSpace)
            , m_LineSpace(nLineSpace)
            , m_font(nDefaultFont)
            , m_fontSize(nDefaultFontSize)
            , m_fontStyle(nDefaultFontStyle)
            , m_fontColor(nDefaultFontColor)
            , m_PX(0)
            , m_PY(0)
            , m_PW(0)
            , m_PH(0)
            , m_paragraph()
            , m_LineList()
        {
            checkDefaultFont();
        }

    public:
        ~XMLTypeset() = default;

    public:
        bool empty() const
        {
            return m_paragraph.empty();
        }

    public:
        void loadXML(const char *szXMLString)
        {
            Clear();
            m_paragraph.loadXML(szXMLString);

            if(m_paragraph.LeafCount() > 0){
                BuildBoard(0, 0);
            }
        }

        void loadXMLNode(const tinyxml2::XMLNode *node)
        {
            Clear();
            m_paragraph.loadXMLNode(node);

            if(m_paragraph.LeafCount() > 0){
                BuildBoard(0, 0);
            }
        }

    public:
        void Clear()
        {
            m_PX = 0;
            m_PY = 0;
            m_PW = 0;
            m_PH = 0;
            m_LineList.clear();
        }

    public:
        XMLTypeset Break();

    private:
        void ResetBoardPixelRegion();

    public:
        bool LineValid(int line) const
        {
            return line >= 0 && line < lineCount();
        }

        int lineCount() const
        {
            return (int)(m_LineList.size());
        }

        int LineTokenCount(int nLine) const
        {
            if(LineValid(nLine)){
                return m_LineList[nLine].Content.size();
            }
            throw fflerror("invalid line specified: %d >= %d", nLine, lineCount());
        }

    private:
        std::tuple<int, int> PrevTokenLoc(int, int) const;
        std::tuple<int, int> NextTokenLoc(int, int) const;

    private:
        std::tuple<int, int> LastTokenLoc() const
        {
            if(empty()){
                throw fflerror("empty board");
            }
            return {LineTokenCount(lineCount() - 1) - 1, lineCount() - 1};
        }

    public:
        std::tuple<int, int> leafTokenLoc(int leaf) const
        {
            if(LeafValid(leaf)){
                return m_leaf2TokenLoc.at(leaf);
            }
            throw fflerror("invalid leaf: %d", leaf);
        }

    private:
        bool TokenLocValid(int nX, int nY) const
        {
            return LineValid(nY) && (nX < LineTokenCount(nY));
        }

    public:
        void update(double);

    public:
        void InsertXML (int, int, const char *);
        void InsertText(int, int, const char *);

    public:
        void Break(int, int);

    public:
        void Delete(int, int, int);

    public:
        int LeafCount() const
        {
            return m_paragraph.LeafCount();
        }

        bool LeafValid(int nLeaf) const
        {
            return nLeaf < LeafCount();
        }

    public:
        void MarkLeafEvent(int nLeaf, int nEvent)
        {
            m_paragraph.leafRef(nLeaf).MarkEvent(nEvent);
        }

    public:
        void drawEx(int, int, int, int, int, int) const;

    public:
        void SetDefaultFont(uint8_t nFont)
        {
            m_font = nFont;
        }

        void SetDefaultFontSize(uint8_t nFontSize)
        {
            m_fontSize = nFontSize;
        }

        void SetDefaultFontStyle(uint8_t nFontStyle)
        {
            m_fontStyle = nFontStyle;
        }

        void SetDefaultFontColor(uint32_t nFontColor)
        {
            m_fontColor = nFontColor;
        }

    public:
        std::string PrintXML() const
        {
            return m_paragraph.PrintXML();
        }

    public:
        std::string GetText(bool) const;

    private:
        void checkDefaultFont() const;

    private:
        void ResetOneLine(int, bool);

    private:
        bool addRawToken(int, const TOKEN &);

    private:
        void SetTokenBoxWordSpace(int);

    private:
        void SetLineTokenStartX(int);
        void SetLineTokenStartY(int);

    private:
        int LineRawWidth(int, bool) const;

    private:
        int LineFullWidth(int) const;

    public:
        const TOKEN *GetToken(int nX, int nY) const
        {
            if(!TokenLocValid(nX, nY)){
                throw std::invalid_argument(str_fflprintf(": Invalid token location: (%d, %d)", nX, nY));
            }
            return &(m_LineList[nY].Content[nX]);
        }

        TOKEN *GetToken(int nX, int nY)
        {
            return const_cast<TOKEN *>(static_cast<const XMLTypeset *>(this)->GetToken(nX, nY));
        }

    public:
        const TOKEN *GetLineBackToken(int nLine) const
        {
            if(!LineValid(nLine)){
                throw std::invalid_argument(str_fflprintf(": Invalid line: %d", nLine));
            }

            if(LineTokenCount(nLine) == 0){
                throw std::runtime_error(str_fflprintf(": Invalie empty line: %d", nLine));
            }

            return GetToken(LineTokenCount(nLine) - 1, nLine);
        }

        TOKEN *GetLineBackToken(int nLine)
        {
            return const_cast<TOKEN *>(static_cast<const XMLTypeset *>(this)->GetLineBackToken(nLine));
        }

    public:
        const TOKEN *GetBackToken() const
        {
            if(lineCount() == 0){
                throw std::runtime_error(str_fflprintf(": empty board"));
            }

            if(LineTokenCount(lineCount() - 1) == 0){
                throw std::runtime_error(str_fflprintf(": Invalie empty line: %d", lineCount() - 1));
            }

            return GetLineBackToken(lineCount() - 1);
        }

        TOKEN *GetBackToken()
        {
            return const_cast<TOKEN *>(static_cast<const XMLTypeset *>(this)->GetBackToken());
        }

    private:
        int GetTokenWordSpace(int, int) const;

    private:
        bool AppendToken(int, const TOKEN &);
        void LinePadding(int);

    private:
        TOKEN BuildUTF8Token(int, uint8_t, uint8_t, uint8_t, uint32_t) const;
        TOKEN buildEmojiToken(int, uint32_t) const;

    private:
        std::tuple<int, int> TokenLocInXMLParagraph(int, int) const;

    private:
        TOKEN CreateToken(int, int) const;

    private:
        void BuildBoard(int, int);

    private:
        int LineReachMaxX(int) const;
        int LineReachMaxY(int) const;
        int LineReachMinX(int) const;
        int LineReachMinY(int) const;

    public:
        int PX() const
        {
            return m_PX;
        }

        int PY() const
        {
            return m_PY;
        }

        int PW() const
        {
            return m_PW;
        }

        int PH() const
        {
            return m_PH;
        }

    public:
        int LineMaxHk(int, int) const;

    private:
        void LineJustifyPadding(int);
        void LineDistributedPadding(int);

    public:
        int LAlign() const;

    public:
        int MaxLineWidth() const
        {
            return m_MaxLineWidth;
        }

        bool CanThrough() const
        {
            return m_CanThrough;
        }

    private:
        int LineNewStartY(int);
        int LineTokenBestY(int, int, int, int) const;
        int LineIntervalMaxH2(int, int, int) const;

    public:
        uint32_t Color() const
        {
            return m_fontColor;
        }

        uint32_t BGColor() const
        {
            return 0;
        }
};
