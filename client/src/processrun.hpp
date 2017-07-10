/*
 * =====================================================================================
 *
 *       Filename: processrun.hpp
 *        Created: 08/31/2015 03:42:07
 *  Last Modified: 07/10/2017 15:14:48
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
#include <cstdint>
#include <unordered_map>

#include "myhero.hpp"
#include "process.hpp"
#include "message.hpp"
#include "mir2xmap.hpp"
#include "focustype.hpp"
#include "creature.hpp"
#include "mir2xmapdata.hpp"
#include "controlboard.hpp"
#include "clientluamodule.hpp"

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
        std::map<uint32_t, Creature*> m_CreatureRecord;

    private:
        // used to keep a record of the UID under track/attack
        // if it moved we need to re-calculate the track path to get it
        int m_AttackUIDX;
        int m_AttackUIDY;

    private:
        int LoadMap(uint32_t);

    public:
        ProcessRun();
        virtual ~ProcessRun() = default;

    public:
        virtual int ID()
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
        void Net_ACTION(const uint8_t *, size_t);
        void Net_LOGINOK(const uint8_t *, size_t);
        void Net_CORECORD(const uint8_t *, size_t);
        void Net_UPDATEHP(const uint8_t *, size_t);
        void Net_DEADFADEOUT(const uint8_t *, size_t);
        void Net_MONSTERGINFO(const uint8_t *, size_t);

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
        std::vector<int> GetPlayerList();

    public:
        bool RegisterLuaExport(ClientLuaModule *, int);
        bool AddOPLog(int, int, const char *, const char *);

    public:
        Creature *RetrieveUID(uint32_t);
        bool LocateUID(uint32_t, int *, int *);

    private:
        bool TrackAttack(bool, uint32_t);
};
