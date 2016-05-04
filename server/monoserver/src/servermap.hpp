/*
 * =====================================================================================
 *
 *       Filename: servermap.hpp
 *        Created: 09/03/2015 03:49:00 AM
 *  Last Modified: 05/03/2016 22:38:18
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

#include <list>
#include <vector>
#include <cstdint>
#include <forward_list>
#include <unordered_map>

#include "sysconst.hpp"
#include "mir2xmap.hpp"
#include "metronome.hpp"
#include "transponder.hpp"

class CharObject;
class RegionMonitor;
class ServerMap: public Transponder
{
    private:
        // some shortcuts for internal use only
        // for public API don't use it
        using ObjectRecord     = std::tuple<uint8_t, uint32_t, uint32_t>;
        using ObjectRecordList = std::list<ObjectRecord>;
        using LockPointer      = std::shared_ptr<std::mutex>;
        template<typename T> using Vec2D = std::vector<std::vector<T>>;

    private:
        uint32_t m_ID;
        Theron::Address m_NullAddress;
        bool   m_RegionMonitorReady;
        size_t m_RegionW;
        size_t m_RegionH;

        Metronome *m_Metronome;

        Theron::Address m_CoreAddress;

        std::unordered_map<uint64_t, CharObject *> m_CharObjectM;


    private:
        void Operate(const MessagePack &, const Theron::Address &);

    public:
        ServerMap(uint32_t);
        ~ServerMap() = default;

    public:
        uint32_t ID()
        {
            return m_ID;
        }

    private:
        Mir2xMap m_Mir2xMap;

    public:
        bool ValidC(int nX, int nY)
        {
            return m_Mir2xMap.ValidC(nX, nY);
        }

        bool ValidP(int nX, int nY)
        {
            return m_Mir2xMap.ValidP(nX, nY);
        }


    private:

        Vec2D<ObjectRecordList>  m_GridObjectRecordListV;
        Vec2D<LockPointer>       m_GridObjectRecordListLockV;

    public:
        bool Load(const char *);
        bool ObjectMove(int, int, CharObject*);
        bool RemoveObject(int, int, uint8_t, uint32_t, uint32_t);

    public:
        bool QueryObject(int, int, const std::function<void(uint8_t, uint32_t, uint32_t)> &);

    private:
        bool GetObjectList(int, int, std::list<ObjectRecord> *, std::function<bool(uint8_t nType)>);

        bool CanSafeWalk(int, int);
        bool CanMove(int, int, int, uint32_t, uint32_t);

    public:
        int W()
        {
            return m_Mir2xMap.W();
        }

        int H()
        {
            return m_Mir2xMap.H();
        }

    public:
        bool DropLocation(int, int, int, int *, int *);

    protected:
        // for region monitors
        typedef struct _RegionMonitorRecord {
            RegionMonitor   *Data;
            Theron::Address  PodAddress;
            bool             Need;
            bool             CanRun;

            _RegionMonitorRecord()
                : Data(nullptr)
                , PodAddress(Theron::Address::Null())
                , Need(false)
                , CanRun(false)
            {}

            bool Valid()
            {
                return Data && (PodAddress != Theron::Address::Null());
            }

            bool Ready()
            {
                return Need ? Valid() : true;
            }
        } RegionMonitorRecord;

        Vec2D<RegionMonitorRecord>  m_RegionMonitorRecordV2D;

        void CheckRegionMonitorNeed();
        bool CheckRegionMonitorReady();

        void CreateRegionMonterV2D();

        const Theron::Address &RegionMonitorAddressP(int nX, int nY)
        {
            if(!ValidP(nX, nY)){ return m_NullAddress; }

            int nGridX = nX / SYS_MAPGRIDXP / m_RegionW;
            int nGridY = nY / SYS_MAPGRIDYP / m_RegionH;

            return m_RegionMonitorRecordV2D[nGridY][nGridX].PodAddress;
        }

        bool RegionMonitorReady()
        {
            return m_RegionMonitorReady;
        }

    private:
        void On_MPK_HI(const MessagePack &, const Theron::Address &);
        void On_MPK_METRONOME(const MessagePack &, const Theron::Address &);
        void On_MPK_ADDMONSTER(const MessagePack &, const Theron::Address &);
        void On_MPK_NEWMONSTER(const MessagePack &, const Theron::Address &);
        void On_MPK_REGIONMONITORREADY(const MessagePack &, const Theron::Address &);
};
