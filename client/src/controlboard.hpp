/*
 * =====================================================================================
 *
 *       Filename: controlboard.hpp
 *        Created: 08/21/2016 04:12:57
 *    Description: main control pannel for running client
 *                 try support dynamically allocated control board
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
#include <cstdint>
#include <functional>

#include "log.hpp"
#include "widget.hpp"
#include "pngtexdb.hpp"
#include "levelbox.hpp"
#include "sdldevice.hpp"
#include "inputline.hpp"
#include "layoutboard.hpp"
#include "tritexbutton.hpp"

class ProcessRun;
class controlBoard: public widget
{
    private:
        ProcessRun *m_ProcessRun;

    private:
        bool m_expand = false;

    private:
        int m_stretchH = 196;
        const int m_stretchHMin = 196; // only can get bigger than original frame

    private:
        TritexButton m_ButtonClose;
        TritexButton m_ButtonMinize;
        TritexButton m_ButtonInventory;

    private:
        TritexButton m_buttonSwitchMode;

    private:
        levelBox m_level;

    private:
        inputLine        m_cmdLine;
        labelBoard       m_LocBoard;

    private:
        layoutBoard m_logBoard;

    public:
        controlBoard(
                int,            // x
                int,            // y
                int,            // screen width
                ProcessRun *,   // 
                widget *,       //
                bool);          //

    public:
        ~controlBoard() = default;

    public:
        void drawEx(int, int, int, int, int, int);

    private:
        static std::tuple<int, int> scheduleStretch(int, int);

    private:
        void drawLeft();
        void drawRight();
        void drawMiddleExpand();
        void drawMiddleDefault();
        void drawLogBoardExpand();
        void drawLogBoardDefault();

    public:
        void Update(double);
        bool processEvent(const SDL_Event &, bool);

    public:
        void inputLineDone();

    public:
        void addLog(int, const char *);

    public:
        bool CheckMyHeroMoved();

    private:
        void switchExpandMode();
        void setButtonLoc();

    public:
        // TODO: we don't support widget::resize()
        // widget doesn't have box concept, parent widget can't calculate proper size for children
        void resizeWidth(int w)
        {
            m_w = w;
        }

    private:
        int logBoardStartY() const;
};
