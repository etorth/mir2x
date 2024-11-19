#pragma once
#include <tuple>
#include <deque>
#include <memory>
#include "token.hpp"
#include "lalign.hpp"
#include "xmlf.hpp"
#include "fflerror.hpp"
#include "colorf.hpp"
#include "bevent.hpp"
#include "xmlparagraph.hpp"

class XMLTypeset // means XMLParagraph typeset
{
    private:
        struct contentLine
        {
            int startY;
            std::deque<TOKEN> content;
        };

    private:
        int m_lineWidth;

    private:
        const int m_lineAlign;

    private:
        const bool m_canThrough;

    private:
        int m_wordSpace;
        int m_lineSpace;

    private:
        uint8_t  m_font;
        uint8_t  m_fontSize;
        uint8_t  m_fontStyle;
        uint32_t m_fontColor;
        uint32_t m_fontBGColor;
        uint32_t m_imageMaskColor;

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
        std::deque<std::tuple<int, int>> m_leaf2TokenLoc;

    public:
        XMLTypeset(
                int      maxLineWidth,
                int      lineAlign             = LALIGN_LEFT,
                bool     canThrough            = true,
                uint8_t  defaultFont           = 0,
                uint8_t  defaultFontSize       = 8,
                uint8_t  defaultFontStyle      = 0,
                uint32_t defaultFontColor      = colorf::WHITE + colorf::A_SHF(255),
                uint32_t defaultFontBGColor    = 0,
                uint32_t defaultImageMaskColor = colorf::RGBA(0XFF, 0XFF, 0XFF, 0XFF),
                int      lineSpace             = 0,
                int      wordSpace             = 0)
            : m_lineWidth(maxLineWidth)
            , m_lineAlign(lineAlign)
            , m_canThrough(canThrough)
            , m_wordSpace(wordSpace)
            , m_lineSpace(lineSpace)
            , m_font(defaultFont)
            , m_fontSize(defaultFontSize)
            , m_fontStyle(defaultFontStyle)
            , m_fontColor(defaultFontColor)
            , m_fontBGColor(defaultFontBGColor)
            , m_imageMaskColor(defaultImageMaskColor)
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

        bool lineEmpty(int line) const
        {
            return m_lineList.at(line).content.empty();
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
            if(lineValid(argLine)){
                return m_lineList[argLine].content.size();
            }
            throw fflerror("invalid line specified: %d >= %d", argLine, lineCount());
        }

    public:
        std::tuple<int, int> prevTokenLoc(int, int) const;
        std::tuple<int, int> nextTokenLoc(int, int) const;

    public:
        std::tuple<int, int> prevCursorLoc(int, int) const;
        std::tuple<int, int> nextCursorLoc(int, int) const;

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
                throw fflerror("empty typeset");
            }
            return {0, 0};
        }

        std::tuple<int, int> lastTokenLoc() const
        {
            if(empty()){
                throw fflerror("empty board");
            }
            return {lineTokenCount(lineCount() - 1) - 1, lineCount() - 1};
        }

    public:
        std::tuple<int, int> firstCursorLoc() const
        {
            if(empty()){
                return {0, 0};
            }
            return {1, 0};
        }

        std::tuple<int, int> lastCursorLoc() const
        {
            if(empty()){
                return {0, 0};
            }
            return {lineTokenCount(lineCount() - 1), lineCount() - 1};
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
        bool tokenLocValid(int argX, int argY) const
        {
            return lineValid(argY) && (argX >= 0) && (argX < lineTokenCount(argY));
        }

        bool cursorLocValid(int argX, int argY) const
        {
            if(empty()){
                return argX == 0 && argY == 0;
            }
            else if(argY == 0){
                return (argX >= 0) && (argX <= lineTokenCount(argY));
            }
            else{
                return lineValid(argY) && (argX >= 1) && (argX <= lineTokenCount(argY));
            }
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

    public:
        void deleteToken(int, int, int);

    public:
        int leafCount() const
        {
            return m_paragraph->leafCount();
        }

        bool leafValid(int leaf) const
        {
            return leaf < leafCount();
        }

    public:
        void clearEvent(int currLeaf = -1)
        {
            for(int leaf = 0; leaf < m_paragraph->leafCount(); ++leaf){
                if(leaf != currLeaf){
                    m_paragraph->leafRef(leaf).markEvent(BEVENT_OFF);
                }
            }
        }

        int markLeafEvent(int leaf, int event)
        {
            return m_paragraph->leafRef(leaf).markEvent(event);
        }

    public:
        void drawEx(int, int, int, int, int, int) const;

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

        void setFontColor(uint32_t fontColor)
        {
            m_fontColor = fontColor;
        }

        void setFontBGColor(uint32_t fontBGColor)
        {
            m_fontBGColor = fontBGColor;
        }

        void setImageMaskColor(uint32_t imageMaskColor)
        {
            m_imageMaskColor = imageMaskColor;
        }

    public:
        std::string getXML() const
        {
            return m_paragraph->getXML();
        }

    public:
        std::string getText(bool) const;

    public:
        const tinyxml2::XMLNode *getXMLNode() const
        {
            return m_paragraph->getXMLNode();
        }

    public:
        const auto leafEvent(int leafID) const
        {
            return m_paragraph->leafRef(leafID).hasEvent();
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
        const TOKEN *getToken(int argX, int argY) const
        {
            if(!tokenLocValid(argX, argY)){
                throw fflerror("invalid token location: (%d, %d)", argX, argY);
            }
            return &(m_lineList[argY].content[argX]);
        }

        TOKEN *getToken(int argX, int argY)
        {
            return const_cast<TOKEN *>(static_cast<const XMLTypeset *>(this)->getToken(argX, argY));
        }

    public:
        const TOKEN *GetLineBackToken(int argLine) const
        {
            if(!lineValid(argLine)){
                throw fflerror("invalid line: %d", argLine);
            }

            if(lineTokenCount(argLine) == 0){
                throw fflerror("invalie empty line: %d", argLine);
            }

            return getToken(lineTokenCount(argLine) - 1, argLine);
        }

        TOKEN *GetLineBackToken(int argLine)
        {
            return const_cast<TOKEN *>(static_cast<const XMLTypeset *>(this)->GetLineBackToken(argLine));
        }

    public:
        const TOKEN *GetBackToken() const
        {
            if(lineCount() == 0){
                throw fflerror("empty board");
            }

            if(lineTokenCount(lineCount() - 1) == 0){
                throw fflerror("invalie empty line: %d", lineCount() - 1);
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
        std::vector<TOKEN> createTokenLine(int, int, std::vector<TOKEN> * = nullptr) const;

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
            return m_fontColor;
        }

        uint32_t bgColor() const
        {
            return m_fontBGColor;
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
        int getDefaultFontHeight() const;
};
