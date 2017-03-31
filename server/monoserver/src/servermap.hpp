/*
 * =====================================================================================
 *
 *       Filename: servermap.hpp
 *        Created: 09/03/2015 03:49:00
 *  Last Modified: 03/31/2017 14:01:08
 *
 *    Description: put all non-atomic function as private
 *
 *                 Map is an transponder, rather than an ReactObject, it has ID() and
 *                 also timing mechanics, but it's that kind of ``object" like human
 *                 like monstor etc.
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
#include <cstdint>
#include <unordered_map>

#include "sysconst.hpp"
#include "metronome.hpp"
#include "activeobject.hpp"
#include "mir2xmapdata.hpp"

class ServiceCore;
class ServerObject;
class ServerMap: public ActiveObject
{
    private:
        template<typename T> using Vec2D  = std::vector<std::vector<T>>;
        typedef struct _CellState
        {
            bool Freezed;

            _CellState()
                : Freezed(false)
            {}
        }CellState;

    private:
        const uint32_t     m_ID;
        const Mir2xMapData m_Mir2xMapData;

    private:
        Metronome      *m_Metronome;
        ServiceCore    *m_ServiceCore;

    private:
        Vec2D<CellState> m_CellStateV2D;
        Vec2D<std::vector<ServerObject *>> m_ObjectV2D;

    private:
        void Operate(const MessagePack &, const Theron::Address &);

    public:
        ServerMap(ServiceCore *, uint32_t);
       ~ServerMap() = default;

    public:
        uint32_t ID() const { return m_ID; }

    public:
        bool In(uint32_t nMapID, int nX, int nY) const
        {
            return (nMapID == m_ID) && ValidC(nX, nY);
        }

    public:
        bool GroundValid(int nX, int nY) const
        {
            return true
                && (m_Mir2xMapData.Valid())
                && (m_Mir2xMapData.ValidC(nX, nY))
                && (m_Mir2xMapData.Cell(nX, nY).Param & 0X80000000)
                && (m_Mir2xMapData.Cell(nX, nY).Param & 0X00800000);
        }

    public:
        int W() const { return m_Mir2xMapData.Valid() ? m_Mir2xMapData.W() : 0; }
        int H() const { return m_Mir2xMapData.Valid() ? m_Mir2xMapData.H() : 0; }

        bool ValidC(int nX, int nY) const { return m_Mir2xMapData.ValidC(nX, nY); }
        bool ValidP(int nX, int nY) const { return m_Mir2xMapData.ValidP(nX, nY); }

    private:
        bool Load(const char *);

    private:
        bool CanMove(int, int);

    private:
        void On_MPK_HI(const MessagePack &, const Theron::Address &);
        void On_MPK_LEAVE(const MessagePack &, const Theron::Address &);
        void On_MPK_ACTION(const MessagePack &, const Theron::Address &);
        void On_MPK_TRYMOVE(const MessagePack &, const Theron::Address &);
        void On_MPK_METRONOME(const MessagePack &, const Theron::Address &);
        void On_MPK_PULLCOINFO(const MessagePack &, const Theron::Address &);
        void On_MPK_TRYSPACEMOVE(const MessagePack &, const Theron::Address &);
        void On_MPK_ADDCHAROBJECT(const MessagePack &, const Theron::Address &);

#if defined(MIR2X_DEBUG) && (MIR2X_DEBUG >= 5)
    protected:
        const char *ClassName()
        {
            return "ServerMap";
        }
#endif
};
