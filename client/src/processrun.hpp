/*
 * =====================================================================================
 *
 *       Filename: processrun.hpp
 *        Created: 08/31/2015 03:42:07
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
#include <set>
#include <map>
#include <list>
#include <tuple>
#include <memory>
#include <cstdint>
#include <algorithm>

#include "myhero.hpp"
#include "process.hpp"
#include "message.hpp"
#include "focustype.hpp"
#include "ascendstr.hpp"
#include "commonitem.hpp"
#include "indepmagic.hpp"
#include "lochashtable.hpp"
#include "mir2xmapdata.hpp"
#include "npcchatboard.hpp"
#include "controlboard.hpp"
#include "clientcreature.hpp"
#include "inventoryboard.hpp"
#include "clientluamodule.hpp"
#include "quickaccessboard.hpp"

class ClientPathFinder;
class ProcessRun: public Process
{
    private:
        struct UserCommandEntry
        {
            std::string Command;
            std::function<int(const std::vector<std::string> &)> Callback;

            UserCommandEntry(const char *szCommand, std::function<int(const std::vector<std::string> &)> rfnOP)
                : Command(szCommand ? szCommand : "")
                , Callback(rfnOP)
            {}
        };

    private:
        enum OutPortType: int
        {
            OUTPORT_NONE         = (0 << 0),
            OUTPORT_LOG          = (1 << 0),
            OUTPORT_SCREEN       = (2 << 1),
            OUTPORT_CONTROLBOARD = (3 << 1),
        };

    private:
        std::vector<UserCommandEntry> m_userCommandGroup;

    private:
        uint32_t     m_mapID;
        Mir2xMapData m_mir2xMapData;

    private:
        LocHashTable<std::vector<CommonItem>> m_groundItemList;

    private:
        uint64_t m_myHeroUID;

    private:
        FPSMonitor m_fps;

    public:
        bool ValidC(int nX, int nY) const
        {
            return m_mir2xMapData.ValidC(nX, nY);
        }

    private:
        std::array<uint64_t, FOCUS_MAX> m_focusUIDTable;

    private:
        int m_viewX;
        int m_viewY;

    private:
        bool m_mapScrolling;

    private:
        uint32_t m_aniSaveTick[8];
        uint8_t  m_aniTileFrame[8][16];

    private:
        ClientLuaModule m_luaModule;

    private:
        NPCChatBoard m_NPCChatBoard;
        ControlBoard m_controlBoard;
        InventoryBoard m_inventoryBoard;
        QuickAccessBoard m_quickAccessBoard;

    private:
        std::list<std::shared_ptr<IndepMagic>> m_indepMagicList;

    private:
        std::unordered_map<uint64_t, std::unique_ptr<ClientCreature>> m_creatureList;

    private:
        std::set<uint64_t> m_UIDPending;

    private:
        // use a tokenboard to show all in future
        LabelBoard m_fpsBoard;
        LabelBoard m_mousePixlLoc;
        LabelBoard m_mouseGridLoc;

    private:
        std::list<std::shared_ptr<AscendStr>> m_ascendStrList;

    private:
        double m_starRatio = 0.0;

    private:
        void scrollMap();

    private:
        void loadMap(uint32_t);

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
            return m_mapID;
        }

    public:
        bool UIDPending(uint64_t nUID) const
        {
            return m_UIDPending.find(nUID) != m_UIDPending.end();
        }

    public:
        void Notify(int, const char *);
        void Notify(const char *, const std::map<std::string, std::function<void()>> &);

    public:
        virtual void update(double) override;
        virtual void draw() override;
        virtual void processEvent(const SDL_Event &) override;

    public:
        bool ScreenPoint2Grid(int, int, int *, int *);

    public:
        std::tuple<int, int> ScreenPoint2Grid(int nPX, int nPY)
        {
            return {(nPX + m_viewX) / SYS_MAPGRIDXP, (nPY + m_viewY) / SYS_MAPGRIDYP};
        }


    public:
        bool OnMap(uint32_t, int, int) const;

    public:
        void Net_EXP(const uint8_t *, size_t);
        void Net_MISS(const uint8_t *, size_t);
        void Net_GOLD(const uint8_t *, size_t);
        void Net_ACTION(const uint8_t *, size_t);
        void Net_OFFLINE(const uint8_t *, size_t);
        void Net_LOGINOK(const uint8_t *, size_t);
        void Net_PICKUPOK(const uint8_t *, size_t);
        void Net_CORECORD(const uint8_t *, size_t);
        void Net_UPDATEHP(const uint8_t *, size_t);
        void Net_FIREMAGIC(const uint8_t *, size_t);
        void Net_NOTIFYDEAD(const uint8_t *, size_t);
        void Net_DEADFADEOUT(const uint8_t *, size_t);
        void Net_MONSTERGINFO(const uint8_t *, size_t);
        void Net_SHOWDROPITEM(const uint8_t *, size_t);
        void Net_NPCXMLLAYOUT(const uint8_t *, size_t);

    public:
        bool CanMove(bool, int, int, int);
        bool CanMove(bool, int, int, int, int, int);

    public:
        double MoveCost(bool, int, int, int, int);

    private:
        uint64_t FocusUID(int);

    public:
        bool  LuaCommand(const char *);
        bool UserCommand(const char *);

    public:
        uint32_t GetFocusFaceKey();

    public:
        std::vector<int> GetPlayerList();

    public:
        void AddOPLog(int, int, const char *, const char *, ...);
        void addCBLog(int, const char *, ...);

    public:
        bool RegisterUserCommand();

    public:
        bool RegisterLuaExport(ClientLuaModule *, int);

    public:
        ClientCreature *findUID(uint64_t, bool checkVisible = true) const;

    private:
        bool trackAttack(bool, uint64_t);

    public:
        void addAscendStr(int, int, int, int);

    public:
        bool GetUIDLocation(uint64_t, bool, int *, int *);

    public:
        void centerMyHero();

    public:
        uint64_t getMyHeroUID() const
        {
            return m_myHeroUID;
        }

        MyHero *getMyHero() const
        {
            if(getMyHeroUID()){
                return dynamic_cast<MyHero *>(findUID(getMyHeroUID()));
            }
            return nullptr;
        }

    public:
        const auto &getGroundItemList(int x, int y) const
        {
            if(auto p = m_groundItemList.find({x, y}); p != m_groundItemList.end()){
                return p->second;
            }

            const static std::vector<CommonItem> emptyList;
            return emptyList;
        }

        int findGroundItem(const CommonItem &item, int x, int y) const
        {
            const auto &itemList = getGroundItemList(x, y);
            if(itemList.empty()){
                return -1;
            }

            for(int i = (int)(itemList.size()) - 1; i >= 0; --i){
                if(itemList[i] == item){
                    return i;
                }
            }
            return -1;
        }

        bool addGroundItem(const CommonItem &item, int x, int y)
        {
            if(!(item && m_mir2xMapData.ValidC(x, y))){
                return false;
            }

            m_groundItemList[{x, y}].push_back(item);
            return true;
        }

        void removeGroundItem(const CommonItem &item, int x, int y)
        {
            auto p = m_groundItemList.find({x, y});
            if(p == m_groundItemList.end()){
                return;
            }

            p->second.erase(std::remove(p->second.begin(), p->second.end(), item), p->second.end());
            if(p->second.empty()){
                m_groundItemList.erase(p);
            }
        }

        void clearGroundItem(int x, int y)
        {
            m_groundItemList.erase({x, y});
        }

    public:
        int CheckPathGrid(int, int) const;
        double OneStepCost(const ClientPathFinder *, bool, int, int, int, int, int) const;

    public:
        void RequestKillPets();
        bool RequestSpaceMove(uint32_t, int, int);

    public:
        void queryCORecord(uint64_t) const;
        void OnActionSpawn(uint64_t, const ActionNode &);

    public:
        Widget *getWidget(const std::string &);

    public:
        void sendNPCEvent(uint64_t, const char *, const char * = nullptr);

    private:
        void drawTile(int, int, int, int);
        void drawGroundItem(int, int, int, int);
        void drawRotateStar(int, int, int, int);

    private:
        void drawGroundObject(int, int, bool);

    public:
        std::tuple<int, int> getACNum(const std::string &) const;
};
