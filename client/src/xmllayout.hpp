/*
 * =====================================================================================
 *
 *       Filename: xmllayout.hpp
 *        Created: 12/11/2018 04:48:41
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
#include <list>
#include <array>
#include <vector>
#include <memory>
#include "xmltypeset.hpp"

class XMLLayout
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
        int m_w;

    private:
        std::array<int, 4> m_margin;

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

    private:
        int m_lineSpace;
        int m_wordSpace;

    public:
        XMLLayout(
                int                w,
                std::array<int, 4> margin     = {0, 0, 0, 0},
                bool               canThrough = false,
                uint8_t            font       =  0,
                uint8_t            fontSize   = 10,
                uint8_t            fontStyle  =  0,
                uint32_t           fontColor  =  ColorFunc::WHITE + 255,
                int                lineAlign  =  LALIGN_LEFT,
                int                lineSpace  =  0,
                int                wordSpace  =  0)
            : m_w(w)
            , m_margin(margin)
            , m_canThrough(canThrough)
            , m_font(font)
            , m_fontSize(fontSize)
            , m_fontStyle(fontStyle)
            , m_fontColor(fontColor)
            , m_align(lineAlign)
            , m_lineSpace(lineSpace)
            , m_wordSpace(wordSpace)
        {
            if(m_w <= m_margin[2] + m_margin[3]){
                throw fflerror("default margin takes all room of width");
            }
        }

    public:
        ~XMLLayout() = default;

    public:
        void addLog(const char *, ...);

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

    public:
        void Clear()
        {
            m_parNodeList.clear();
        }

    public:
        int pw();

    public:
        void drawEx(int, int, int, int, int, int);

    public:
        int parCount() const
        {
            return (int)(m_parNodeList.size());
        }

        bool empty() const
        {
            return m_parNodeList.empty();
        }

    public:
        void addPar(int loc, const tinyxml2::XMLNode * node)
        {
            addPar(loc, m_margin, node);
        }

    public:
        void addPar(int, const std::array<int, 4> &, const tinyxml2::XMLNode *);

    public:
        int W() const
        {
            return m_w;
        }

        int H() const
        {
            if(empty()){
                return 0;
            }

            const auto &backNode = m_parNodeList.back();
            return backNode.startY + backNode.margin[0] + backNode.margin[1] + backNode.tpset->ph();
        }

    private:
        auto ithIterator(int i)
        {
            auto p = m_parNodeList.begin();
            std::advance(p, i);
            return p;
        }

    public:
        void loadXML(const char *);

    public:
        void update(double);
};
