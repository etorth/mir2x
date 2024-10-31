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
                Widget::VarDir    argDir,
                Widget::VarOff argX,
                Widget::VarOff argY,

                int argLineWidth,

                const char *argInitXML = nullptr,
                size_t argParLimit = 0,

                std::array<int, 4> argMargin = {0, 0, 0, 0},

                bool argCanSelect  = false,
                bool argCanEdit    = false,
                bool argIMEEnabled = false,
                bool argCanThrough = false,

                uint8_t  argFont        =  0,
                uint8_t  argFontSize    = 10,
                uint8_t  argFontStyle   =  0,
                uint32_t argFontColor   =  colorf::WHITE + colorf::A_SHF(255),
                uint32_t argFontBGColor =  0,

                int argLineAlign = LALIGN_LEFT,
                int argLineSpace = 0,
                int argWordSpace = 0,

                int      argCursorWidth = 2,
                uint32_t argCursorColor = colorf::WHITE + colorf::A_SHF(255),

                std::function<void()> argOnTab = nullptr,
                std::function<void()> argOnCR  = nullptr,
                std::function<void(const std::unordered_map<std::string, std::string> &, int)> argEventCB = nullptr,

                Widget *argParent     = nullptr,
                bool    argAutoDelete = false)

            : Widget
              {
                  std::move(argDir),
                  std::move(argX),
                  std::move(argY),

                  [this](const Widget *)
                  {
                      int maxW = 0;
                      for(const auto &node: m_parNodeList){
                          maxW = std::max<int>(maxW, node.margin[2] + node.tpset->pw() + node.margin[3]);
                      }

                      if(m_canEdit){
                          maxW = std::max<int>(maxW, m_cursorWidth);
                      }

                      return maxW;
                  },

                  [this](const Widget *)
                  {
                      if(empty()){
                          if(m_canEdit){
                              throw fflerror("editable layout shall have at least one par");
                          }
                          return 0;
                      }

                      const auto &backNode = m_parNodeList.back();
                      return backNode.startY + std::max<int>(backNode.tpset->ph(), m_canEdit ? backNode.tpset->getDefaultFontHeight() : 0) + backNode.margin[1];
                  },

                  {},

                  argParent,
                  argAutoDelete,
              }

            , m_parNodeConfig
              {
                  argLineWidth,
                  argMargin,
                  argCanThrough,
                  argFont,
                  argFontSize,
                  argFontStyle,
                  argFontColor,
                  argFontBGColor,
                  argLineAlign,
                  argLineSpace,
                  argWordSpace,
              }

            , m_cursorClip
              {
                  DIR_UPLEFT,
                  0,
                  0,

                  [this](const Widget *){ return this->w(); },
                  [this](const Widget *){ return this->h(); },

                  [this](const Widget *, int drawDstX, int drawDstY)
                  {
                      drawCursorBlink(drawDstX, drawDstY);
                  },

                  this,
                  false,
              }

            , m_canSelect(argCanSelect)
            , m_canEdit(argCanEdit)
            , m_imeEnabled(argIMEEnabled)

            , m_cursorWidth(argCursorWidth)
            , m_cursorColor(argCursorColor)

            , m_onTab(std::move(argOnTab))
            , m_onCR(std::move(argOnCR))
            , m_eventCB(std::move(argEventCB))
        {
            for(size_t i = 0; i < m_parNodeConfig.margin.size(); ++i){
                if(m_parNodeConfig.margin[i] < 0){
                    throw fflerror("invalid parNodeConfig::margin[%zu]: %d", i, m_parNodeConfig.margin[i]);
                }
            }

            if((m_parNodeConfig.lineWidth > 0) && (m_parNodeConfig.lineWidth <= m_parNodeConfig.margin[2] + m_parNodeConfig.margin[3])){
                throw fflerror("invalid default paragraph parameters");
            }

            if(argInitXML){
                loadXML(argInitXML, argParLimit);
            }

            if(m_canEdit){
                if(empty()){
                    loadXML("<layout><par></par></layout>");
                }

                m_cursorLoc.par = parCount() - 1;
                std::tie(m_cursorLoc.x, m_cursorLoc.y) = m_parNodeList.rbegin()->tpset->lastCursorLoc();
            }
        }

    public:
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
