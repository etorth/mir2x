/*
 * =====================================================================================
 *
 *       Filename: processrun.hpp
 *        Created: 08/31/2015 03:42:07
 *  Last Modified: 09/05/2017 15:06:32
 *
 *    Description: 
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
#include <map>
#include <list>
#include <cstdint>
#include <unordered_map>

#include "myhero.hpp"
#include "process.hpp"
#include "message.hpp"
#include "creature.hpp"
#include "focustype.hpp"
#include "ascendstr.hpp"
#include "grounditem.hpp"
#include "indepmagic.hpp"
#include "labelboard.hpp"
#include "mir2xmapdata.hpp"
#include "controlboard.hpp"
#include "clientluamodule.hpp"
#include "linebrowserboard.hpp"

class ProcessRun: public Process
{
    private:
        enum OutPortType: int
        {
            OUTPORT_NONE         = (0 << 0),
            OUTPORT_LOG          = (1 << 0),
            OUTPORT_SCREEN       = (2 << 1),
            OUTPORT_CONTROLBOARD = (3 << 1),
        };

    private:
        uint32_t     m_MapID;
        Mir2xMapData m_Mir2xMapData;

    private:
        MyHero *m_MyHero;

    private:
        std::array<uint32_t, FOCUS_MAX> m_FocusUIDV;

    private:
        int m_ViewX;
        int m_ViewY;

    private:
        bool m_RollMap;

    private:
        ClientLuaModule m_LuaModule;

    private:
        ControlBoard m_ControbBoard;

    private:
        std::vector<std::shared_ptr<IndepMagic>> m_IndepMagicList;

    private:
        std::vector<GroundItem  > m_GroundItemList;
        std::map<uint32_t, Creature*> m_CreatureRecord;

    private:
        // used to keep a record of the UID under track/attack
        // if it moved we need to re-calculate the track path to get it
        int m_AttackUIDX;
        int m_AttackUIDY;

    private:
        // use a tokenboard to show all in future
        LabelBoard m_PointerPixlInfo;
        LabelBoard m_PointerTileInfo;

    private:
        std::list<AscendStr *> m_AscendStrRecord;

    private:
        int LoadMap(uint32_t);

    public:
        ProcessRun();
        virtual ~ProcessRun() = default;

    public:
        int ID() const
        {
            return PROCESSID_RUN;
        }

        uint32_t MapID() const
        {
            return m_MapID;
        }

    public:
        void Notify(int, const char *);
        void Notify(const char *, const std::map<std::string, std::function<void()>> &);

    public:
        virtual void Update(double);
        virtual void Draw();
        virtual void ProcessEvent(const SDL_Event &);

    public:
        bool LocatePoint(int, int, int *, int *);

    public:
        bool OnMap(uint32_t, int, int) const;

    public:
        void Net_EXP(const uint8_t *, size_t);
        void Net_ACTION(const uint8_t *, size_t);
        void Net_LOGINOK(const uint8_t *, size_t);
        void Net_CORECORD(const uint8_t *, size_t);
        void Net_UPDATEHP(const uint8_t *, size_t);
        void Net_FIREMAGIC(const uint8_t *, size_t);
        void Net_DEADFADEOUT(const uint8_t *, size_t);
        void Net_MONSTERGINFO(const uint8_t *, size_t);
        void Net_SHOWDROPITEM(const uint8_t *, size_t);

    public:
        bool CanMove(bool, int, int);
        bool CanMove(bool, int, int, int, int);

    public:
        double MoveCost(bool, int, int, int, int);

    private:
        uint32_t FocusUID(int);

    public:
        bool  LuaCommand(const char *);
        bool UserCommand(const char *);

    public:
        // used by controlboard to draw HP/MP
        uint32_t GetControlBoardFaceKey();
        bool     GetMyHeroHMPRatio(double *, double *);

    public:
        std::vector<int> GetPlayerList();

    public:
        bool RegisterLuaExport(ClientLuaModule *, int);
        bool AddOPLog(int, int, const char *, const char *, ...);

    public:
        Creature *RetrieveUID(uint32_t);
        bool LocateUID(uint32_t, int *, int *);

    private:
        bool TrackAttack(bool, uint32_t);

    public:
        void AddAscendStr(int, int, int, int);

    public:
        bool GetUIDLocation(uint32_t, bool, int *, int *);

    public:
        void CenterMyHero();
};
