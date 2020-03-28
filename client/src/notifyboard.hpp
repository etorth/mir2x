/*
 * =====================================================================================
 *
 *       Filename: notifyboard.hpp
 *        Created: 02/13/2018 19:07:00
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
#include <queue>
#include <array>
#include "tokenboard.hpp"

class NotifyBoard: public Widget
{
    protected:
        struct LogLine
        {
            uint32_t LineCount;
            uint32_t ExpireTime;
        };

    protected:
        std::queue<LogLine> m_LogQueue;

    protected:
        TokenBoard m_LogBoard;

    public:
        NotifyBoard()
            : Widget(0, 0, 0, 0, nullptr, false)
            , m_LogBoard
              {
                  0,
                  0,
                  false,
                  false,
                  false,
                  false,
                 -1,
                  0,
                  0,
                  0,
                  16,
                  0,
                  {0XFF, 0X00, 0X00, 0X00},
                  0,
                  0,
                  0,
                  0,
                  nullptr,
                  false
              }
        {}

    public:
        virtual ~NotifyBoard() = default;

    protected:
        void Pop();

    public:
        void Update(double);

    public:
        bool processEvent(const SDL_Event &event, bool valid)
        {
            return m_LogBoard.processEvent(event, valid);
        }

    public:
        void AddLog(std::array<std::string, 4>, const char *, ...);
        void AddXML(const char *szXML, const std::map<std::string, std::function<void()>> &rstMap = {});

    public:
        void drawEx(int, int, int, int, int, int);
};
