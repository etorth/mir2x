/*
 * =====================================================================================
 *
 *       Filename: regionmonitor.hpp
 *        Created: 04/21/2016 12:09:03
 *  Last Modified: 05/31/2016 18:16:17
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

#include "mathfunc.hpp"
#include "transponder.hpp"
#include "activeobject.hpp"

class RegionMonitor: public Transponder
{
    private:
        // used when query neighbor's opinion for cover check
        enum QueryType: int{
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
            uint8_t Type;               // object type

            // TODO I did't make it as a union
            //      I can but since I'm not concerned about the memory
            //      just let it as it is
            struct _Monster{
                uint32_t MonsterID;

                _Monster(uint32_t nMonsterID = 0)
                    : MonsterID(nMonsterID)
                {}
            }Monster;

            struct _Player{
                uint32_t GUID;
                uint32_t JobID;

                _Player(uint32_t nGUID = 0, uint32_t nJobID = 0)
                    : GUID(nGUID)
                    , JobID(nJobID)
                {}
            }Player;

            uint32_t    UID;            // zero for new object
            uint32_t    AddTime;        // zero for new object
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
                : Type(OBJECT_UNKNOWN)
                , Monster()
                , Player()
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
                Type              = OBJECT_UNKNOWN;
                Monster.MonsterID = 0;
                Player.GUID       = 0;
                Player.JobID      = 0;
                UID               = 0;
                AddTime           = 0;
                MPKID             = 0;
                CurrX             = 0;
                CurrY             = 0;
                X                 = 0;
                Y                 = 0;
                R                 = 0;
                OnlyIn            = false;
                CurrIn            = false;
                FreezeRM          = false;
                CoverCheck        = false;
                NeighborCheck     = false;
                PodAddress        = Theron::Address::Null();
            }
        }MoveRequest;

        typedef struct _CharObjectRecord{
            uint8_t Type;
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
        }CORecord;

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

    private:
        const uint32_t m_MapID;

        const int m_X;
        const int m_Y;
        const int m_W;
        const int m_H;

        const Theron::Address m_SCAddress;
        const Theron::Address m_MapAddress;

        MoveRequest m_MoveRequest;
        std::vector<CORecord> m_CORecordV;
        std::array<std::array<NeighborRecord, 3>, 3> m_NeighborV2D;

    public:
        RegionMonitor(const Theron::Address &rstMapAddr,
                uint32_t nMapID, int nX, int nY, int nW, int nH)
            : Transponder()
            , m_MapID(nMapID)
            , m_X(nX)
            , m_Y(nY)
            , m_W(nW)
            , m_H(nH)
            , m_MapAddress(rstMapAddr)
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

    protected:
        bool In(uint32_t nMapID, int nMapX, int nMapY)
        {
            if(nMapID != m_MapID){ return false; }
            if(PointInRectangle(nMapX, nMapY, m_X, m_Y, m_W, m_H)){ return true; }
            return false;
        }

    public:
        void Operate(const MessagePack &, const Theron::Address &);

    private:
        void On_MPK_LEAVE(const MessagePack &, const Theron::Address &);
        void On_MPK_TRYMOVE(const MessagePack &, const Theron::Address &);
        void On_MPK_NEIGHBOR(const MessagePack &, const Theron::Address &);
        void On_MPK_METRONOME(const MessagePack &, const Theron::Address &);
        void On_MPK_NEWPLAYER(const MessagePack &, const Theron::Address &);
        void On_MPK_NEWMONSTER(const MessagePack &, const Theron::Address &);
        void On_MPK_CHECKCOVER(const MessagePack &, const Theron::Address &);
        void On_MPK_MOTIONSTATE(const MessagePack &, const Theron::Address &);
        void On_MPK_TRYSPACEMOVE(const MessagePack &, const Theron::Address &);
        void On_MPK_ADDCHAROBJECT(const MessagePack &, const Theron::Address &);
        // void On_MPK_INITREGIONMONITOR(const MessagePack &, const Theron::Address &);

    private:
        bool GroundValid(int, int, int);
        bool CoverValid(uint32_t, uint32_t, int, int, int);

        Theron::Address NeighborAddress(int, int);

    private:
        void For_Update();
        void For_MoveRequest();
};
