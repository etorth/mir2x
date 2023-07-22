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
#include "texslider.hpp"
#include "layoutboard.hpp"
#include "texaniboard.hpp"
#include "wmdaniboard.hpp"
#include "tritexbutton.hpp"
#include "alphaonbutton.hpp"

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
        class WidgetMiddleGroup: public WidgetContainer
        {
            public:
                WidgetMiddleGroup(int x, int y, int w, int h, Widget *parent = nullptr, bool autoDelete = false)
                    : WidgetContainer(DIR_UPLEFT, x, y, w, h, parent, autoDelete)
                {}

            public:
                void resetWidth(int width)
                {
                    m_w = width;
                }
        };

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
        WidgetContainer m_left;
        WidgetContainer m_right;

    private:
        WidgetMiddleGroup m_middle;

    private:
        TritexButton m_buttonQuickAccess;

    private:
        TritexButton m_buttonClose;
        TritexButton m_buttonMinize;

    private:
        AlphaOnButton m_buttonExchange;
        AlphaOnButton m_buttonMiniMap;
        AlphaOnButton m_buttonMagicKey;

    private:
        TritexButton m_buttonInventory;
        TritexButton m_buttonHeroState;
        TritexButton m_buttonHeroMagic;

    private:
        TritexButton m_buttonGuild;
        TritexButton m_buttonTeam;
        TritexButton m_buttonQuest;
        TritexButton m_buttonHorse;
        TritexButton m_buttonRuntimeConfig;
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
        TexSlider m_slider;

    private:
        InputLine m_cmdLine;

    private:
        LayoutBoard m_logBoard;

    private:
        double m_accuTime = 0.0;

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
        void drawEx(int, int, int, int, int, int) const override;

    private:
        static std::tuple<int, int> scheduleStretch(int, int);

    private:
        void drawHeroLoc() const;
        void drawRatioBar(int, int, double) const;

    private:
        void drawLeft() const;
        void drawRight() const;
        void drawFocusFace() const;
        void drawMiddleExpand() const;
        void drawMiddleDefault() const;
        void drawLogBoardExpand() const;
        void drawLogBoardDefault() const;
        void drawInputGreyBackground() const;

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

    private:
        void setButtonLoc();

    public:
        void onWindowResize(int, int);

    private:
        int logBoardStartY() const;

    public:
        TritexButton *getButton(const std::string &buttonName)
        {
            if(buttonName == "Inventory"){
                return &m_buttonInventory;
            }
            else{
                return nullptr;
            }
        }
};
