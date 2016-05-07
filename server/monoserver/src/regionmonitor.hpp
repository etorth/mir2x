/*
 * =====================================================================================
 *
 *       Filename: regionmonitor.hpp
 *        Created: 04/21/2016 12:09:03
 *  Last Modified: 05/07/2016 01:10:06
 *
 *    Description: at the beginning I was thinking to init region monitro first, to
 *                 set all region/neighbor, and then call Activate(), then I found
 *                 that ``when you have the address, you already activated it", so
 *                 I need to use message to pass the address to it. Since so, I just
 *                 put all initialization work into message.
 *
 *                 RegionMonitor is an transponder, which has no UID()/AddTime(), but
 *                 it has an actor pod to response to message
 *
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
#include <list>
#include <array>
#include <cassert>
#include <cstdint>
#include <Theron/Theron.h>

#include "transponder.hpp"
#include "activeobject.hpp"

class RegionMonitor: public Transponder
{
    private:
        // used when query neighbor's opinion for cover check
        enum QueryType: int {
            QUERY_NA,
            QUERY_OK,
            QUERY_ERROR,
            QUERY_PENDING,
        };

    private:
        // struct to describe the move
        //      1. new object to get inside
        //      2. existing object to get inside
        //      3. existing object to move outside
        //      4. successful cover check
        typedef struct _MoveRequest{
            void       *Data;           // used when adding new object

            uint32_t    UID;
            uint32_t    AddTime;
            uint32_t    MPKID;          // MessagePack::ID() for response

            int     CurrX;
            int     CurrY;
            int     X;
            int     Y;
            int     R;

            bool    In;
            bool    CurrIn;

            bool    CoverCheck;

            Theron::Address PodAddress; // address for response

            _MoveRequest()
                : Data(nullptr)
                , UID(0)
                , AddTime(0)
                , MPKID(0)
                , CurrX(-1)
                , CurrY(-1)
                , X(-1)
                , Y(-1)
                , R(-1)
                , In(false)
                , CurrIn(false)
                , CoverCheck(false)
                , PodAddress(Theron::Address::Null())
            {}

            // means this record contains a valid record
            // this has many possibilities
            bool Valid()
            {
                // new object adding inside
                if(Data){ return true; }

                // not an new object, then
                if(UID && AddTime){ return true; }

                // if now there is a cover check in current region
                if(CoverCheck){ return true; }

                return false;
            }

            void Clear()
            {
                Data = nullptr;
                UID = 0;
                AddTime = 0;
            }
        }MoveRequest;

        typedef struct _CharObjectRecord{
            uint32_t UID;
            uint32_t AddTime;

            int X;
            int Y;
            int R;

            Theron::Address PodAddress;

            _CharObjectRecord()
                : UID(0)
                , AddTime(0)
                , X(0)
                , Y(0)
                , R(0)
                , PodAddress(Theron::Address::Null())
            {}
        }CharObjectRecord;

    protected:
        MoveRequest m_MoveRequest;

    private:
        std::vector<CharObjectRecord> m_CharObjectRecordV;

    private:
        Theron::Address m_MapAddress;

    private:
        // +---+---+---+      arrange it in the form:
        // | 0 | 1 | 2 |      for(nDY = -1; nDY <= 1; ++nDY){
        // +---+---+---+          for(nDX = -1; nDX <= 1; ++nDX){
        // | 3 | x | 4 |              ....
        // +---+---+---+          }
        // | 5 | 6 | 7 |      }
        // +---+---+---+
        //
        typedef struct _NeighborRecord{
            int Query;
            uint32_t MPKID;
            Theron::Address PodAddress;

            _NeighborRecord()
                : Query(QUERY_NA)
                , MPKID(0)
                , PodAddress(Theron::Address::Null())
            {}

            bool Valid()
            {
                return PodAddress != Theron::Address::Null();
            }
        }NeighborRecord;
        std::array<std::array<NeighborRecord, 3>, 3> m_NeighborV2D;

    private:
        // region it takes in charge
        int     m_X;
        int     m_Y;
        int     m_W;
        int     m_H;

        int     m_LocX;
        int     m_LocY;
        bool    m_RegionDone;
        bool    m_NeighborDone;

        uint32_t m_MapID;

    public:
        RegionMonitor(const Theron::Address &rstMapAddr)
            : Transponder()
            , m_MapAddress(rstMapAddr)
            , m_X(0)
            , m_Y(0)
            , m_W(0)
            , m_H(0)
            , m_RegionDone(false)
            , m_NeighborDone(false)
            , m_MapID(0)
        {
            // in transponder we alreay put ``DelayQueue" trigger inside
            //
            // Install("Update", [this](){ For_Update(); });
            Install("MoveRequest", [this](){ For_MoveRequest(); });
        }

        virtual ~RegionMonitor() = default;

    protected:
        void DoMoveRequest();

    public:
        Theron::Address Activate();
        void Operate(const MessagePack &, const Theron::Address &);

    private:
        void On_MPK_TRYMOVE(const MessagePack &, const Theron::Address &);
        void On_MPK_NEIGHBOR(const MessagePack &, const Theron::Address &);
        void On_MPK_METRONOME(const MessagePack &, const Theron::Address &);
        void On_MPK_NEWMONSTER(const MessagePack &, const Theron::Address &);
        void On_MPK_CHECKCOVER(const MessagePack &, const Theron::Address &);
        void On_MPK_INITREGIONMONITOR(const MessagePack &, const Theron::Address &);

    private:
        bool GroundValid(int, int, int);
        bool CoverValid(uint32_t, uint32_t, int, int, int);

        Theron::Address NeighborAddress(int, int);

    private:
        void For_Update();
        void For_MoveRequest();
};
