#include "client.hpp"
#include "fflerror.hpp"
#include "processlogin.hpp"

extern Client *g_client;
void ProcessLogin::net_LOGINOK(const uint8_t *, size_t)
{
    g_client->requestProcess(PROCESSID_SELECTCHAR);
}

void ProcessLogin::net_LOGINERROR(const uint8_t *buf, size_t)
{
    const auto smLE = ServerMsg::conv<SMLoginError>(buf);
    switch(smLE.error){
        case LOGINERR_NOACCOUNT:
            {
                m_notifyBoard.addLog(u8"无效的账号或密码");
                return;
            }
        case LOGINERR_MULTILOGIN:
            {
                m_notifyBoard.addLog(u8"该账号已经登录");
                return;
            }
        default:
            {
                throw bad_reach();
            }
    }
}
