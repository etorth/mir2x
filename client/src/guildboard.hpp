#pragma once
#include "widget.hpp"
#include "serdesmsg.hpp"
#include "texslider.hpp"
#include "imageboard.hpp"
#include "tritexbutton.hpp"

class ProcessRun;
class GuildBoard: public Widget
{
    private:
        ProcessRun *m_processRun;

    private:
        ImageBoard m_bg;

    private:
        TritexButton m_closeButton;

    private:
        TritexButton m_announcement;
        TritexButton m_members;
        TritexButton m_chat;
        TritexButton m_editAnnouncement;
        TritexButton m_removeMember;
        TritexButton m_disbandGuild;
        TritexButton m_editMemberPosition;
        TritexButton m_dissolveCovenant;

    private:
        TexSlider m_slider;

    public:
        GuildBoard(int, int, ProcessRun *, Widget * = nullptr, bool = false);

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;
};
