/*
 * =====================================================================================
 *
 *       Filename: layoutboard.hpp
 *        Created: 03/25/2020 22:27:45
 *    Description: 
 *                  LayoutBoard doesn't have width configuration in the constructor
 *                  it's conceptually a container of XMLTypeset, every typeset has its own line width setup
 *
 *                  the w()/h() returns actual pixel size of it
 *                  for a line-width configurable LayoutBoard, try LayoutViewBoard, it supports cropping
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
#include <list>
#include <array>
#include <vector>
#include <memory>
#include "widget.hpp"
#include "xmltypeset.hpp"

class LayoutBoard: public Widget
{
    private:
        struct parNode
        {
            // margin[0]  up
            // margin[1]  down
            // margin[2]  left
            // margin[3]  right

            int startY = -1;
            std::array<int, 4> margin {0, 0, 0, 0};
            std::unique_ptr<XMLTypeset> tpset; // no copy support for XMLTypeset
        };

    private:
        std::list<parNode> m_parNodeList;

    private:
        int m_parLineWidth;
        std::array<int, 4> m_parMargin;

    private:
        bool m_canThrough;

    private:
        uint8_t m_font;
        uint8_t m_fontSize;
        uint8_t m_fontStyle;

    private:
        uint32_t m_fontColor;

    private:
        int m_align;
        int m_lineSpace;
        int m_wordSpace;

    private:
        bool m_canSelect;

    public:
        LayoutBoard(
                int                x,
                int                y,
                int                parLineWidth,
                bool               canSelect  =  false,
                std::array<int, 4> parMargin  =  {0, 0, 0, 0},
                bool               canThrough =  false,
                uint8_t            font       =  0,
                uint8_t            fontSize   = 10,
                uint8_t            fontStyle  =  0,
                uint32_t           fontColor  =  colorf::WHITE + 255,
                int                lineAlign  =  LALIGN_LEFT,
                int                lineSpace  =  0,
                int                wordSpace  =  0,
                Widget            *parent     =  nullptr,
                bool               autoDelete =  false)
            : Widget(x, y, 0, 0, parent, autoDelete)
            , m_parLineWidth(parLineWidth)
            , m_parMargin(parMargin)
            , m_canThrough(canThrough)
            , m_font(font)
            , m_fontSize(fontSize)
            , m_fontStyle(fontStyle)
            , m_fontColor(fontColor)
            , m_align(lineAlign)
            , m_lineSpace(lineSpace)
            , m_wordSpace(wordSpace)
            , m_canSelect(canSelect)
        {
            if(m_parLineWidth <= m_parMargin[2] + m_parMargin[3]){
                throw fflerror("invalid default paragraph parameters");
            }
        }

    public:
        void loadXML(const char *);

    public:
        void addParXML(int, const std::array<int, 4> &, const char *);

    public:
        void update(double ms) override
        {
            for(const auto &node: m_parNodeList){
                node.tpset->update(ms);
            }
        }

    public:
        int parCount() const
        {
            return (int)(m_parNodeList.size());
        }

    private:
        auto ithParIterator(int i)
        {
            auto p = m_parNodeList.begin();
            std::advance(p, i);
            return p;
        }

    public:
        bool empty() const
        {
            return m_parNodeList.empty();
        }

        void clear()
        {
            m_parNodeList.clear();
        }

    public:
        void SetFont(uint8_t nFont)
        {
            m_font = nFont;
        }

        void SetFontSize(uint8_t nFontSize)
        {
            m_fontSize = nFontSize;
        }

        void SetFontStyle(uint8_t nFontStyle)
        {
            m_fontStyle = nFontStyle;
        }

        void SetFontColor(uint32_t nFontColor)
        {
            m_fontColor = nFontColor;
        }

        void setLineWidth(int)
        {
        }

    public:
        void drawEx(int, int, int, int, int, int) override;

    private:
        void setupSize();
        void addPar(int, const std::array<int, 4> &, const tinyxml2::XMLNode *, bool);
};
