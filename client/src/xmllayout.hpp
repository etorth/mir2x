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
        int m_W;

    private:
        uint8_t m_DefaultFont;
        uint8_t m_DefaultFontSize;
        uint8_t m_DefaultFontStyle;

    private:
        uint32_t m_DefaultFontColor;

    public:
        XMLLayout(
                int              nW,
                uint8_t          nDefaultFont      = 0,
                uint8_t          nDefaultFontSize  = 10,
                uint8_t          nDefaultFontStyle = 0,
                uint32_t         nDefaultFontColor = ColorFunc::WHITE + 255)
            : m_W(nW)
            , m_DefaultFont(nDefaultFont)
            , m_DefaultFontSize(nDefaultFontSize)
            , m_DefaultFontStyle(nDefaultFontStyle)
            , m_DefaultFontColor(nDefaultFontColor)
        {}

    public:
        ~XMLLayout() = default;

    public:
        void addLog(const char *, ...);

    public:
        void SetFont(uint8_t nFont)
        {
            m_DefaultFont = nFont;
        }

        void SetFontSize(uint8_t nFontSize)
        {
            m_DefaultFontSize = nFontSize;
        }

        void SetFontStyle(uint8_t nFontStyle)
        {
            m_DefaultFontStyle = nFontStyle;
        }

        void SetFontColor(uint32_t nFontColor)
        {
            m_DefaultFontColor = nFontColor;
        }

    public:
        void Clear()
        {
            m_parNodeList.clear();
        }

    public:
        int PW();

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
        void addPar(int, const std::array<int, 4> &, size_t, int, bool, size_t, size_t, const tinyxml2::XMLNode *);

    public:
        int W() const
        {
            return m_W;
        }

        int H() const
        {
            if(empty()){
                return 0;
            }

            const auto &backNode = m_parNodeList.back();
            return backNode.startY + backNode.margin[0] + backNode.margin[1] + backNode.tpset->PH();
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
