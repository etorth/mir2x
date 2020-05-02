/*
 * =====================================================================================
 *
 *       Filename: npchar.cpp
 *        Created: 04/12/2020 16:01:51
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
#include "npchar.hpp"
#include "fflerror.hpp"

NPChar::NPChar(uint16_t lookId, ServiceCore *core, ServerMap *serverMap, int mapX, int mapY, int dirIndex)
    : CharObject(core, serverMap, uidf::buildNPCUID(lookId), mapX, mapY, DIR_NONE)
    , m_dirIndex(dirIndex)
{
    // TODO
    // support lua script, don't use hard coded logic

    m_onEventID[0] = [this](uint64_t uid, const AMNPCEvent &)
    {
        const char *xmlMessage = 
            u8R"###(<layout>                                                                )###"
            u8R"###(    <par>客官你好，我是%llu，欢迎来到传奇旧时光！<emoji id="0"/></par>  )###"
            u8R"###(    <par>有什么可以为你效劳的吗？</par>                                 )###"
            u8R"###(    <par></par>                                                         )###"
            u8R"###(    <par><event id="1">如何快速升级</event></par>                       )###"
            u8R"###(    <par><event id="close">关闭</event></par>                           )###"
            u8R"###(</layout>                                                               )###";
        sendXMLLayout(uid, str_printf(xmlMessage, toLLU(UID())).c_str());
    };

    m_onEventID[1] = [this](uint64_t uid, const AMNPCEvent &)
    {
        const char *xmlMessage = 
            u8R"###(<layout>                                           )###"
            u8R"###(    <par>多多上线打怪升级！<emoji id="1"/></par>   )###"
            u8R"###(    <par><event id="close">关闭</event></par>      )###"
            u8R"###(</layout>                                          )###";
        sendXMLLayout(uid, xmlMessage);
    };
}

bool NPChar::Update()
{
    return true;
}

bool NPChar::InRange(int, int, int)
{
    return true;
}

void NPChar::ReportCORecord(uint64_t)
{
}

bool NPChar::DCValid(int, bool)
{
    return true;
}

DamageNode NPChar::GetAttackDamage(int)
{
    return {};
}

bool NPChar::StruckDamage(const DamageNode &)
{
    return true;
}

void NPChar::checkFriend(uint64_t, std::function<void(int)>)
{
}

bool NPChar::GoDie()
{
    return true;
}

bool NPChar::GoGhost()
{
    return true;
}

void NPChar::sendXMLLayout(uint64_t uid, const char *xmlString)
{
    if(!xmlString){
        throw fflerror("xmlString null");
    }

    AMNPCXMLLayout amNPCXMLL;
    std::memset(&amNPCXMLL, 0, sizeof(amNPCXMLL));

    if(std::strlen(xmlString) > sizeof(amNPCXMLL.xmlLayout)){
        throw fflerror("xmlString is too long");
    }

    std::strcpy(amNPCXMLL.xmlLayout, xmlString);
    m_actorPod->forward(uid, {MPK_NPCXMLLAYOUT, amNPCXMLL});
}

void NPChar::OperateAM(const MessagePack &mpk)
{
    switch(mpk.Type()){
        case MPK_OFFLINE:
        case MPK_METRONOME:
            {
                break;
            }
        case MPK_ACTION:
            {
                On_MPK_ACTION(mpk);
                break;
            }
        case MPK_NPCEVENT:
            {
                On_MPK_NPCEVENT(mpk);
                break;
            }
        case MPK_NOTIFYNEWCO:
            {
                On_MPK_NOTIFYNEWCO(mpk);
                break;
            }
        case MPK_QUERYCORECORD:
            {
                On_MPK_QUERYCORECORD(mpk);
                break;
            }
        case MPK_QUERYLOCATION:
            {
                On_MPK_QUERYLOCATION(mpk);
                break;
            }
        default:
            {
                throw fflerror("unsupported message: %s", mpk.Name());
            }
    }
}
