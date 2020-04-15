/*
 * =====================================================================================
 *
 *       Filename: npc.cpp
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
#include "npc.hpp"
#include "fflerror.hpp"

NPC::NPC(uint16_t lookId, ServiceCore *core, ServerMap *serverMap, int mapX, int mapY, int dirIndex)
    : CharObject(core, serverMap, uidf::buildNPCUID(lookId), mapX, mapY, DIR_NONE)
    , m_dirIndex(dirIndex)
{
    // TODO
    // support lua script, don't use hard coded logic

    m_onEventID[0] = [this](uint64_t uid, const AMNPCEvent &)
    {
        const char *xmlMessage = 
            u8R"###(<layout>                                                             )###"
            u8R"###(    <par>客官你好，有什么可以为你服务的吗？<emoji id="0"/></par>     )###"
            u8R"###(    <par><event id="1">如何快速升级</event></par>                    )###"
            u8R"###(    <par><event id="close">关闭</event></par>                        )###"
            u8R"###(</layout>                                                            )###";
        sendXMLLayout(uid, xmlMessage);
    };

    m_onEventID[1] = [this](uint64_t uid, const AMNPCEvent &)
    {
        const char *xmlMessage = 
            u8R"###(<layout>                                                             )###"
            u8R"###(    <par>多多上线打怪升级！<emoji id="1"/></par>                     )###"
            u8R"###(    <par><event id="close">关闭</event></par>                        )###"
            u8R"###(</layout>                                                            )###";
        sendXMLLayout(uid, xmlMessage);
    };
}

void NPC::sendXMLLayout(uint64_t uid, const char *xmlString)
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
    m_actorPod->Forward(uid, {MPK_NPCXMLLAYOUT, amNPCXMLL});
}
