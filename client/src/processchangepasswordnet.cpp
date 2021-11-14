/*
 * =====================================================================================
 *
 *       Filename: processnewnet.cpp
 *        Created: 08/14/2015 02:47:49
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

#include <cstdint>
#include "servermsg.hpp"
#include "processchangepassword.hpp"

void ProcessChangePassword::net_CHANGEPASSWORDOK(const uint8_t *, size_t)
{
    setInfoStr(u8"修改密码成功", 2);
    m_boxID.focus(false);
    m_boxPwd.focus(false);
    m_boxNewPwd.focus(false);
    m_boxNewPwdConfirm.focus(false);
}

void ProcessChangePassword::net_CHANGEPASSWORDERROR(const uint8_t *buf, size_t)
{
    const auto smCAE = ServerMsg::conv<SMChangePasswordError>(buf);
    switch(smCAE.error){
        case CHGPWDERR_BADACCOUNT:
            {
                setInfoStr(u8"无效的账号", 2);
                clearInput();

                m_boxID.focus(true);
                m_boxPwd.focus(false);
                m_boxNewPwd.focus(false);
                m_boxNewPwdConfirm.focus(false);
                return;
            }
        case CHGPWDERR_BADPASSWORD:
            {
                setInfoStr(u8"无效的密码", 2);
                clearInput();

                m_boxID.focus(true);
                m_boxPwd.focus(false);
                m_boxNewPwd.focus(false);
                m_boxNewPwdConfirm.focus(false);
                return;
            }
        case CHGPWDERR_BADNEWPASSWORD:
            {
                setInfoStr(u8"无效的新密码", 2);
                clearInput();

                m_boxID.focus(true);
                m_boxPwd.focus(false);
                m_boxNewPwd.focus(false);
                m_boxNewPwdConfirm.focus(false);
                return;
            }
        case CHGPWDERR_BADACCOUNTPASSWORD:
            {
                setInfoStr(u8"错误的账号或密码", 2);
                clearInput();

                m_boxID.focus(true);
                m_boxPwd.focus(false);
                m_boxNewPwd.focus(false);
                m_boxNewPwdConfirm.focus(false);
                return;
            }
        default:
            {
                throw fflreach();
            }
    }
}
