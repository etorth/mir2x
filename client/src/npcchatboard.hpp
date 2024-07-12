#pragma once
#include <cstdint>
#include "widget.hpp"
#include "layoutboard.hpp"
#include "tritexbutton.hpp"

class ProcessRun;
class NPCChatBoard: public Widget
{
    private:
        int m_margin;
        ProcessRun *m_process;

    private:
        LayoutBoard  m_chatBoard;
        TritexButton m_buttonClose;

    private:
        uint64_t m_npcUID;
        std::string m_eventPath;

    public:
        NPCChatBoard(ProcessRun *, Widget *pwidget = nullptr, bool autoDelete = false);

    public:
        void drawEx(int, int, int, int, int, int) const override;

    private:
        void drawFrame() const;

    private:
        void drawPlain() const;
        void drawWithNPCFace() const;

    public:
        void loadXML(uint64_t, const char *, const char *);

    private:
        void onClickEvent(const char *, const char *, const char *, bool);

    private:
        int getMiddlePixels() const;

    private:
        uint32_t getNPCFaceKey() const
        {
            return 0X50000000 | uidf::getNPCID(m_npcUID);
        }
};
