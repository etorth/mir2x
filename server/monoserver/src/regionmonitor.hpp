/*
 * =====================================================================================
 *
 *       Filename: regionmonitor.hpp
 *        Created: 04/21/2016 12:09:03
 *  Last Modified: 05/04/2016 22:49:33
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
        typedef struct _MoveRequest{
            void           *Data;           // used when adding new object

            uint8_t         Type;           // object type
            uint32_t        UID;
            uint32_t        AddTime;
            uint32_t        MPKID;          // MessagePack::ID() for response

            int             X;
            int             Y;
            int             R;

            Theron::Address PodAddress;     // address for response

            _MoveRequest()
                : Data(nullptr)
                , Type(OBJECT_UNKNOWN)
                , UID(0)
                , AddTime(0)
                , MPKID(0)
                , X(0)
                , Y(0)
                , R(0)
                , PodAddress(Theron::Address::Null())
            {}

            // means this record contains a valid record
            // maybe existing moving obj, or new obj for place
            bool Valid()
            {
                return UID != 0 && AddTime != 0;
            }

            void Clear()
            {
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
            Theron::Address PodAddress;

            _NeighborRecord()
                : Query(-1)
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
        {
            Install("Update", [this](){ For_Update(); });
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
        void On_MPK_INITREGIONMONITOR(const MessagePack &, const Theron::Address &);

    private:
        bool GroundValid(int, int, int);

    private:
        void For_Update();
        void For_MoveRequest();
};
