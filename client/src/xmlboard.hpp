/*
 * =====================================================================================
 *
 *       Filename: xmlboard.hpp
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
#include "colorfunc.hpp"
#include "xmlparagraph.hpp"

class XMLBoard
{
    private:
        struct ContentLine
        {
            size_t StartY;
            std::vector<TOKEN> Content;
        };

    private:
        const size_t m_MaxLineWidth;

    private:
        const int m_LAlign;

    private:
        const bool m_CanThrough;

    private:
        size_t m_WordSpace;
        size_t m_LineSpace;

    private:
        uint8_t  m_DefaultFont;
        uint8_t  m_DefaultFontSize;
        uint8_t  m_DefaultFontStyle;
        uint32_t m_DefaultFontColor;

    private:
        size_t m_PX;
        size_t m_PY;
        size_t m_PW;
        size_t m_PH;

    private:
        XMLParagraph m_Paragraph;

    private:
        std::vector<ContentLine> m_LineList;

    public:
        XMLBoard(
                size_t      nMaxLineWidth,
                int         nLAlign           = LALIGN_LEFT,
                bool        bCanThrough       = true,
                size_t      nWordSpace        = 0,
                size_t      nLineSpace        = 0,
                uint8_t     nDefaultFont      = 0,
                uint8_t     nDefaultFontSize  = 0,
                uint8_t     nDefaultFontStyle = 0,
                uint32_t    nDefaultFontColor = ColorFunc::WHITE + 255)
            : m_MaxLineWidth(nMaxLineWidth)
            , m_LAlign(nLAlign)
            , m_CanThrough(bCanThrough)
            , m_WordSpace(nWordSpace)
            , m_LineSpace(nLineSpace)
            , m_DefaultFont(nDefaultFont)
            , m_DefaultFontSize(nDefaultFontSize)
            , m_DefaultFontStyle(nDefaultFontStyle)
            , m_DefaultFontColor(nDefaultFontColor)
            , m_PX(0)
            , m_PY(0)
            , m_PW(0)
            , m_PH(0)
            , m_Paragraph()
            , m_LineList()
        {
            CheckDefaultFont();
        }

    public:
        ~XMLBoard() = default;

    public:
        bool Empty() const
        {
            return m_Paragraph.Empty();
        }

    public:
        void LoadXML(const char *szXMLString)
        {
            Clear();
            m_Paragraph.LoadXML(szXMLString);
            BuildBoard(0, 0);
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
        XMLBoard Break();

    private:
        void ResetBoardPixelRegion();

    public:
        bool LineValid(size_t nLine) const
        {
            return nLine < m_LineList.size();
        }

        size_t LineCount() const
        {
            return m_LineList.size();
        }

        size_t LineTokenCount(size_t nLine) const
        {
            if(nLine < LineCount()){
                return m_LineList[nLine].Content.size();
            }
            throw std::invalid_argument(str_fflprintf(": Invalid line specified: %zu >= %zu", nLine, LineCount()));
        }

    private:
        std::tuple<size_t, size_t> PrevTokenLoc(size_t, size_t) const;
        std::tuple<size_t, size_t> NextTokenLoc(size_t, size_t) const;

    private:
        std::tuple<size_t, size_t, size_t> PrevTokenLoc(size_t, size_t, size_t) const;
        std::tuple<size_t, size_t, size_t> NextTokenLoc(size_t, size_t, size_t) const;

    private:
        std::tuple<size_t, size_t> LastTokenLoc() const
        {
            if(Empty()){
                throw std::runtime_error(str_fflprintf(": Empty board"));
            }
            return {LineTokenCount(LineCount() - 1) - 1, LineCount() - 1};
        }

    private:
        bool TokenLocValid(size_t nX, size_t nY) const
        {
            return LineValid(nY) && (nX < LineTokenCount(nY));
        }

    public:
        void Update(double);

    public:
        void InsertXML (size_t, size_t, const char *);
        void InsertText(size_t, size_t, const char *);

    public:
        void Break(size_t, size_t);

    public:
        void Delete(size_t, size_t, size_t);

    public:
        size_t LeafCount() const
        {
            return m_Paragraph.LeafCount();
        }

        bool LeafValid(size_t nLeaf) const
        {
            return nLeaf < LeafCount();
        }

    public:
        void MarkLeafEvent(size_t nLeaf, int nEvent)
        {
            m_Paragraph.LeafRef(nLeaf).MarkEvent(nEvent);
        }

    public:
        void DrawEx(int, int, int, int, int, int) const;

    public:
        void SetDefaultFont(uint8_t nFont)
        {
            m_DefaultFont = nFont;
        }

        void SetDefaultFontSize(uint8_t nFontSize)
        {
            m_DefaultFontSize = nFontSize;
        }

        void SetDefaultFontStyle(uint8_t nFontStyle)
        {
            m_DefaultFontStyle = nFontStyle;
        }

        void SetDefaultFontColor(uint32_t nFontColor)
        {
            m_DefaultFontColor = nFontColor;
        }

    public:
        std::string PrintXML() const
        {
            return m_Paragraph.PrintXML();
        }

    public:
        std::string GetText(bool) const;

    private:
        void CheckDefaultFont() const;

    private:
        void ResetOneLine(size_t, bool);

    private:
        bool AddRawToken(size_t, const TOKEN &);

    private:
        void SetTokenBoxWordSpace(size_t);

    private:
        void SetLineTokenStartX(size_t);
        void SetLineTokenStartY(size_t);

    private:
        size_t LineRawWidth(size_t, bool) const;

    private:
        size_t LineFullWidth(size_t) const;

    public:
        const TOKEN *GetToken(size_t nX, size_t nY) const
        {
            if(!TokenLocValid(nX, nY)){
                throw std::invalid_argument(str_fflprintf(": Invalid token location: (%zu, %zu)", nX, nY));
            }
            return &(m_LineList[nY].Content[nX]);
        }

        TOKEN *GetToken(size_t nX, size_t nY)
        {
            return const_cast<TOKEN *>(static_cast<const XMLBoard *>(this)->GetToken(nX, nY));
        }

    public:
        const TOKEN *GetLineBackToken(size_t nLine) const
        {
            if(!LineValid(nLine)){
                throw std::invalid_argument(str_fflprintf(": Invalid line: %zu", nLine));
            }

            if(LineTokenCount(nLine) == 0){
                throw std::runtime_error(str_fflprintf(": Invalie empty line: %zu", nLine));
            }

            return GetToken(LineTokenCount(nLine) - 1, nLine);
        }

        TOKEN *GetLineBackToken(size_t nLine)
        {
            return const_cast<TOKEN *>(static_cast<const XMLBoard *>(this)->GetLineBackToken(nLine));
        }

    public:
        const TOKEN *GetBackToken() const
        {
            if(LineCount() == 0){
                throw std::runtime_error(str_fflprintf(": Empty board"));
            }

            if(LineTokenCount(LineCount() - 1) == 0){
                throw std::runtime_error(str_fflprintf(": Invalie empty line: %zu", LineCount() - 1));
            }

            return GetLineBackToken(LineCount() - 1);
        }

        TOKEN *GetBackToken()
        {
            return const_cast<TOKEN *>(static_cast<const XMLBoard *>(this)->GetBackToken());
        }

    private:
        size_t GetTokenWordSpace(size_t, size_t) const;

    private:
        bool AppendToken(size_t, const TOKEN &);
        void LinePadding(size_t);

    private:
        TOKEN BuildUTF8Token(uint8_t, uint8_t, uint8_t, uint32_t) const;

    private:
        std::tuple<size_t, size_t> TokenLocInXMLParagraph(size_t, size_t) const;

    private:
        TOKEN CreateToken(size_t, size_t) const;

    private:
        void BuildBoard(size_t, size_t);

    private:
        size_t LineReachMaxX(size_t) const;
        size_t LineReachMaxY(size_t) const;
        size_t LineReachMinX(size_t) const;
        size_t LineReachMinY(size_t) const;

    public:
        size_t PX() const
        {
            return m_PX;
        }

        size_t PY() const
        {
            return m_PY;
        }

        size_t PW() const
        {
            return m_PW;
        }

        size_t PH() const
        {
            return m_PH;
        }

    public:
        size_t LineMaxHk(size_t, size_t) const;

    private:
        void LineJustifyPadding(size_t);
        void LineDistributedPadding(size_t);

    public:
        int LAlign() const;

    public:
        size_t MaxLineWidth() const
        {
            return m_MaxLineWidth;
        }

        bool CanThrough() const
        {
            return m_CanThrough;
        }

    private:
        size_t LineNewStartY(size_t);
        size_t LineTokenBestY(size_t, size_t, size_t, size_t) const;
        int    LineIntervalMaxH2(size_t, size_t, size_t) const;

    public:
        uint32_t Color() const
        {
            return ColorFunc::WHITE + 255;
        }

        uint32_t BGColor() const
        {
            return 0;
        }
};
