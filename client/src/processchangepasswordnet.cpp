#include <cstdint>
#include "servermsg.hpp"
#include "processchangepassword.hpp"

void ProcessChangePassword::net_CHANGEPASSWORDOK(const uint8_t *, size_t)
{
    setInfoStr(u8"修改密码成功", 2);
    m_boxID.setFocus(false);
    m_boxPwd.setFocus(false);
    m_boxNewPwd.setFocus(false);
    m_boxNewPwdConfirm.setFocus(false);
}

void ProcessChangePassword::net_CHANGEPASSWORDERROR(const uint8_t *buf, size_t)
{
    const auto smCAE = ServerMsg::conv<SMChangePasswordError>(buf);
    switch(smCAE.error){
        case CHGPWDERR_BADACCOUNT:
            {
                setInfoStr(u8"无效的账号", 2);
                clearInput();

                m_boxID.setFocus(true);
                m_boxPwd.setFocus(false);
                m_boxNewPwd.setFocus(false);
                m_boxNewPwdConfirm.setFocus(false);
                return;
            }
        case CHGPWDERR_BADPASSWORD:
            {
                setInfoStr(u8"无效的密码", 2);
                clearInput();

                m_boxID.setFocus(true);
                m_boxPwd.setFocus(false);
                m_boxNewPwd.setFocus(false);
                m_boxNewPwdConfirm.setFocus(false);
                return;
            }
        case CHGPWDERR_BADNEWPASSWORD:
            {
                setInfoStr(u8"无效的新密码", 2);
                clearInput();

                m_boxID.setFocus(true);
                m_boxPwd.setFocus(false);
                m_boxNewPwd.setFocus(false);
                m_boxNewPwdConfirm.setFocus(false);
                return;
            }
        case CHGPWDERR_BADACCOUNTPASSWORD:
            {
                setInfoStr(u8"错误的账号或密码", 2);
                clearInput();

                m_boxID.setFocus(true);
                m_boxPwd.setFocus(false);
                m_boxNewPwd.setFocus(false);
                m_boxNewPwdConfirm.setFocus(false);
                return;
            }
        default:
            {
                throw fflreach();
            }
    }
}
