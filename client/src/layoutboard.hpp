#pragma once
#include <list>
#include <array>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include "widget.hpp"
#include "xmltypeset.hpp"
#include "gfxshapeboard.hpp"

class LayoutBoard: public Widget
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            int lineWidth = 0; // line width

            const char *initXML = nullptr;
            size_t parLimit = 0;

            // margin for par-node, not for the whole board
            // dynamic margin is not supported since which requires gfx-update
            Widget::IntMargin margin {};

            bool canSelect  = false;
            bool canEdit    = false;
            bool enableIME  = false;
            bool canThrough = false;

            Widget::FontConfig font {};

            int lineAlign = LALIGN_LEFT;
            int lineSpace = 0;
            int wordSpace = 0;

            Widget::CursorConfig cursor {};

            std::function<void()> onTab;
            std::function<void()> onCR;
            std::function<void()> onCursorMove;
            std::function<void(const std::unordered_map<std::string, std::string> &, int)> onClickText;

            Widget::WADPair parent {};
        };

    private:
        struct ParNode
        {
            // margin[0]  up
            // margin[1]  down
            // margin[2]  left
            // margin[3]  right

            // x---------+  x: (0, 0)
            // | C-----+ |  C: (margin[2], startY), margin not included
            // | |*****| |
            // | +-----+ |
            // +---------+

            int startY {};
            Widget::IntMargin margin {};
            std::unique_ptr<XMLTypeset> tpset {}; // no copy support for XMLTypeset
        };

    private:
        std::list<ParNode> m_parNodeList;

    private:
        struct ParNodeConfig
        {
            // default setup of paragraph in LayoutBoard
            // may needed this to support theme

            int lineWidth;
            Widget::IntMargin margin;

            bool canThrough;

            uint8_t font;
            uint8_t fontSize;
            uint8_t fontStyle;

            Widget::VarU32 fontColor;
            Widget::VarU32 fontBGColor;

            int align;
            int lineSpace;
            int wordSpace;

        } m_parNodeConfig;

    private:
        struct ParCursorLoc
        {
            int par = -1;
            int x   = -1;
            int y   = -1;
        }
        m_cursorLoc;

        int m_cursorBlink = 0;
        GfxShapeBoard m_cursorClip;

    private:
        bool m_canSelect;
        bool m_canEdit;
        bool m_imeEnabled;

    private:
        int m_cursorWidth;
        Widget::VarU32 m_cursorColor;

    private:
        const std::function<void()> m_onTab;
        const std::function<void()> m_onCR;
        const std::function<void()> m_onCursorMove; // after cursor moved
        const std::function<void(const std::unordered_map<std::string, std::string> &, int)> m_eventCB;

    public:
        LayoutBoard(LayoutBoard::InitArgs);

    public:
        void updateGfx();
        void loadXML(const char *, size_t = 0);

    public:
        void   addParXML   (int, const Widget::IntMargin &, const char *);
        size_t addLayoutXML(int, const Widget::IntMargin &, const char *, size_t = 0);

    public:
        void updateDefault(double ms) override
        {
            for(const auto &node: m_parNodeList){
                node.tpset->update(ms);
            }
            m_cursorBlink += ms;
        }

    private:
        std::list<ParNode>::iterator ithParIterator(int i)
        {
            auto p = m_parNodeList.begin();
            std::advance(p, i);
            return p;
        }

        std::list<ParNode>::const_iterator ithParIterator(int i) const
        {
            auto p = m_parNodeList.begin();
            std::advance(p, i);
            return p;
        }

    public:
        bool cursorLocValid(int argPar, int argX, int argY) const
        {
            return argPar >= 0 && argPar < parCount() && ithParIterator(argPar)->tpset->cursorLocValid(argX, argY);
        }

        bool cursorLocValid(const ParCursorLoc &parCursorLoc) const
        {
            return cursorLocValid(parCursorLoc.par, parCursorLoc.x, parCursorLoc.y);
        }

    public:
        int parCount() const
        {
            return to_d(m_parNodeList.size());
        }

    public:
        bool empty() const
        {
            return m_parNodeList.empty();
        }

        bool hasToken() const
        {
            for(const auto &par: m_parNodeList){
                for(int i = 0; i < par.tpset->lineCount(); ++i){
                    if(par.tpset->lineTokenCount(i) > 0){
                        return true;
                    }
                }
            }
            return false;
        }

    public:
        void clear()
        {
            m_parNodeList.clear();
            if(m_canEdit){
                loadXML("<layout><par></par></layout>");
                m_cursorLoc.par = 0;
                m_cursorLoc.x   = 0;
                m_cursorLoc.y   = 0;
            }
        }

    public:
        void setFont(uint8_t);
        void setFontSize(uint8_t);
        void setFontStyle(uint8_t);
        void setFontColor(Widget::VarU32);
        void setFontBGColor(Widget::VarU32);

    public:
        void setLineWidth(int);

    public:
        void drawDefault(Widget::ROIMap) const override;

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    private:
        void addPar(int, const Widget::IntMargin &, const tinyxml2::XMLNode *);

    private:
        void setupStartY(int);

    public:
        int getLineWidth() const
        {
            return m_parNodeConfig.lineWidth;
        }

    private:
        void drawCursorBlink(int, int) const;

    public:
        std::string getXML() const;
        std::string getText() const;

    public:
        void setFocus(bool argFocus) override
        {
            Widget::setFocus(argFocus);
            m_cursorBlink = 0.0;
        }

    public:
        static const char * findAttrValue(const std::unordered_map<std::string, std::string> &, const char *, const char * = nullptr);

    public:
        std::tuple<int, int          > getCursorOff() const;  // -> cursor: par, off
        std::tuple<int, int, int     > getCursorLoc() const;  // -> cursor: par, x, y
        std::tuple<int, int, int, int> getCursorPLoc() const; // -> cursor: ROI
};
