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
#include "guimanager.hpp"
#include "wilanitimer.hpp"
#include "delaycommand.hpp"
#include "lochashtable.hpp"
#include "mir2xmapdata.hpp"
#include "fixedlocmagic.hpp"
#include "followuidmagic.hpp"
#include "clientcreature.hpp"
#include "clientluamodule.hpp"

class ClientPathFinder;
class ProcessRun: public Process
{
    private:
        struct UserCommand
        {
            std::string command;
            std::function<int(const std::vector<std::string> &)> callback;

            UserCommand(const char *cmdString, std::function<int(const std::vector<std::string> &)> cmdCB)
                : command(cmdString ? cmdString : "")
                , callback(cmdCB)
            {
                if(command.empty()){
                    throw fflerror("empty command name");
                }

                if(!callback){
                    throw fflerror("command callback is not callable: %s", command.c_str());
                }
            }
        };

    private:
        std::vector<UserCommand> m_userCommandList;

    private:
        uint32_t     m_mapID;
        Mir2xMapData m_mir2xMapData;

    private:
        LocHashTable<std::vector<uint32_t>> m_groundItemIDList;

    private:
        DelayCommandQueue m_delayCmdQ;

    private:
        uint64_t m_myHeroUID;

    private:
        FPSMonitor m_fps;

    private:
        LocHashTable<uint64_t> m_strikeGridList;

    private:
        bool m_drawMagicKey = true;

    public:
        bool validC(int nX, int nY) const
        {
            return m_mir2xMapData.validC(nX, nY);
        }

        bool groundValid(int nX, int nY) const
        {
            return m_mir2xMapData.validC(nX, nY) && m_mir2xMapData.cell(nX, nY).land.canThrough();
        }

    private:
        std::array<uint64_t, FOCUS_END> m_focusUIDTable;

    private:
        int m_viewX;
        int m_viewY;

    private:
        bool m_mapScrolling;

    private:
        WilAniTimer m_aniTimer;

    private:
        ClientLuaModule m_luaModule;

    private:
        GUIManager m_GUIManager;

    private:
        std::list<std::unique_ptr<FixedLocMagic>> m_fixedLocMagicList;
        std::list<std::unique_ptr<FollowUIDMagic>> m_followUIDMagicList;

    private:
        std::unordered_map<uint64_t, std::unique_ptr<ClientCreature>> m_coList;

    private:
        std::set<uint64_t> m_actionBlocker;

    private:
        LabelBoard m_mousePixlLoc;
        LabelBoard m_mouseGridLoc;

    private:
        std::list<std::shared_ptr<AscendStr>> m_ascendStrList;

    private:
        bool     m_lastPingDone = true;
        uint32_t m_lastPingTick = 0;

    private:
        double m_starRatio = 0.0;
        double m_iconRatio = 0.0;

    private:
        void scrollMap();

    private:
        void loadMap(uint32_t, int, int);

    public:
        ProcessRun();
        virtual ~ProcessRun() = default;

    public:
        int id() const override
        {
            return PROCESSID_RUN;
        }

        uint32_t mapID() const
        {
            return m_mapID;
        }

    public:
        void Notify(int, const char *);
        void Notify(const char *, const std::map<std::string, std::function<void()>> &);

    public:
        virtual void draw() const override;
        virtual void update(double) override;
        virtual void processEvent(const SDL_Event &) override;

    public:
        std::tuple<int, int> fromPLoc2Grid(int pixelX, int pixelY) const
        {
            return {(pixelX + m_viewX) / SYS_MAPGRIDXP, (pixelY + m_viewY) / SYS_MAPGRIDYP};
        }

        std::tuple<int, int> getViewShift() const
        {
            return {m_viewX, m_viewY};
        }

        std::tuple<int, int> getMouseGLoc() const
        {
            const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc();
            return fromPLoc2Grid(mousePX, mousePY);
        }

    public:
        bool onMap(uint32_t id, int nX, int nY) const
        {
            return (mapID() == id) && m_mir2xMapData.validC(nX, nY);
        }

        bool onMap(int x, int y) const
        {
            return m_mir2xMapData.validC(x, y);
        }

    public:
        void net_EXP(const uint8_t *, size_t);
        void net_BUFF(const uint8_t *, size_t);
        void net_BUFFIDLIST(const uint8_t *, size_t);
        void net_MISS(const uint8_t *, size_t);
        void net_TEXT(const uint8_t *, size_t);
        void net_PING(const uint8_t *, size_t);
        void net_GOLD(const uint8_t *, size_t);
        void net_INVOPCOST(const uint8_t *, size_t);
        void net_ACTION(const uint8_t *, size_t);
        void net_OFFLINE(const uint8_t *, size_t);
        void net_NPCSELL(const uint8_t *, size_t);
        void net_STARTGAMESCENE(const uint8_t *, size_t);
        void net_RUNTIMECONFIG(const uint8_t *, size_t);
        void net_LEARNEDMAGICLIST(const uint8_t *, size_t);
        void net_CORECORD(const uint8_t *, size_t);
        void net_HEALTH(const uint8_t *, size_t);
        void net_NEXTSTRIKE(const uint8_t *, size_t);
        void net_CASTMAGIC(const uint8_t *, size_t);
        void net_NOTIFYDEAD(const uint8_t *, size_t);
        void net_PLAYERNAME(const uint8_t *, size_t);
        void net_STRIKEGRID(const uint8_t *, size_t);
        void net_PICKUPERROR(const uint8_t *, size_t);
        void net_PLAYERWLDESP(const uint8_t *, size_t);
        void net_UPDATEITEM(const uint8_t *, size_t);
        void net_BELT(const uint8_t *, size_t);
        void net_INVENTORY(const uint8_t *, size_t);
        void net_REMOVEITEM(const uint8_t *, size_t);
        void net_REMOVESECUREDITEM(const uint8_t *, size_t);
        void net_DEADFADEOUT(const uint8_t *, size_t);
        void net_MONSTERGINFO(const uint8_t *, size_t);
        void net_SELLITEMLIST(const uint8_t *, size_t);
        void net_NPCXMLLAYOUT(const uint8_t *, size_t);
        void net_BUYERROR(const uint8_t *, size_t);
        void net_BUYSUCCEED(const uint8_t *, size_t);
        void net_GROUNDITEMIDLIST(const uint8_t *, size_t);
        void net_GROUNDFIREWALLLIST(const uint8_t *, size_t);
        void net_EQUIPWEAR(const uint8_t *, size_t);
        void net_EQUIPWEARERROR(const uint8_t *, size_t);
        void net_GRABWEAR(const uint8_t *, size_t);
        void net_GRABWEARERROR(const uint8_t *, size_t);
        void net_EQUIPBELT(const uint8_t *, size_t);
        void net_EQUIPBELTERROR(const uint8_t *, size_t);
        void net_GRABBELT(const uint8_t *, size_t);
        void net_GRABBELTERROR(const uint8_t *, size_t);
        void net_STARTINVOP(const uint8_t *, size_t);
        void net_STARTINPUT(const uint8_t *, size_t);
        void net_SHOWSECUREDITEMLIST(const uint8_t *, size_t);

    public:
        bool canMove(bool, int, int, int);
        bool canMove(bool, int, int, int, int, int);

    public:
        double MoveCost(bool, int, int, int, int);

    public:
        uint64_t getFocusUID(int) const;
        void setFocusUID(int, uint64_t);

    public:
        void updateMouseFocus()
        {
            m_focusUIDTable[FOCUS_MOUSE] = getFocusUID(FOCUS_MOUSE);
        }

    public:
        bool  luaCommand(const char *);
        bool userCommand(const char *);

    public:
        std::vector<int> GetPlayerList();

    public:
        void addCBLog(int, const char8_t *, ...);

    public:
        void RegisterUserCommand();

    public:
        void RegisterLuaExport(ClientLuaModule *);

    public:
        ClientCreature *findUID(uint64_t, bool checkVisible = true) const;

    private:
        bool trackAttack(bool, uint64_t);

    public:
        void addAscendStr(int, int, int, int);

    public:
        void centerMyHero();

    public:
        uint64_t getMyHeroUID() const
        {
            return m_myHeroUID;
        }

        MyHero *getMyHero() const
        {
            if(auto myHeroPtr = dynamic_cast<MyHero *>(findUID(getMyHeroUID()))){
                return myHeroPtr;
            }
            throw fflerror("failed to get MyHero pointer: uid = %llu", to_llu(getMyHeroUID()));
        }

    public:
        const auto &getGroundItemIDList(int x, int y) const
        {
            if(auto p = m_groundItemIDList.find({x, y}); p != m_groundItemIDList.end()){
                return p->second;
            }

            const static std::vector<uint32_t> s_emptyList;
            return s_emptyList;
        }

        void clearGroundItemIDList(int x, int y)
        {
            m_groundItemIDList.erase({x, y});
        }

    public:
        bool    hasGroundItemID(uint32_t, int, int) const;
        bool    addGroundItemID(uint32_t, int, int);
        bool removeGroundItemID(uint32_t, int, int);

    public:
        int CheckPathGrid(int, int) const;
        double OneStepCost(const ClientPathFinder *, bool, int, int, int, int, int) const;

    private:
        std::tuple<int, int> getRandLoc(uint32_t);

    public:
        void RequestKillPets();
        void requestAddExp(uint64_t);
        bool requestSpaceMove(uint32_t, int, int);

    public:
        void queryCORecord(uint64_t) const;
        void onActionSpawn(uint64_t, const ActionNode &);

    public:
        void queryUIDBuff(uint64_t) const;
        void queryPlayerWLDesp(uint64_t) const;
        void queryInvOp(int, uint32_t, uint32_t) const;

    public:
        Widget *getWidget(const std::string &widgetName)
        {
            return getGUIManager()->getWidget(widgetName);
        }

        const Widget *getWidget(const std::string &widgetName) const
        {
            return const_cast<ProcessRun *>(this)->getWidget(widgetName);
        }

    public:
        void sendNPCEvent(uint64_t, std::string, std::optional<std::string> = {});

    private:
        void drawFPS() const;
        void drawMouseLocation() const;

    private:
        void drawTile(int, int, int, int) const;
        void drawGroundItem(int, int, int, int) const;
        void drawRotateStar(int, int, int, int) const;

    private:
        void drawObject(int, int, int, bool) const;

    private:
        void checkMagicSpell(const SDL_Event &);

    public:
        std::tuple<int, int> getACNum(const std::string &) const;

    public:
        GUIManager *getGUIManager()
        {
            return &m_GUIManager;
        }

    public:
        void flipDrawMagicKey()
        {
            m_drawMagicKey = !m_drawMagicKey;
        }

    public:
        struct uidPixelLocation
        {
            int x = -1;
            int y = -1;

            operator bool () const
            {
                return x >= 0 && y >= 0;
            }
        };
        std::tuple<int, int> getUIDLoc(bool);

    public:
        FixedLocMagic *addFixedLocMagic(std::unique_ptr<FixedLocMagic> magicPtr)
        {
            m_fixedLocMagicList.emplace_back(std::move(magicPtr));
            return m_fixedLocMagicList.back().get();
        }

        FollowUIDMagic *addFollowUIDMagic(std::unique_ptr<FollowUIDMagic> magicPtr)
        {
            m_followUIDMagicList.emplace_back(std::move(magicPtr));
            return m_followUIDMagicList.back().get();
        }

    public:
        void requestPickUp();
        void requestMagicDamage(int, uint64_t);
        void requestBuy(uint64_t, uint32_t, uint32_t, size_t count);
        void requestConsumeItem(uint32_t, uint32_t, size_t);
        void requestEquipWear(uint32_t, uint32_t, int);
        void requestGrabWear(int);
        void requestEquipBelt(uint32_t, uint32_t, int);
        void requestGrabBelt(int);
        void requestDropItem(uint32_t, uint32_t, size_t);
        void requestSetMagicKey(uint32_t, char);
        void requestRemoveSecuredItem(uint32_t, uint32_t);

    public:
        std::tuple<uint32_t, int, int> getMap() const
        {
            return {m_mapID, m_mir2xMapData.w(), m_mir2xMapData.h()};
        }

        const auto &getCOList() const
        {
            return m_coList;
        }

    public:
        int getAimDirection(const ActionNode &, int defDir = DIR_NONE) const;

    public:
        void addDelay(uint32_t delayTick, std::function<void()> cmd)
        {
            m_delayCmdQ.addDelay(delayTick, std::move(cmd));
        }
};
