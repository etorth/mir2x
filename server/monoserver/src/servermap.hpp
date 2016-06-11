/*
 * =====================================================================================
 *
 *       Filename: servermap.hpp
 *        Created: 09/03/2015 03:49:00 AM
 *  Last Modified: 06/10/2016 13:01:19
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

    protected:
        typedef struct _RegionMonitorRecord{
            RegionMonitor   *Data;              // pointer to RM
            bool             Need;              // this RM can contain object
            bool             Inform;            // we have informed the RM with the neighbor list
            bool             RMReady;           // the RM actor is ready for message
            Theron::Address  PodAddress;        // RM address

            _RegionMonitorRecord()
                : Data(nullptr)
                , Need(false)
                , Inform(false)
                , RMReady(false)
                , PodAddress(Theron::Address::Null())
            {}

            // this RM is fully ready and valid for accepting message
            bool Valid()
            {
                return true
                    && Data    // allocated the instance
                    && Need    // it's capable for containing objects
                    && Inform  // we have sent the neighbor list
                    && RMReady // the RM got the list and make self ready for accepting messages
                    && (PodAddress != Theron::Address::Null()); // we have the RM's address
            }

            // this RM won't need further initialization, it's possible to be invalid, invalid
            // is always ``ready" since it needs no initialization
            bool Ready()
            {
                return Need ? Valid() : true;
            }

            // half-inited, we did everything, sent neighbor list, and now we are waiting 
            // for RM's response for READY
            bool Pending()
            {
                return true
                    && Data     // allocated the instance
                    && Need     // it's capable for containing objects
                    && Inform   // we have sent the neighbor list
                    && !RMReady // the RM got the list and make self ready for accepting messages
                    && (PodAddress != Theron::Address::Null()); // we have the RM's address
            }
        }RegionMonitorRecord;

    private:
        uint32_t m_MapID;
        bool   m_RegionMonitorReady;
        size_t m_RegionW;
        size_t m_RegionH;

        Theron::Address m_SCAddress;
        Metronome *m_Metronome;

        // TODO
        // RM V2D can only be create on time
        // do I need to put the v 2d initialization in the constructor?
        bool m_RMV2DCreated;

    private:
        void Operate(const MessagePack &, const Theron::Address &);

    public:
        ServerMap(uint32_t);
        ServerMap(const Theron::Address&, uint32_t);
        ~ServerMap() = default;

    public:
        uint32_t ID()
        {
            return m_MapID;
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
        Vec2D<RegionMonitorRecord>  m_RMRecordV2D;

        void CheckRegionMonitorNeed();
        bool CheckRegionMonitorReady();

        void CreateRegionMonterV2D();

        const Theron::Address &RegionMonitorAddress(int nRMX, int nRMY)
        {
            if(false
                    || nRMY < 0
                    || nRMY >= (int)m_RMRecordV2D.size()
                    || nRMX < 0
                    || nRMX >= (int)m_RMRecordV2D[0].size()){
                return Theron::Address::Null();
            }

            return m_RMRecordV2D[nRMY][nRMX].PodAddress;
        }

        const Theron::Address &RegionMonitorAddressC(int nX, int nY)
        {
            if(!ValidC(nX, nY)){ return Theron::Address::Null(); }

            int nGridX = nX / m_RegionW;
            int nGridY = nY / m_RegionH;

            return RegionMonitorAddress(nGridX, nGridY);
        }

        const Theron::Address &RegionMonitorAddressP(int nX, int nY)
        {
            if(!ValidP(nX, nY)){ return Theron::Address::Null(); }

            int nGridX = nX / SYS_MAPGRIDXP / m_RegionW;
            int nGridY = nY / SYS_MAPGRIDYP / m_RegionH;

            return RegionMonitorAddress(nGridX, nGridY);
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
        void On_MPK_MOTIONSTATE(const MessagePack &, const Theron::Address &);
        void On_MPK_QUERYSCADDRESS(const MessagePack &, const Theron::Address &);
        void On_MPK_QUERYRMADDRESS(const MessagePack &, const Theron::Address &);
        void On_MPK_REGIONMONITORREADY(const MessagePack &, const Theron::Address &);

#ifdef MIR2X_DEBUG
    protected:
        const char *ClassName()
        {
            return "ServerMap";
        }
#endif
};
