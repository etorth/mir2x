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
        struct contentLine
        {
            int startY;
            std::vector<TOKEN> content;
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
        int m_px;
        int m_py;
        int m_pw;
        int m_ph;

    private:
        XMLParagraph m_paragraph;

    private:
        std::vector<contentLine> m_lineList;

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
            , m_px(0)
            , m_py(0)
            , m_pw(0)
            , m_ph(0)
            , m_paragraph()
            , m_lineList()
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

        bool lineEmpty(int line) const
        {
            return m_lineList.at(line).content.empty();
        }

    public:
        void loadXML(const char *szXMLString)
        {
            clear();
            m_paragraph.loadXML(szXMLString);

            if(m_paragraph.leafCount() > 0){
                buildTypeset(0, 0);
            }
        }

        void loadXMLNode(const tinyxml2::XMLNode *node)
        {
            clear();
            m_paragraph.loadXMLNode(node);

            if(m_paragraph.leafCount() > 0){
                buildTypeset(0, 0);
            }
        }

    public:
        void clear()
        {
            m_px = 0;
            m_py = 0;
            m_pw = 0;
            m_ph = 0;
            m_lineList.clear();
            m_paragraph.clear();
        }

    public:
        XMLTypeset Break();

    private:
        void resetBoardPixelRegion();

    public:
        bool lineValid(int line) const
        {
            return line >= 0 && line < lineCount();
        }

        int lineCount() const
        {
            return (int)(m_lineList.size());
        }

        int lineTokenCount(int nLine) const
        {
            if(lineValid(nLine)){
                return m_lineList[nLine].content.size();
            }
            throw fflerror("invalid line specified: %d >= %d", nLine, lineCount());
        }

    private:
        std::tuple<int, int> prevTokenLoc(int, int) const;
        std::tuple<int, int> NextTokenLoc(int, int) const;

    public:
        static bool locInToken(int, int, const TOKEN *, bool withPadding);

    public:
        std::tuple<int, int> locToken(int, int, bool withPadding) const;

    public:
        std::tuple<int, int> locCursor(int, int) const;

    private:
        std::tuple<int, int> LastTokenLoc() const
        {
            if(empty()){
                throw fflerror("empty board");
            }
            return {lineTokenCount(lineCount() - 1) - 1, lineCount() - 1};
        }

    public:
        std::tuple<int, int> leafTokenLoc(int leaf) const
        {
            if(leafValid(leaf)){
                return m_leaf2TokenLoc.at(leaf);
            }
            throw fflerror("invalid leaf: %d", leaf);
        }

    public:
        bool tokenLocValid(int nX, int nY) const
        {
            return lineValid(nY) && (nX >= 0) && (nX < lineTokenCount(nY));
        }

        bool cursorLocValid(int nX, int nY) const
        {
            return (nX == 0 && nY == 0) || (lineValid(nY) && (nX >= 0) && (nX <= lineTokenCount(nY)));
        }

    public:
        void update(double);

    public:
        void InsertXML(int, int, const char *);

    public:
        void insertUTF8String(int, int, const char *);

    public:
        void Break(int, int);

    public:
        void deleteToken(int, int, int);

    public:
        int leafCount() const
        {
            return m_paragraph.leafCount();
        }

        bool leafValid(int leaf) const
        {
            return leaf < leafCount();
        }

    public:
        void MarkLeafEvent(int leaf, int nEvent)
        {
            m_paragraph.leafRef(leaf).markEvent(nEvent);
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
        void resetOneLine(int, bool);

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
        const TOKEN *getToken(int nX, int nY) const
        {
            if(!tokenLocValid(nX, nY)){
                throw std::invalid_argument(str_fflprintf(": Invalid token location: (%d, %d)", nX, nY));
            }
            return &(m_lineList[nY].content[nX]);
        }

        TOKEN *getToken(int nX, int nY)
        {
            return const_cast<TOKEN *>(static_cast<const XMLTypeset *>(this)->getToken(nX, nY));
        }

    public:
        const TOKEN *GetLineBackToken(int nLine) const
        {
            if(!lineValid(nLine)){
                throw std::invalid_argument(str_fflprintf(": Invalid line: %d", nLine));
            }

            if(lineTokenCount(nLine) == 0){
                throw std::runtime_error(str_fflprintf(": Invalie empty line: %d", nLine));
            }

            return getToken(lineTokenCount(nLine) - 1, nLine);
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

            if(lineTokenCount(lineCount() - 1) == 0){
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
        TOKEN buildUTF8Token(int, uint8_t, uint8_t, uint8_t, uint32_t) const;
        TOKEN buildEmojiToken(int, uint32_t) const;

    private:
        std::tuple<int, int> leafLocInXMLParagraph(int, int) const;

    private:
        TOKEN createToken(int, int) const;

    private:
        void buildTypeset(int, int);

    private:
        int LineReachMaxX(int) const;
        int LineReachMaxY(int) const;
        int LineReachMinX(int) const;
        int LineReachMinY(int) const;

    public:
        int px() const
        {
            return m_px;
        }

        int py() const
        {
            return m_py;
        }

        int pw() const
        {
            return m_pw;
        }

        int ph() const
        {
            return m_ph;
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

    public:
        std::string getRawString() const
        {
            return m_paragraph.getRawString();
        }
};
