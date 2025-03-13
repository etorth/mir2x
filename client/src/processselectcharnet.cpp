#include "uidf.hpp"
#include "dbcomid.hpp"
#include "client.hpp"
#include "fflerror.hpp"
#include "clientargparser.hpp"
#include "processselectchar.hpp"

extern Client *g_client;
extern ClientArgParser *g_clientArgParser;

void ProcessSelectChar::on_SM_QUERYCHAROK(const uint8_t *buf, size_t)
{
    m_smChar = ServerMsg::conv<SMQueryCharOK>(buf);
    fflassert(m_smChar.value().name.size > 0);
    m_notifyBoard.clear();

    if(m_smChar.value().name.empty()){
        m_notifyBoard.addLog(u8"请先创建游戏角色");
    }
    else if(g_clientArgParser->autoLogin){
        onStart();
    }

    updateGUIActive();
}

void ProcessSelectChar::on_SM_QUERYCHARERROR(const uint8_t *buf, size_t)
{
    const auto smQCE = ServerMsg::conv<SMQueryCharError>(buf);
    switch(smQCE.error){
        case QUERYCHARERR_NOCHAR:
            {
                m_smChar.emplace();
                m_smChar.value().name.clear();

                updateGUIActive();
                m_notifyBoard.addLog(u8"请先创建游戏角色");
                break;
            }
        case QUERYCHARERR_NOLOGIN:
        default:
            {
                throw fflreach();
            }
    }
}

void ProcessSelectChar::on_SM_DELETECHAROK(const uint8_t *, size_t)
{
    m_smChar.emplace();
    m_smChar.value().name.clear();

    updateGUIActive();
    m_notifyBoard.addLog(u8"删除角色成功");
}

void ProcessSelectChar::on_SM_DELETECHARERROR(const uint8_t *buf, size_t)
{
    const auto smDCE = ServerMsg::conv<SMDeleteCharError>(buf);
    switch(smDCE.error){
        case DELCHARERR_BADPASSWORD:
            {
                m_notifyBoard.addLog(u8"密码错误");
                return;
            }
        case DELCHARERR_NOCHAR:
            {
                m_notifyBoard.addLog(u8"没有角色可以删除");
                return;
            }
        default:
            {
                throw fflreach();
            }
    }
}

void ProcessSelectChar::on_SM_ONLINEOK(const uint8_t *buf, size_t)
{
    const auto smOOK = ServerMsg::conv<SMOnlineOK>(buf);
    fflassert(uidf::isPlayer(smOOK.uid), smOOK.uid, uidf::getUIDString(smOOK.uid));
    fflassert(DBCOM_MAPRECORD(smOOK.mapID), smOOK.mapID);

    g_client->setOnlineOK(smOOK);
    g_client->requestProcess(PROCESSID_RUN);
}

void ProcessSelectChar::on_SM_ONLINEERROR(const uint8_t *buf, size_t)
{
    const auto smOE = ServerMsg::conv<SMOnlineError>(buf);
    switch(smOE.error){
        case ONLINEERR_NOCHAR:
            {
                m_notifyBoard.addLog(u8"先创建角色以运行游戏");
                return;
            }
        case ONLINEERR_MULTIONLINE:
            {
                m_notifyBoard.addLog(u8"请勿频繁登录");
                return;
            }
        default:
            {
                throw fflvalue(smOE.error);
            }
    }
}
