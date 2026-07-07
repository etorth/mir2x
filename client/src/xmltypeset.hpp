#pragma once
#include <climits>
#include <tuple>
#include <deque>
#include <memory>
#include <optional>
#include "token.hpp"
#include "lalign.hpp"
#include "xmlf.hpp"
#include "fflerror.hpp"
#include "colorf.hpp"
#include "bevent.hpp"
#include "xmlparagraph.hpp"
#include "widget.hpp" // Widget::VarXXX

class XMLTypeset // means XMLParagraph typeset
{
    private:
        struct contentLine
        {
            int startY; // Y-axis coordinate reached by all tokens' H1, representing the bottom line of H1 pixels
                        // If all tokens are with H1 == 0, startY is the Y-axis coordinate above starting line of all tokens
            std::deque<TOKEN> content;
        };

        struct LeafInfo
        {
            int tokenX = 0;
            int tokenY = 0;
            int  maxH1 = 0;
            int  maxH2 = 0;

            std::tuple<int, int> tokenLoc() const
            {
                return {tokenX, tokenY};
            }
        };

    private:
        int m_lineWidth;

    private:
        const int m_lineAlign;

    private:
        const bool m_canThrough;
        const bool m_compactLine;

    private:
        int m_wordSpace;
        int m_lineSpace;

    private:
        uint8_t m_font;
        uint8_t m_fontSize;
        uint8_t m_fontStyle;

        Widget::VarU32 m_fontColor;
        Widget::VarU32 m_fontBGColor;
        Widget::VarU32 m_imageMaskColor;

    private:
        int m_px = 0;
        int m_py = 0;
        int m_pw = 0;
        int m_ph = 0;

    private:
        std::unique_ptr<XMLParagraph> m_paragraph;

    private:
        std::deque<contentLine> m_lineList;

    private:
        std::deque<LeafInfo> m_leafInfoList;

    public:
        XMLTypeset(
                int maxLineWidth,

                int  lineAlign  = LALIGN_LEFT,
                bool canThrough = true,
                bool compactLine = false,

                uint8_t defaultFont      = 0,
                uint8_t defaultFontSize  = 8,
                uint8_t defaultFontStyle = 0,

                Widget::VarU32 defaultFontColor      = colorf::WHITE_A255,
                Widget::VarU32 defaultFontBGColor    = 0U,
                Widget::VarU32 defaultImageMaskColor = colorf::WHITE_A255,

                int lineSpace = 0,
                int wordSpace = 0)

            : m_lineWidth(maxLineWidth)
            , m_lineAlign(lineAlign)
            , m_canThrough(canThrough)
            , m_compactLine(compactLine)
            , m_wordSpace(wordSpace)
            , m_lineSpace(lineSpace)

            , m_font(defaultFont)
            , m_fontSize(defaultFontSize)
            , m_fontStyle(defaultFontStyle)

            , m_fontColor(std::move(defaultFontColor))
            , m_fontBGColor(std::move(defaultFontBGColor))
            , m_imageMaskColor(std::move(defaultImageMaskColor))
            , m_paragraph(std::make_unique<XMLParagraph>())
        {
            checkDefaultFontEx();
        }

    public:
        ~XMLTypeset() = default;

    public:
        bool empty() const
        {
            return m_paragraph->empty();
        }

        bool lineEmpty(int argLine) const
        {
            fflassert(lineValid(argLine), argLine);
            return m_lineList.at(argLine).content.empty();
        }

    public:
        void loadXML(const char *xmlString)
        {
            clear();
            m_paragraph->loadXML(xmlString);
            updateGfx();
        }

        void loadXMLNode(const tinyxml2::XMLNode *node)
        {
            clear();
            m_paragraph->loadXMLNode(node);
            updateGfx();
        }

    public:
        void clear() // release everything
        {
            m_px = 0;
            m_py = 0;
            m_pw = 0;
            m_ph = 0;
            m_lineList.clear();
            m_paragraph->clear();
        }

        void updateGfx() // build without reload xml
        {
            if(m_paragraph->leafCount() > 0){
                buildTypeset(0, 0);
            }
            else{
                m_ph = getDefaultFontHeight();
            }
        }

    private:
        void resetBoardPixelRegion();

    public:
        bool lineValid(int line) const
        {
            return line >= 0 && line < lineCount();
        }

        int lineCount() const
        {
            return to_d(m_lineList.size());
        }

        int lineTokenCount(int argLine) const
        {
            fflassert(lineValid(argLine), argLine);
            return to_d(m_lineList[argLine].content.size());
        }

        int lineStartY(int argLine) const
        {
            fflassert(lineValid(argLine), argLine);
            return m_lineList[argLine].startY;
        }

    public:
        std::tuple<int, int> prevTokenLoc(int, int, int = 1, bool = true) const;
        std::tuple<int, int> nextTokenLoc(int, int, int = 1, bool = true) const;

    public:
        std::tuple<int, int> prevCursorLoc(int, int,      bool, bool = true) const;
        std::tuple<int, int> nextCursorLoc(int, int,      bool, bool = true) const;
        std::tuple<int, int> prevCursorLoc(int, int, int, bool, bool = true) const;
        std::tuple<int, int> nextCursorLoc(int, int, int, bool, bool = true) const;

    public:
        std::optional<std::tuple<int, int>> tokenLocBeforeCursor(int, int) const; // use for deleteToken(cursorLoc)

    public:
        static bool locInToken(int, int, const TOKEN *, bool withPadding);

    public:
        std::tuple<int, int> locToken(int, int, bool withPadding) const;

    public:
        std::tuple<int, int> locCursor(int, int) const;

    public:
        std::tuple<int, int> firstTokenLoc() const
        {
            if(empty()){
                throw fflpanic("empty typeset");
            }
            return {0, 0};
        }

        std::tuple<int, int> lastTokenLoc() const
        {
            if(empty()){
                throw fflpanic("empty board");
            }
            return {lineTokenCount(lineCount() - 1) - 1, lineCount() - 1};
        }

    public:
        std::tuple<int, int> firstCursorLoc() const
        {
            if(empty()){
                return {0, 0};
            }
            return {0, 0};
        }

        std::tuple<int, int> lastCursorLoc() const
        {
            if(empty()){
                return {0, 0};
            }
            return {lineTokenCount(lineCount() - 1), lineCount() - 1};
        }

    public:
        std::tuple<int, int> leafTokenLoc(int leafIndex) const
        {
            if(leafValid(leafIndex)){
                return m_leafInfoList.at(leafIndex).tokenLoc();
            }
            throw fflpanic("invalid leaf: {}", leafIndex);
        }

    public:
        bool tokenLocValid(int argX, int argY) const
        {
            return lineValid(argY) && (argX >= 0) && (argX < lineTokenCount(argY));
        }

        bool cursorLocValid(int argX, int argY) const
        {
            if(empty()){
                return argX == 0 && argY == 0;
            }
            return lineValid(argY) && (argX >= 0) && (argX <= lineTokenCount(argY));
        }

    public:
        int cursorLoc2Off(int, int) const; // actually returns how many tokens in front of cursor
        std::tuple<int, int> cursorOff2Loc(int) const;

    public:
        void update(double);

    public:
        void InsertXML(int, int, const char *);

    public:
        size_t insertUTF8String(int, int, const char *);

    public:
        XMLTypeset *split(int, int);
        void join(const XMLTypeset &, bool);

    public:
        void deleteToken(int, int, int); // deleteToken(tokenLoc)

    public:
        int leafCount() const
        {
            return m_paragraph->leafCount();
        }

        bool leafValid(int leafIndex) const
        {
            return leafIndex >= 0 && leafIndex < leafCount();
        }

    public:
        void clearEvent(int currLeaf = -1)
        {
            for(int leafIndex = 0; leafIndex < m_paragraph->leafCount(); ++leafIndex){
                if(leafIndex != currLeaf){
                    m_paragraph->leaf(leafIndex).markEvent(BEVENT_OFF);
                }
            }
        }

        int markLeafEvent(int leafIndex, int event)
        {
            return m_paragraph->leaf(leafIndex).markEvent(event);
        }

    public:
        void draw(Widget::ROIMap) const;

    public:
        void setFont(uint8_t font)
        {
            m_font = font;
        }

        void setFontSize(uint8_t fontSize)
        {
            m_fontSize = fontSize;
        }

        void setFontStyle(uint8_t fontStyle)
        {
            m_fontStyle = fontStyle;
        }

        void setFontColor(Widget::VarU32 fontColor)
        {
            m_fontColor = std::move(fontColor);
        }

        void setFontBGColor(Widget::VarU32 fontBGColor)
        {
            m_fontBGColor = std::move(fontBGColor);
        }

        void setImageMaskColor(Widget::VarU32 imageMaskColor)
        {
            m_imageMaskColor = std::move(imageMaskColor);
        }

    public:
        std::string getXML() const
        {
            return m_paragraph->getXML();
        }

    public:
        std::string getText() const;

    public:
        const tinyxml2::XMLNode *getXMLNode() const
        {
            return m_paragraph->getXMLNode();
        }

    public:
        const auto leafEvent(int leafID) const
        {
            return m_paragraph->leaf(leafID).hasEvent();
        }

    private:
        void checkDefaultFontEx() const;

    private:
        void resetOneLine(int, bool);

    private:
        bool addRawTokenLine(int, const std::vector<TOKEN> &);

    private:
        void setTokenBoxWordSpace(int);

    private:
        void setLineTokenStartX(int);
        void setLineTokenStartY(int);

    private:
        int LineRawWidth(int, bool) const;

    private:
        int LineFullWidth(int) const;

    public:
        auto getToken(this auto && self, int argX, int argY)
        {
            if(!self.tokenLocValid(argX, argY)){
                throw fflpanic("invalid token location: ({}, {})", argX, argY);
            }
            return std::addressof(self.m_lineList[argY].content[argX]);
        }

    public:
        auto GetLineBackToken(this auto && self, int argLine)
        {
            if(!self.lineValid(argLine)){
                throw fflpanic("invalid line: {}", argLine);
            }

            if(self.lineTokenCount(argLine) == 0){
                throw fflpanic("invalie empty line: {}", argLine);
            }

            return self.getToken(self.lineTokenCount(argLine) - 1, argLine);
        }

    public:
        auto GetBackToken(this auto && self)
        {
            if(self.lineCount() == 0){
                throw fflpanic("empty board");
            }

            if(self.lineTokenCount(self.lineCount() - 1) == 0){
                throw fflpanic("invalie empty line: {}", self.lineCount() - 1);
            }

            return self.GetLineBackToken(self.lineCount() - 1);
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
        std::vector<TOKEN> createTokenLine(int, int, int &, int &, std::vector<TOKEN> * = nullptr) const;

    private:
        void buildTypeset(int, int);

    private:
        int lineReachMaxX(int) const;
        int lineReachMaxY(int) const;
        int lineReachMinX(int) const;
        int lineReachMinY(int) const;

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
        int lineAlign() const;

    public:
        int MaxLineWidth() const
        {
            return m_lineWidth;
        }

        bool CanThrough() const
        {
            return m_canThrough;
        }

    private:
        int LineNewStartY(int);
        int LineTokenBestY(int, int, int, int) const;
        int LineIntervalMaxH2(int, int, int) const;

    public:
        uint32_t color() const
        {
            return Widget::evalU32(m_fontColor, nullptr, this);
        }

        uint32_t bgColor() const
        {
            return Widget::evalU32(m_fontBGColor, nullptr, this);
        }

    public:
        std::string getRawString() const
        {
            return m_paragraph->getRawString();
        }

        void setLineWidth(int);

    public:
        bool blankToken(int, int) const;

    public:
        std::tuple<int, int> getDefaultFontHk() const;
        std::tuple<int, int> getTokenCursorHk(int, int) const;
        int getDefaultFontHeight() const;
};
