#pragma once
#include <array>
#include <algorithm>
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
    public:
        struct InitArgs final
        {
            int lineWidth = 0; // lineMargin not included

            const int  lineAlign  = LALIGN_LEFT;
            const bool canThrough = true;
            const bool compactLine = false;

            Widget::FontConfig font
            {
                .id = 0,
                .size = 8,
            };

            Widget::VarU32 imageMaskColor = colorf::WHITE_A255;

            int lineSpace = 0;
            int wordSpace = 0;

            std::array<int, 2> lineMargin {0, 0}; // for w1 of first token and w2 of last token
            std::function<uint32_t(uint32_t)> codeXfer = nullptr;
        };

    private:
        XMLTypeset::InitArgs m_initArgs;

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

            std::tuple<int, int> tokenLoc() const noexcept
            {
                return {tokenX, tokenY};
            }

            std::tuple<int, int> maxHk() const noexcept
            {
                return {maxH1, maxH2};
            }
        };

    private:
        int m_px = 0;
        int m_py = 0;
        int m_pw = 0;
        int m_ph = 0;
        int m_fw = 0;
        int m_fh = 0;

    private:
        std::unique_ptr<XMLParagraph> m_paragraph;

    private:
        std::deque<contentLine> m_lineList;

    private:
        std::deque<LeafInfo> m_leafInfoList;

    public:
        XMLTypeset(XMLTypeset::InitArgs);

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
            m_fw = 0;
            m_fh = 0;
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
                m_fh = m_ph;
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
        bool locInToken(int, int, const TOKEN *, bool withPadding, bool withMaxH) const;

    public:
        std::tuple<int, int> locToken(int, int, bool withPadding, bool withMaxH) const;

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
            m_initArgs.font.id = font;
        }

        void setFontSize(uint8_t fontSize)
        {
            m_initArgs.font.size = fontSize;
        }

        void setFontStyle(uint8_t fontStyle)
        {
            m_initArgs.font.style = fontStyle;
        }

        void setFontColor(Widget::VarU32 fontColor)
        {
            m_initArgs.font.color = std::move(fontColor);
        }

        void setFontBGColor(Widget::VarU32 fontBGColor)
        {
            m_initArgs.font.bgColor = std::move(fontBGColor);
        }

        void setImageMaskColor(Widget::VarU32 imageMaskColor)
        {
            m_initArgs.imageMaskColor = std::move(imageMaskColor);
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
        void setLineTokenStartX(int);
        void setLineTokenStartY(int);

    private:
        int LineRawWidth(int, bool) const;

    private:
        int LineFullWidth(int) const;
        int LineTargetWidth() const;

    public:
        auto getToken(this auto && self, int argX, int argY)
        {
            if(!self.tokenLocValid(argX, argY)){
                throw fflpanic("invalid token location: ({}, {})", argX, argY);
            }
            return std::addressof(self.m_lineList[argY].content[argX]);
        }

    public:
        auto getLineBackToken(this auto && self, int argLine)
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
        auto getBackToken(this auto && self)
        {
            if(self.lineCount() == 0){
                throw fflpanic("empty board");
            }

            if(self.lineTokenCount(self.lineCount() - 1) == 0){
                throw fflpanic("invalie empty line: {}", self.lineCount() - 1);
            }

            return self.getLineBackToken(self.lineCount() - 1);
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
        void recalcLeafMaxHk(int, int);
        void buildTypeset(int, int);

    private:
        int lineReachMaxX(int, bool) const;
        int lineReachMaxY(int, bool) const;
        int lineReachMinX(int, bool) const;
        int lineReachMinY(int, bool) const;

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

        int fw() const
        {
            return m_fw;
        }

        int fh() const
        {
            return m_fh;
        }

    public:
        int LineMaxHk(int, int, bool) const;

    private:
        void LineJustifyPadding(int);
        void LineDistributedPadding(int);

    public:
        int lineAlign() const;

    public:
        int MaxLineWidth() const
        {
            return m_initArgs.lineWidth;
        }

        bool CanThrough() const
        {
            return m_initArgs.canThrough;
        }

    private:
        int LineNewStartY(int);
        int LineTokenBestY(int, int, int, int) const;
        int LineIntervalMaxH2(int, int, int) const;

    public:
        uint32_t color() const
        {
            return Widget::evalU32(m_initArgs.font.color, nullptr, this);
        }

        uint32_t bgColor() const
        {
            return Widget::evalU32(m_initArgs.font.bgColor, nullptr, this);
        }

    public:
        std::string getRawString() const
        {
            return m_paragraph->getRawString();
        }

        void setLineWidth(int, const std::array<int, 2> & = {0, 0});

    public:
        bool blankToken(int, int) const;

    public:
        std::tuple<int, int> getDefaultFontHk() const;
        std::tuple<int, int> getTokenCursorHk(int, int) const;

    public:
        int getDefaultFontHeight() const;

    public:
        void setCodeXferFunc(std::function<uint32_t(uint32_t)> codeXferFunc)
        {
            m_initArgs.codeXfer = std::move(codeXferFunc);
        }

    private:
        uint32_t codeXfer(uint32_t codePoint) const
        {
            return m_initArgs.codeXfer ? m_initArgs.codeXfer(codePoint) : codePoint;
        }

        uint64_t u64KeyXfer(uint64_t u64Key) const
        {
            return utf8f::exchangeCodePointInU64Key(u64Key, codeXfer(utf8f::codePointFromU64Key(u64Key)));
        }
};
