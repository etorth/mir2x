#pragma once
#include <string>
#include <cstdint>
#include <cstddef>
#include <optional>
#include "widget.hpp"
#include "serdesmsg.hpp"
#include "texslider.hpp"
#include "inputline.hpp"
#include "textboard.hpp"
#include "textbutton.hpp"
#include "imageboard.hpp"
#include "layoutboard.hpp"
#include "tritexbutton.hpp"
#include "gfxcropboard.hpp"
#include "gfxshapeboard.hpp"

class ProcessRun;
class GuildBoard: public Widget
{
    private:
        enum PageType: int
        {
            PAGE_NOTICE,
            PAGE_MEMBER,
            PAGE_CHAT,
        };

    private:
        constexpr static int m_textX = 20;
        constexpr static int m_textY = 44;
        constexpr static int m_textW = 526;

        constexpr static int m_noticeH = 296;
        constexpr static int m_chatH   = 284;

        constexpr static int m_chatInputY = 336;
        constexpr static int m_chatInputH =  18;

        constexpr static int m_memberX = 14;
        constexpr static int m_memberY = 40;
        constexpr static int m_memberW = 538;

        constexpr static int m_memberHeaderH   = 22;
        constexpr static int m_memberRowH      = 20;
        constexpr static int m_memberRowCount  = 13;
        constexpr static int m_memberListH     = m_memberHeaderH + m_memberRowCount * m_memberRowH;

        constexpr static int    m_chatLineCount   = 200;
        constexpr static size_t m_chatMessageSize = 128;

    private:
        ProcessRun *m_processRun;

    private:
        SDGuildInfo m_guild;
        PageType m_page = PAGE_NOTICE;

        uint32_t m_selectedDBID = 0;
        bool m_editNotice = false;

    private:
        ImageBoard m_bg;
        TextBoard  m_guildName;
        TextBoard  m_noGuild;

    private:
        GfxCropBoard m_noticeView;
        LayoutBoard  m_noticeBoard;

        GfxShapeBoard m_noticeEditFrame;
        GfxCropBoard  m_noticeEditView;
        LayoutBoard   m_noticeEditBoard;
        TextBoard     m_noticeEditHint;

    private:
        Widget     m_memberList;
        TextButton m_invite;

    private:
        GfxCropBoard  m_chatView;
        LayoutBoard   m_chatBoard;
        GfxShapeBoard m_chatInputBg;
        InputLine     m_chatInput;

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

    public:
        void setGuildInfo(SDGuildInfo);
        void addChatMessage(const std::string &, const std::string &);

    private:
        bool pageVisible(PageType) const;
        void setPage(PageType);

    private:
        const SDGuildMember *findMember(uint64_t) const;
        bool isLeader() const;

    private:
        bool checkGuild() const;  // log error and return false if not in a guild
        bool checkLeader() const; // ....                          if not the leader

    private:
        int scrollOffset(int) const;
        int scrollMax() const;
        std::optional<size_t> memberIndex(int) const;

    private:
        void loadNotice();
        void editNotice();
        void stopEditNotice();

    private:
        void sendChat();

    private:
        void inviteMember();
        void removeMember();
        void leaveGuild();
        void editMemberPosition();
        void editMemberTitle(uint32_t);
        void transferLeader(uint32_t, const std::string &);
};
