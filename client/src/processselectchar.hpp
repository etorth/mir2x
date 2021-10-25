#pragma once
#include <cstdint>
#include <optional>
#include "process.hpp"
#include "raiitimer.hpp"
#include "servermsg.hpp"
#include "labelboard.hpp"
#include "notifyboard.hpp"
#include "passwordbox.hpp"
#include "tritexbutton.hpp"
#include "inputstringboard.hpp"

class ProcessSelectChar: public Process
{
    private:
        TritexButton m_start;
        TritexButton m_create;
        TritexButton m_delete;
        TritexButton m_exit;

    private:
        NotifyBoard m_notifyBoard;
        InputStringBoard m_deleteInput;

    private:
        std::optional<SMQueryCharOK> m_smChar;

    private:
        int m_charAni = 0;
        double m_charAniTime = 0.0;
        uint32_t m_charAniSwitchFrame = 0;

    public:
        ProcessSelectChar();

    public:
        int id() const override
        {
            return PROCESSID_SELECTCHAR;
        }

    public:
        void draw() const override;
        void update(double) override;
        void processEvent(const SDL_Event &) override;

    private:
        void drawChar() const;
        void drawCharName() const;

    private:
        void switchCharGfx();

    private:
        uint32_t charFrameCount() const;
        std::optional<uint32_t> charGfxBaseID() const;

    private:
        uint32_t absFrame() const
        {
            return to_u32(std::lround(m_charAniTime / 200.0));
        }

    private:
        bool hasChar() const
        {
            return m_smChar.has_value() && !m_smChar.value().name.empty();
        }

    private:
        void onStart();
        void onCreate();
        void onDelete();
        void onExit();

    public:
        void net_QUERYCHAROK    (const uint8_t *, size_t);
        void net_QUERYCHARERROR (const uint8_t *, size_t);
        void net_DELETECHAROK   (const uint8_t *, size_t);
        void net_DELETECHARERROR(const uint8_t *, size_t);
        void net_ONLINEOK       (const uint8_t *, size_t);
        void net_ONLINEERROR    (const uint8_t *, size_t);
};
