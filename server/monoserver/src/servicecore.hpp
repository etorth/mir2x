/*
 * =====================================================================================
 *
 *       Filename: servicecore.hpp
 *        Created: 04/22/2016 17:59:06
 *  Last Modified: 04/30/2016 01:01:55
 *
 *    Description: split monoserver into actor-code and non-actor code
 *                 put all actor code in this class
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
#include "transponder.hpp"

class ServiceCore: public Transponder
{
    private:
        uint32_t m_CurrUID;

    private:
        typedef struct _PlayerRecord {
            uint32_t GUID;
            _PlayerRecord()
                : GUID(0)
            {}
        }PlayerRecord;

        typedef struct _MapRecord {
            uint32_t        MapID;
            ServerMap      *Map;
            Theron::Address PodAddress;
            _MapRecord(uint32_t nMapID = 0)
                : MapID(nMapID)
                , Map(nullptr)
                , PodAddress(Theron::Address::Null())
            {}
        }MapRecord;

        std::unordered_map<uint32_t, MapRecord>    m_MapRecordM;
        std::unordered_map<uint32_t, PlayerRecord> m_PlayerRecordM;


    public:
        ServiceCore();
        virtual ~ServiceCore();

    public:
        void Operate(const MessagePack &, const Theron::Address &);

    protected:
        bool LoadMap(uint32_t);
};
