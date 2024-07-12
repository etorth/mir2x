#pragma once
#include "widget.hpp"
#include "serdesmsg.hpp"
#include "texslider.hpp"
#include "tritexbutton.hpp"

class ProcessRun;
class QuestStateBoard: public Widget
{
    private:
        struct QuestDespState
        {
            bool folded = true;
            std::map<std::string, std::string> desp {}; // fsm desp
        };

    private:
        bool m_left = true;
        bool m_loadRequested = false;

    private:
        const int m_despX =  40;
        const int m_despY = 100;
        const int m_despW = 270;
        const int m_despH = 300;

    private:
        LayoutBoard m_despBoard;

    private:
        TexSlider m_slider;

    private:
        TritexButton m_lrButton;
        TritexButton m_closeButton;

    private:
        ProcessRun *m_processRun;

    private:
        std::map<std::string, QuestDespState> m_questDesp;

    public:
        QuestStateBoard(int, int, ProcessRun *, Widget * = nullptr, bool = false);

    public:
        void update(double) override;

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEvent(const SDL_Event &, bool) override;

    public:
        void updateQuestDesp(SDQuestDespUpdate);
        void setQuestDesp(SDQuestDespList);

    private:
        void loadQuestDesp();
};
