#pragma once
#include <list>
#include <array>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include "widget.hpp"
#include "xmltypeset.hpp"
#include "shapeclipboard.hpp"

class LayoutBoard: public Widget
{
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

            int startY = -1;
            std::array<int, 4> margin {0, 0, 0, 0};
            std::unique_ptr<XMLTypeset> tpset; // no copy support for XMLTypeset
        };

    private:
        std::list<ParNode> m_parNodeList;

    private:
        struct ParNodeConfig
        {
            // default setup of paragraph in LayoutBoard
            // may needed this to support theme

            int lineWidth;
            std::array<int, 4> margin;

            bool canThrough;

            uint8_t  font;
            uint8_t  fontSize;
            uint8_t  fontStyle;
            uint32_t fontColor;
            uint32_t fontBGColor;

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
        } m_cursorLoc;

        int m_cursorBlink = 0;
        ShapeClipBoard m_cursorClip;

    private:
        bool m_canSelect;
        bool m_canEdit;
        bool m_imeEnabled;

    private:
        int m_cursorWidth;
        uint32_t m_cursorColor;

    private:
        const std::function<void()> m_onTab;
        const std::function<void()> m_onCR;
        const std::function<void(const std::unordered_map<std::string, std::string> &, int)> m_eventCB;

    public:
        LayoutBoard(
                Widget::VarDir,
                Widget::VarOff,
                Widget::VarOff,

                int, // line width

                const char * = nullptr,
                size_t = 0,

                std::array<int, 4> = {0, 0, 0, 0},

                bool = false,
                bool = false,
                bool = false,
                bool = false,

                uint8_t  =  0,
                uint8_t  = 10,
                uint8_t  =  0,
                uint32_t =  colorf::WHITE + colorf::A_SHF(255),
                uint32_t =  0,

                int = LALIGN_LEFT,
                int = 0,
                int = 0,

                int      = 2,
                uint32_t = colorf::WHITE + colorf::A_SHF(255),

                std::function<void()> = nullptr,
                std::function<void()> = nullptr,
                std::function<void(const std::unordered_map<std::string, std::string> &, int)> = nullptr,

                Widget * = nullptr,
                bool     = false);

    public:
        void updateGfx();
        void loadXML(const char *, size_t = 0);

    public:
        void addParXML(int, const std::array<int, 4> &, const char *);

    public:
        void update(double ms) override
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
        int parCount() const { return to_d(m_parNodeList.size()); }
        int parCount()       { return to_d(m_parNodeList.size()); }

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
        void setFont(uint8_t font)
        {
            m_parNodeConfig.font = font;
        }

        void setFontSize(uint8_t fontSize)
        {
            m_parNodeConfig.fontSize = fontSize;
        }

        void setFontStyle(uint8_t fontStyle)
        {
            m_parNodeConfig.fontStyle = fontStyle;
        }

        void setFontColor(uint32_t fontColor)
        {
            m_parNodeConfig.fontColor = fontColor;
        }

        void setFontBGColor(uint32_t fontBGColor)
        {
            m_parNodeConfig.fontBGColor = fontBGColor;
        }

        void setLineWidth(int);

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEventDefault(const SDL_Event &, bool) override;

    private:
        void addPar(int, const std::array<int, 4> &, const tinyxml2::XMLNode *);

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

    public:
        Widget *setFocus(bool argFocus) override
        {
            Widget::setFocus(argFocus);
            m_cursorBlink = 0.0;
            return this;
        }

    public:
        static const char * findAttrValue(const std::unordered_map<std::string, std::string> &, const char *, const char * = nullptr);
};
