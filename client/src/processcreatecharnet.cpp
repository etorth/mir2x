#include "client.hpp"
#include "servermsg.hpp"
#include "processcreatechar.hpp"

extern Client *g_client;
void ProcessCreateChar::on_SM_CREATECHAROK(const uint8_t *, size_t)
{
    g_client->requestProcess(PROCESSID_SELECTCHAR);
}

void ProcessCreateChar::on_SM_CREATECHARERROR(const uint8_t *buf, size_t)
{
    const auto smCCE = ServerMsg::conv<SMCreateCharError>(buf);
    switch(smCCE.error){
        case CRTCHARERR_BADNAME:
            {
                setGUIActive(true);
                m_notifyBoard.addLog(u8"无效的角色名");
                break;
            }
        case CRTCHARERR_CHAREXIST:
            {
                setGUIActive(true);
                m_notifyBoard.addLog(u8"一个账号只能创建一个角色");
                break;
            }
        case CRTCHARERR_NAMEEXIST:
            {
                setGUIActive(true);
                m_notifyBoard.addLog(u8"角色名已被使用");
                break;
            }
        default:
            {
                throw fflreach();
            }
    }
}
