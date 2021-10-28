#pragma once
#include <cstdint>
#include <optional>
#include "widget.hpp"
#include "process.hpp"
#include "raiitimer.hpp"
#include "labelboard.hpp"
#include "inputline.hpp"
#include "passwordbox.hpp"
#include "tritexbutton.hpp"

class ProcessChangePassword: public Process
{
    private:
        constexpr static int m_x = 180;
        constexpr static int m_y = 145;

    private:
        LabelBoard m_LBID;
        LabelBoard m_LBPwd;
        LabelBoard m_LBNewPwd;
        LabelBoard m_LBNewPwdConfirm;

    private:
        InputLine   m_boxID;
        PasswordBox m_boxPwd;
        PasswordBox m_boxNewPwd;
        PasswordBox m_boxNewPwdConfirm;

    private:
        LabelBoard m_LBCheckID;
        LabelBoard m_LBCheckPwd;
        LabelBoard m_LBCheckNewPwd;
        LabelBoard m_LBCheckNewPwdConfirm;

    private:
        TritexButton m_submit;
        TritexButton m_quit;

    private:
        LabelBoard m_infoStr;
        uint32_t   m_infoStrSec;
        hres_timer m_infoStrTimer;

    public:
        ProcessChangePassword();
        virtual ~ProcessChangePassword() = default;

    public:
        int id() const override
        {
            return PROCESSID_CHANGEPASSWORD;
        }

    public:
        void draw() const override;
        void update(double) override;
        void processEvent(const SDL_Event &) override;

    private:
        void doPostPasswordChange();
        void doExit();

    private:
        bool localCheckID (const char *) const;
        bool localCheckPwd(const char *) const;

    private:
        void localCheck();

    private:
        void clearInput();

    private:
        bool hasInfo() const;
        void setInfoStr(const char8_t *);
        void setInfoStr(const char8_t *, uint32_t);

    public:
        void net_CHANGEPASSWORDOK   (const uint8_t *, size_t);
        void net_CHANGEPASSWORDERROR(const uint8_t *, size_t);
};
