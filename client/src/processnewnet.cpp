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
#include "processnew.hpp"

void ProcessNew::net_ACCOUNT(const uint8_t *buf, size_t)
{
    const auto smA = ServerMsg::conv<SMAccount>(buf);
    switch(smA.error){
        case CAERR_NONE:
            {
                setInfoStr(u8"注册成功", 2);
                m_boxID.focus(false);
                m_boxPwd.focus(false);
                m_boxPwdConfirm.focus(false);
                return;
            }
        case CAERR_EXIST:
            {
                setInfoStr(u8"账号已存在", 2);
                clearInput();

                m_boxID.focus(true);
                m_boxPwd.focus(false);
                m_boxPwdConfirm.focus(false);
                return;
            }
        case CAERR_INVALID:
            {
                setInfoStr(u8"无效的账号或密码", 2);
                clearInput();

                m_boxID.focus(true);
                m_boxPwd.focus(false);
                m_boxPwdConfirm.focus(false);
                return;
            }
        default:
            {
                throw fflerror("Invalid server message: %d", to_d(smA.error));
            }
    }
}
