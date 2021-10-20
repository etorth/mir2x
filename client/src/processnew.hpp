/*
 * =====================================================================================
 *
 *       Filename: processnew.hpp
 *        Created: 08/14/2015 02:47:30 PM
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
#include <optional>
#include "process.hpp"
#include "raiitimer.hpp"
#include "labelboard.hpp"
#include "inputline.hpp"
#include "passwordbox.hpp"
#include "tritexbutton.hpp"

class ProcessNew: public Process
{
    private:
        constexpr static int m_x = 180;
        constexpr static int m_y = 145;

    private:
        LabelBoard m_LBID;
        LabelBoard m_LBPwd;
        LabelBoard m_LBPwdConfirm;

    private:
        InputLine   m_boxID;
        PasswordBox m_boxPwd;
        PasswordBox m_boxPwdConfirm;

    private:
        LabelBoard m_LBCheckID;
        LabelBoard m_LBCheckPwd;
        LabelBoard m_LBCheckPwdConfirm;

    private:
        TritexButton m_submit;
        TritexButton m_quit;

    private:
        LabelBoard m_infoStr;
        uint32_t   m_infoStrSec;
        hres_timer m_infoStrTimer;

    public:
        ProcessNew();
        virtual ~ProcessNew() = default;

    public:
        int ID() const override
        {
            return PROCESSID_NEW;
        }

    public:
        void update(double) override;
        void draw() override;
        void processEvent(const SDL_Event &) override;

    private:
        void doPostAccount();
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
        void net_ACCOUNT(const uint8_t *, size_t);
};
