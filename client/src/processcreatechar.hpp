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
        bool m_activeGender = true;

    private:
        TritexButton m_warrior;
        TritexButton m_wizard;
        TritexButton m_taoist;

    private:
        TritexButton m_submit;
        TritexButton m_exit;

    private:
        InputLine m_nameBox;

    private:
        NotifyBoard m_notifyBoard;

    private:
        double m_aniTime = 0.0;
        uint32_t m_lastStartAbsFrame = 0;

    public:
        ProcessCreateChar();

    public:
        ~ProcessCreateChar() override;

    public:
        int id() const override
        {
            return PROCESSID_CREATECHAR;
        }

    private:
        uint32_t absFrame() const
        {
            return to_u32(std::lround(m_aniTime / 200.0));
        }

    private:
        static uint32_t charGfxBaseID(int, bool);
        static uint32_t charFrameCount(int, bool);

    public:
        void draw() const override;
        void update(double) override;
        void processEvent(const SDL_Event &) override;

    private:
        void drawChar(bool, int, int) const;

    private:
        void onSubmit();
        void onExit();

    private:
        void setGUIActive(bool);

    public:
        void on_SM_CREATECHAROK   (const uint8_t *, size_t);
        void on_SM_CREATECHARERROR(const uint8_t *, size_t);

    private:
        void playMagicSoundEffect();
};
