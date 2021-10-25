#pragma once
#include <cstdint>
#include <optional>
#include "process.hpp"
#include "raiitimer.hpp"
#include "labelboard.hpp"
#include "inputline.hpp"
#include "notifyboard.hpp"
#include "passwordbox.hpp"
#include "tritexbutton.hpp"

class ProcessCreateChar: public Process
{
    private:
        int m_job = JOB_WARRIOR;
        bool m_gender = true;

    private:
        TritexButton m_warrior;
        TritexButton m_wizard;
        TritexButton m_taoist;

    private:
        TritexButton m_submit;
        TritexButton m_exit;

    private:
        InputLine m_nameLine;

    private:
        NotifyBoard m_notifyBoard;

    public:
        ProcessCreateChar();

    public:
        int id() const override
        {
            return PROCESSID_CREATECHAR;
        }

    public:
        void update(double) override;
        void draw() override;
        void processEvent(const SDL_Event &) override;

    private:
        void onSubmit();
        void onExit();

    private:
        void setGUIActive(bool);

    private:
        void net_CREATECHAROK   (const uint8_t *, size_t);
        void net_CREATECHARERROR(const uint8_t *, size_t);
};
