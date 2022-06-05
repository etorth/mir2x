#include <cstdint>
#include "servermsg.hpp"
#include "processcreateaccount.hpp"

void ProcessCreateAccount::net_CREATEACCOUNTOK(const uint8_t *, size_t)
{
    setInfoStr(u8"注册成功", 2);
    m_boxID.focus(false);
    m_boxPwd.focus(false);
    m_boxPwdConfirm.focus(false);
}

void ProcessCreateAccount::net_CREATEACCOUNTERROR(const uint8_t *buf, size_t)
{
    const auto smCAE = ServerMsg::conv<SMCreateAccountError>(buf);
    switch(smCAE.error){
        case CRTACCERR_ACCOUNTEXIST:
            {
                setInfoStr(u8"账号已存在", 2);
                clearInput();

                m_boxID.focus(true);
                m_boxPwd.focus(false);
                m_boxPwdConfirm.focus(false);
                return;
            }
        case CRTACCERR_BADACCOUNT:
            {
                setInfoStr(u8"无效的账号", 2);
                clearInput();

                m_boxID.focus(true);
                m_boxPwd.focus(false);
                m_boxPwdConfirm.focus(false);
                return;
            }
        case CRTACCERR_BADPASSWORD:
            {
                setInfoStr(u8"无效的密码", 2);
                clearInput();

                m_boxID.focus(true);
                m_boxPwd.focus(false);
                m_boxPwdConfirm.focus(false);
                return;
            }
        default:
            {
                throw fflreach();
            }
    }
}
