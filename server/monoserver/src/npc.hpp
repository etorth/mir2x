/*
 * =====================================================================================
 *
 *       Filename: npc.hpp
 *        Created: 04/12/2020 15:53:55
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
#include "servicecore.hpp"
#include "servermap.hpp"
#include "charobject.hpp"

class NPC final: public CharObject
{
    private:
        int m_dirIndex;
        std::unordered_map<int, std::function<void(uint64_t, const AMNPCEvent &)>> m_onEventID;

    public:
        NPC(uint16_t, ServiceCore *, ServerMap *, int, int, int);

    public:
        uint32_t mapID() const
        {
            return 0;
        }

    private:
        void On_MPK_NPCEVENT(const MessagePack &);

    private:
        void sendXMLLayout(uint64_t, const char *);
};
