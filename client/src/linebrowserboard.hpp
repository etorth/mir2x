/*
 * =====================================================================================
 *
 *       Filename: linebrowserboard.hpp
 *        Created: 07/12/2017 23:09:28
 *  Last Modified: 01/14/2018 18:44:35
 *
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
#include <map>
#include <deque>
#include <string>
#include <functional>
#include "widget.hpp"
#include "tokenboard.hpp"

class LineBrowserBoard: public Widget
{
    protected:
        TokenBoard m_TokenBoard;

    protected:
        std::deque<std::string> m_XMLArray;

    public:
        LineBrowserBoard(
                int              nX,
                int              nY,
                int              nW,
                int              nH,
                bool             bSelectable,
                bool             bSpacePadding,
                bool             bCanThrough,
                int              nWordSpace      =  0,
                int              nLineSpace      =  0,
                uint8_t          nDefaultFont    =  0,
                uint8_t          nDefaultSize    =  10,
                uint8_t          nDefaultStyle   =  0,
                const SDL_Color &rstDefaultColor =  {0XFF, 0XFF, 0XFF, 0XFF},
                Widget          *pWidget         =  nullptr,
                bool             bFreeWidget     =  false)
            : Widget(nX, nY, nW, nH, pWidget, bFreeWidget)
            , m_TokenBoard(
                      nX,
                      nY,
                      bSelectable,
                      false,
                      bSpacePadding,
                      bCanThrough,
                      nW,
                      nWordSpace,
                      nLineSpace,
                      nDefaultFont,
                      nDefaultSize,
                      nDefaultStyle,
                      rstDefaultColor,
                      0,
                      0,
                      0,
                      0,
                      nullptr,
                      false)
            , m_XMLArray()
        {}

    public:
        ~LineBrowserBoard() = default;

    public:
        size_t Size() const
        {
            return m_XMLArray.size();
        }

    public:
        bool Add   (const char *);
        bool AddXML(const char *, const std::unordered_map<std::string, std::function<void()>> &);

    public:
        void DrawEx(int, int, int, int, int, int);

    public:
        const std::string &Print(size_t nIndex) const
        {
            return m_XMLArray[nIndex];
        }
};
