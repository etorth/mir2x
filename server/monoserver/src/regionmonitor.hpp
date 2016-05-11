/*
 * =====================================================================================
 *
 *       Filename: regionmonitor.hpp
 *        Created: 04/21/2016 12:09:03
 *  Last Modified: 05/10/2016 22:21:22
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
 *                 TODO & TBD
 *                 How to handle object move request???
 *                 1. object ask RM for move
 *                 2. RM checks after the move, the object will
 *                          A. still stay inside the RM, and won't overlap with neighbor RM
 *                          B. still stay inside the RM, but will cover neighbor RM
 *                          C. out of current RM, maybe different RM on current map or out
 *                             of current map
 *
 *                    for case-A, respond directly with MPK_OK or MPK_ERROR, when responding
 *                    OK, object will respond again to commit the move or cancel the move,
 *                    between (RM->Obj: OK) and (Obj->RM: commit / cancel move), the RM will
 *                    be freezed. if (RM->Obj: ERROR), the object won't respond and no
 *                    freezing is needed.
 *
 *
 *                    for case-B, first do cover check on current RM, if failed then do
 *                    (RM->Obj: ERROR) directly. if succeed, find overlapped RM's out of the
 *                    8 neighbors, if all these selected RM's are valid (not null address),
 *                    send cover check to them and freeze current RM. otherwise directly do
 *                    (RM->Obj: ERROR)
 *
 *                    for neighbors who got cover check, if check succeeds then freeze the
 *                    RM and respond MPK_OK, if fails only send MPK_ERROR.
 *
 *                    in the move request trigger, if there is no pending, means get all
 *                    responses already from neighbors, then if there is ERROR, send
 *                    MPK_ERROR to all RM with QUERY_OK to free them, and send MPK_ERROR
 *                    to the object requestor who asks for move
 *
 *                    TODO
 *                    for case-C, RM will try to find proper RM address to move in, and send
 *                    this address to the object, and the object will then do space move
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

            bool    OnlyIn;
            bool    CurrIn;
            bool    FreezeRM;
            bool    CoverCheck;
            bool    NeighborCheck;

            Theron::Address PodAddress; // address for response

            _MoveRequest()
                : Data(nullptr)
                , UID(0)
                , AddTime(0)
                , MPKID(0)
                , CurrX(0)
                , CurrY(0)
                , X(0)
                , Y(0)
                , R(0)
                , OnlyIn(false)
                , CurrIn(false)
                , FreezeRM(false)
                , CoverCheck(false)
                , NeighborCheck(false)
                , PodAddress(Theron::Address::Null())
            {}

            // means this record contains a valid record
            // this has many possibilities
            bool Freezed()
            {
                return FreezeRM;
            }

            void Freeze()
            {
                FreezeRM = true;
            }

            void Clear()
            {
                Data          = nullptr;
                UID           = 0;
                AddTime       = 0;
                MPKID         = 0;
                CurrX         = 0;
                CurrY         = 0;
                X             = 0;
                Y             = 0;
                R             = 0;
                OnlyIn        = false;
                CurrIn        = false;
                FreezeRM      = false;
                CoverCheck    = false;
                NeighborCheck = false;
                PodAddress    = Theron::Address::Null();
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
            m_MoveRequest.Clear();
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
        void On_MPK_LEAVE(const MessagePack &, const Theron::Address &);
        void On_MPK_TRYMOVE(const MessagePack &, const Theron::Address &);
        void On_MPK_NEIGHBOR(const MessagePack &, const Theron::Address &);
        void On_MPK_METRONOME(const MessagePack &, const Theron::Address &);
        void On_MPK_NEWMONSTER(const MessagePack &, const Theron::Address &);
        void On_MPK_CHECKCOVER(const MessagePack &, const Theron::Address &);
        void On_MPK_TRYSPACEMOVE(const MessagePack &, const Theron::Address &);
        void On_MPK_INITREGIONMONITOR(const MessagePack &, const Theron::Address &);

    private:
        bool GroundValid(int, int, int);
        bool CoverValid(uint32_t, uint32_t, int, int, int);

        Theron::Address NeighborAddress(int, int);

    private:
        void For_Update();
        void For_MoveRequest();
};
