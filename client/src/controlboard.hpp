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
#include "acbutton.hpp"
#include "sdldevice.hpp"
#include "inputline.hpp"
#include "layoutboard.hpp"
#include "texaniboard.hpp"
#include "wmdaniboard.hpp"
#include "tritexbutton.hpp"
#include "alphaonbutton.hpp"
#include "quickaccessbutton.hpp"

enum
{
    CBLOG_DEF = 0,
    CBLOG_SYS,
    CBLOG_DBG,
    CBLOG_ERR,
};

class ProcessRun;
class ControlBoard: public Widget
{
    private:
        ProcessRun *m_processRun;

    private:
        bool m_expand = false;

    private:
        int m_stretchH = 196;
        const int m_stretchHMin = 196; // only can get bigger than original frame

    private:
        // define group for widget moving
        // but will not call widgetGroup::drawEx, we still draw manually
        WidgetGroup m_left;
        WidgetGroup m_middle;
        WidgetGroup m_right;

    private:
        QuickAccessButton m_buttonQuickAccess;

    private:
        TritexButton m_buttonClose;
        TritexButton m_buttonMinize;

    private:
        AlphaOnButton m_buttonExchange;
        AlphaOnButton m_buttonMiniMap;
        AlphaOnButton m_buttonMagicKey;

    private:
        TritexButton m_buttonInventory;
        TritexButton m_buttonHeroStatus;
        TritexButton m_buttonHeroMagic;

    private:
        TritexButton m_buttonGuild;
        TritexButton m_buttonTeam;
        TritexButton m_buttonTask;
        TritexButton m_buttonHorse;
        TritexButton m_buttonEnvConfig;
        TritexButton m_buttonSysMessage;

    private:
        ACButton m_buttonAC;
        ACButton m_buttonDC;

    private:
        TritexButton m_buttonSwitchMode;

    private:
        TritexButton m_buttonEmoji;
        TritexButton m_buttonMute;

    private:
        LevelBox m_levelBox;
        TexAniBoard m_arcAniBoard;

    private:
        InputLine  m_cmdLine;
        LabelBoard m_locBoard;

    private:
        LayoutBoard m_logBoard;

    public:
        ControlBoard(
                int,    // boardW
                int,    // startY
                ProcessRun *,
                Widget *pwidget = nullptr,
                bool autoDelete = false);

    public:
        ~ControlBoard() = default;

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
        void drawInputGreyBackground();

    public:
        void update(double) override;
        bool processEvent(const SDL_Event &, bool) override;

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
        // we don't support widget::resize()
        // widget doesn't have box concept, parent widget can't calculate proper size for children
        void resizeWidth(int w);

    private:
        int logBoardStartY() const;
};
