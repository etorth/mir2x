/*
 * =====================================================================================
 *
 *       Filename: regionmonitor.hpp
 *        Created: 04/21/2016 12:09:03
 *  Last Modified: 06/18/2016 18:05:05
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
#include "monoserver.hpp"
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
        typedef struct _MoveRequest{
            // current region monitor is locked because:

            bool    WaitCO;             // of waiting for co's response to ths the move permission
            bool    CoverCheck;         // of successful cover check
            bool    NeighborCheck;      // of waiting neighbors' response of the sent cover checks

            bool    FreezeRM;

            _MoveRequest()
                : WaitCO(false)
                , CoverCheck(false)
                , NeighborCheck(false)
                , FreezeRM(false)
            {}

            bool Freezed()
            {
                return FreezeRM;
            }

            void Freeze()
            {
                if(CoverCheck || NeighborCheck || WaitCO){
                    FreezeRM = true;
                    return;
                }

                // otherwise it's an error
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "can't freeze current RM");
                g_MonoServer->Restart();
            }

            void Clear()
            {
                WaitCO            = false;
                CoverCheck        = false;
                NeighborCheck     = false;
                FreezeRM          = false;
            }
        }MoveRequest;

        typedef struct _CORecord{
            uint32_t UID;
            uint32_t AddTime;

            uint8_t Type;

            int X;
            int Y;
            int R;

            Theron::Address PodAddress;

            _CORecord()
                : UID(0)
                , AddTime(0)
                , Type(OBJECT_UNKNOWN)
                , X(0)
                , Y(0)
                , R(0)
                , PodAddress(Theron::Address::Null())
            {}

            // TODO: maybe this name is not good
            //       maybe still invalid but this is the most we can check here
            bool Valid()
            {
                return UID && AddTime && PodAddress && (X >= 0) && (Y >= 0);
            }
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
        // the static map data, won't change after initialization
        typedef struct _GroundAtrribute{
            bool CanWalk[4];

            _GroundAtrribute()
                : CanWalk {false, false, false, false}
            {}

            void ResetAttribute(size_t nIndex, bool bCanWalk)
            {
                if(nIndex >= 4){
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid arguments");
                    g_MonoServer->Restart();
                }

                GroundCanWalk[nIndex] = bCanWalk;
            }
        }GroundAttribute;

    private:
        const uint32_t m_MapID;

        const int m_X;
        const int m_Y;
        const int m_W;
        const int m_H;

        int m_SCAddressQuery;

        Theron::Address m_SCAddress;
        Theron::Address m_MapAddress;
        Theron::Address m_EmptyAddress;


        MoveRequest m_MoveRequest;
        std::vector<CORecord> m_CORecordV;
        std::array<std::array<NeighborRecord, 3>, 3> m_NeighborV2D;
        std::vector<std::vector<GroundValid>> m_GroundAtributeV2D;

    public:
        // constructor, here (nX, nY, nW, nH) are in pixel
        RegionMonitor(const Theron::Address &rstMapAddr, uint32_t nMapID, int nX, int nY, int nW, int nH)
            : Transponder()
            , m_MapID(nMapID)
            , m_X(nX)
            , m_Y(nY)
            , m_W(nW)
            , m_H(nH)
            , m_SCAddressQuery(QUERY_NA)
            , m_SCAddress(Theron::Address::Null())
            , m_MapAddress(rstMapAddr)
            , m_EmptyAddress(Theron::Address::Null())
        {
            m_MoveRequest.Clear();

            size_t nGW = nW / SYS_MAPGRIDXP;
            size_t nGH = nH / SYS_MAPGRIDYP;

            if(!(nGW && nGH)){
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid arguments");
                g_MonoServer->Restart();
            }

            m_GroundAtributeV2D.resize(nGH);
            for(auto &stGroundAttributeV: m_GroundAtributeV2D){
                stGroundAttributeV.resize(nGW);
            }
        }

        virtual ~RegionMonitor() = default;

    protected:
        bool OnlyIn(uint32_t nMapID, int nMapX, int nMapY, int nMapR)
        {
            return (nMapID == m_MapID) && RectangleInside(m_X, m_Y, m_W, m_H, nMapX - nMapR, nMapY - nMapR, 2 * nMapR, 2 * nMapR);
        }

        bool In(uint32_t nMapID, int nMapX, int nMapY)
        {
            return (nMapID == m_MapID) && PointInRectangle(nMapX, nMapY, m_X, m_Y, m_W, m_H);
        }

        bool NeighborIn(uint32_t nMapID, int nMapX, int nMapY)
        {
            return (nMapID == m_MapID) && PointInRectangle(nMapX, nMapY, m_X - m_W, m_Y - m_H, 3 * m_W, 3 * m_H);
        }

        uint32_t MapID()
        {
            return m_MapID;
        }

    public:
        template<typename... T> void ResetAttribute(size_t nGX, size_t nGY, T&&... stT)
        {
            if(!(nGY < m_GroundAtributeV2D.size() && nGX < m_GroundAtributeV2D[0].size())){
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid arguments");
                g_MonoServer->Restart();
            }

            // if access check failed it will exit
            if(AccessCheck()){
                m_GroundAtributeV2D[nGY][nGX].ResetAttribute(stT);
            }
        }

    public:
        void Operate(const MessagePack &, const Theron::Address &);

    private:
        void On_MPK_LEAVE(const MessagePack &, const Theron::Address &);
        void On_MPK_TRYMOVE(const MessagePack &, const Theron::Address &);
        void On_MPK_NEIGHBOR(const MessagePack &, const Theron::Address &);
        void On_MPK_METRONOME(const MessagePack &, const Theron::Address &);
        void On_MPK_NEWPLAYER(const MessagePack &, const Theron::Address &);
        void On_MPK_CHECKCOVER(const MessagePack &, const Theron::Address &);
        void On_MPK_ACTIONSTATE(const MessagePack &, const Theron::Address &);
        void On_MPK_UPDATECOINFO(const MessagePack &, const Theron::Address &);
        void On_MPK_TRYSPACEMOVE(const MessagePack &, const Theron::Address &);
        void On_MPK_ADDCHAROBJECT(const MessagePack &, const Theron::Address &);
        void On_MPK_QUERYSCADDRESS(const MessagePack &, const Theron::Address &);
        void On_MPK_QUERYMAPADDRESS(const MessagePack &, const Theron::Address &);

    private:
        bool GroundValid(int, int, int);
        bool CoverValid(uint32_t, uint32_t, int, int, int);

        const Theron::Address &NeighborAddress(int, int);

    private:

    private:
        void For_Update();
        void For_MoveRequest();

    private:
        int  NeighborQueryCheck();
        void NeighborClearCheck();
        bool NeighborValid(int, int, int, bool);
        void NeighborSendCheck(uint32_t, uint32_t, int, int, int, bool);

    private:
        int QuerySCAddress();
        int QueryRMAddress(uint32_t, int, int, bool, const std::function<void(int, const Theron::Address &)> &);
#if defined(MIR2X_DEBUG) && (MIR2X_DEBUG >= 5)
    protected:
        const char *ClassName()
        {
            return "RegionMonitor";
        }
#endif
};
