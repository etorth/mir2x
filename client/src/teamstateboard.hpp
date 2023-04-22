#pragma once
#include "widget.hpp"
#include "sysconst.hpp"
#include "labelboard.hpp"
#include "protocoldef.hpp"
#include "tritexbutton.hpp"

class ProcessRun;
class TeamStateBoard: public Widget
{
    private:
        TritexButton m_enableTeam;

    private:
        TritexButton m_createTeam;
        TritexButton m_addMember;
        TritexButton m_deleteMember;
        TritexButton m_refresh;

    private:
        TritexButton m_close;

    private:
        ProcessRun *m_processRun;

    public:
        TeamStateBoard(int, int, ProcessRun *, Widget * = nullptr, bool = false);

    public:
        void update(double) override;

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEvent(const SDL_Event &, bool) override;
};
