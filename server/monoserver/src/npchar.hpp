/*
 * =====================================================================================
 *
 *       Filename: npchar.hpp
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

class NPChar final: public CharObject
{
    private:
        int m_dirIndex;
        std::unordered_map<int, std::function<void(uint64_t, const AMNPCEvent &)>> m_onEventID;

    public:
        NPChar(uint16_t, ServiceCore *, ServerMap *, int, int, int);

    public:
        uint32_t mapID() const
        {
            return 0;
        }

    public:
        bool Update() override;
        bool InRange(int, int, int) override;

    public:
        void ReportCORecord(uint64_t) override;

    public:
        bool DCValid(int, bool) override;
        DamageNode GetAttackDamage(int) override;

    public:
        bool StruckDamage(const DamageNode &) override;

    public:
        bool GoDie() override;
        bool GoGhost() override;

    public:
        void checkFriend(uint64_t, std::function<void(int)>) override;

    private:
        void On_MPK_ACTION(const MessagePack &);
        void On_MPK_NPCEVENT(const MessagePack &);
        void On_MPK_NOTIFYNEWCO(const MessagePack &);
        void On_MPK_QUERYCORECORD(const MessagePack &);
        void On_MPK_QUERYLOCATION(const MessagePack &);

    private:
        void sendXMLLayout(uint64_t, const char *);

    public:
        void OperateAM(const MessagePack &) override;
};
