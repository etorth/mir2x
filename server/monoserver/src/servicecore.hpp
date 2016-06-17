/*
 * =====================================================================================
 *
 *       Filename: servicecore.hpp
 *        Created: 04/22/2016 17:59:06
 *  Last Modified: 06/16/2016 23:13:17
 *
 *    Description: split monoserver into actor-code and non-actor code
 *                 put all actor code in this class
 *
 *                 TODO & TBD
 *                 everytime when creating a lambda in an actor to use ThreadPN to
 *                 invoke, never use [this, ...] since this will access the internal
 *                 state from another thread
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
#include <vector>
#include <unordered_map>

#include "netpod.hpp"
#include "transponder.hpp"

class ServerMap;
class ServiceCore: public Transponder
{
    private:
        // TODO & TBD
        // I decide to put a record of player in service core
        // otherwise if we want a player, we have no idea where to find it since
        // it's created locally in the corresponding region monitor
        typedef struct _PlayerRecord{
            uint32_t GUID;
            uint32_t UID;
            uint32_t AddTime;
            Theron::Address PodAddress;

            _PlayerRecord(uint32_t nGUID = 0,
                    uint32_t nUID = 0, uint32_t nAddTime = 0,
                    const Theron::Address &rstAddr = Theron::Address::Null())
                : GUID(nGUID)
                , UID(nUID)
                , AddTime(nAddTime)
                , PodAddress(rstAddr)
            {}
        }PlayerRecord;

        enum QueryType: int{
            QUERY_NA = 0,
            QUERY_OK,
            QUERY_ERROR,
            QUERY_PENDING,
        };

        // TODO & TBD
        // there is a RM cache for RM's so we don't have to pass
        // the address everytime
        typedef struct _RMRecord{
            uint32_t MapID;
            int      RMX;
            int      RMY;
            int      Query;
            Theron::Address PodAddress;

            _RMRecord(uint32_t nMapID = 0,
                    int nRMX = 0, int nRMY = 0, int nQuery = QUERY_NA,
                    const Theron::Address &rstAddr = Theron::Address::Null())
                : MapID(nMapID)
                , RMX(nRMX)
                , RMY(nRMY)
                , Query(nQuery)
                , PodAddress(rstAddr)
            {}

            // check whether it's an empty record
            bool Empty()
            {
                return MapID == 0; 
            }

            bool Empty() const
            {
                return MapID == 0; 
            }

            bool Ready()
            {
                if(!MapID){ return false; }
                return (Query == QUERY_NA) ? true : Valid();
            }

            bool Ready() const
            {
                if(!MapID){ return false; }
                return (Query == QUERY_NA) ? true : Valid();
            }

            bool Valid()
            {
                // didn't check validation of RMX, RMY but this is enough
                return MapID && Query == QUERY_OK && PodAddress != Theron::Address::Null();
            }

            bool Valid() const
            {
                // didn't check validation of RMX, RMY but this is enough
                return MapID && Query == QUERY_OK && PodAddress != Theron::Address::Null();
            }
        }RMRecord;

        typedef struct _MapRecord{
            uint32_t        MapID;
            ServerMap      *Map;

            uint32_t        GridW;
            uint32_t        GridH;
            size_t          RMW;
            size_t          RMH;

            Theron::Address PodAddress;
            std::unordered_map<uint32_t, RMRecord> RMRecordMap;

            _MapRecord(uint32_t nMapID = 0, ServerMap *pMap = nullptr,
                    uint32_t nGridW = 0, uint32_t nGridH = 0, size_t nRMW = 0, size_t nRMH = 0,
                    const Theron::Address &rstAddr = Theron::Address::Null())
                : MapID(nMapID)
                , Map(pMap)
                , GridW(nGridW)
                , GridH(nGridH)
                , RMW(nRMW)
                , RMH(nRMH)
                , PodAddress(rstAddr)
                , RMRecordMap()
            {}
        }MapRecord;

    protected:
        const RMRecord m_EmptyRMRecord;
        std::unordered_map<uint32_t, MapRecord>    m_MapRecordMap;
        std::unordered_map<uint32_t, PlayerRecord> m_PlayerRecordMap;

    public:
        ServiceCore();
        virtual ~ServiceCore();

    protected:

    public:
        void Operate(const MessagePack &, const Theron::Address &);
        void OperateNet(uint32_t, uint8_t, const uint8_t *, size_t);

    protected:
        bool LoadMap(uint32_t);
        bool ValidP(uint32_t, int, int);

    private:
        // dangerious functions, can add self-driven tigger
        const RMRecord &GetRMRecord(uint32_t, int, int, bool);
        int QueryRMAddress(uint32_t, int, int, bool);

    private:
        void On_MPK_LOGIN(const MessagePack &, const Theron::Address &);
        void On_MPK_DUMMY(const MessagePack &, const Theron::Address &);
        void On_MPK_NETPACKAGE(const MessagePack &, const Theron::Address &);
        void On_MPK_LOGINQUERYDB(const MessagePack &, const Theron::Address &);
        void On_MPK_PLAYERPHATOM(const MessagePack &, const Theron::Address &);
        void On_MPK_NEWCONNECTION(const MessagePack &, const Theron::Address &);
        void On_MPK_ADDCHAROBJECT(const MessagePack &, const Theron::Address &);
        void On_MPK_QUERYMONSTERGINFO(const MessagePack &, const Theron::Address &);

    private:
        void Net_CM_Login(uint32_t, uint8_t, const uint8_t *, size_t);

#if defined(MIR2X_DEBUG) && (MIR2X_DEBUG >= 5)
    protected:
        const char *ClassName()
        {
            return "ServiceCore";
        }
#endif
};
