/*
 * =====================================================================================
 *
 *       Filename: npcchatboard.hpp
 *        Created: 04/12/2020 18:56:22
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
        uint64_t m_NPCUID;

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
        void loadXML(uint64_t, const char *);

    private:
        void onClickEvent(const char *, const char *);

    private:
        int getMiddleCount() const;

    private:
        uint32_t getNPCFaceKey() const
        {
            return 0X50000000 | uidf::getNPCID(m_NPCUID);
        }
};
