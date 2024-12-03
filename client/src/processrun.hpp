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
#include "framecounter.hpp"
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
    public:
        enum
        {
            CURSOR_NONE  = 0,
            CURSOR_BEGIN = 1,
            CURSOR_DEFAULT = CURSOR_BEGIN,
            CURSOR_TEAMFLAG,
            CURSOR_END,
        };

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
        const uint64_t m_myHeroUID;

    private:
        uint32_t m_mapID = 0;
        Mir2xMapData m_mir2xMapData;

    private:
        LocHashTable<std::vector<uint32_t>> m_groundItemIDList;

    private:
        DelayCommandQueue m_delayCmdQ;

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
        std::array<uint64_t, FOCUS_END> m_focusUIDTable {};

    private:
        int m_viewX = 0;
        int m_viewY = 0;

    private:
        bool m_mapScrolling = false;

    private:
        WilAniTimer m_aniTimer;

    private:
        ClientLuaModule m_luaModule;

    private:
        SDChatPeer m_defaultChatPeer;

    private:
        GUIManager m_guiManager;

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
        FrameCounter m_teamFlag;

    private:
        int m_cursorState = CURSOR_DEFAULT;

    private:
        void scrollMap();

    private:
        void loadMap(uint32_t, int, int);

    public:
        ProcessRun(const SMOnlineOK &);
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
#define _support_sm(sm) void on_##sm(const uint8_t *, size_t)
        _support_sm(SM_EXP);
        _support_sm(SM_BUFF);
        _support_sm(SM_BUFFIDLIST);
        _support_sm(SM_MISS);
        _support_sm(SM_TEXT);
        _support_sm(SM_PING);
        _support_sm(SM_GOLD);
        _support_sm(SM_INVOPCOST);
        _support_sm(SM_ACTION);
        _support_sm(SM_OFFLINE);
        _support_sm(SM_NPCSELL);
        _support_sm(SM_STARTGAMESCENE);
        _support_sm(SM_PLAYERCONFIG);
        _support_sm(SM_FRIENDLIST);
        _support_sm(SM_LEARNEDMAGICLIST);
        _support_sm(SM_CORECORD);
        _support_sm(SM_HEALTH);
        _support_sm(SM_NEXTSTRIKE);
        _support_sm(SM_CASTMAGIC);
        _support_sm(SM_NOTIFYDEAD);
        _support_sm(SM_PLAYERNAME);
        _support_sm(SM_STRIKEGRID);
        _support_sm(SM_PICKUPERROR);
        _support_sm(SM_PLAYERWLDESP);
        _support_sm(SM_UPDATEITEM);
        _support_sm(SM_BELT);
        _support_sm(SM_INVENTORY);
        _support_sm(SM_REMOVEITEM);
        _support_sm(SM_REMOVESECUREDITEM);
        _support_sm(SM_DEADFADEOUT);
        _support_sm(SM_MONSTERGINFO);
        _support_sm(SM_SELLITEMLIST);
        _support_sm(SM_NPCXMLLAYOUT);
        _support_sm(SM_BUYERROR);
        _support_sm(SM_BUYSUCCEED);
        _support_sm(SM_GROUNDITEMIDLIST);
        _support_sm(SM_GROUNDFIREWALLLIST);
        _support_sm(SM_EQUIPWEAR);
        _support_sm(SM_EQUIPWEARERROR);
        _support_sm(SM_GRABWEAR);
        _support_sm(SM_GRABWEARERROR);
        _support_sm(SM_EQUIPBELT);
        _support_sm(SM_EQUIPBELTERROR);
        _support_sm(SM_GRABBELT);
        _support_sm(SM_GRABBELTERROR);
        _support_sm(SM_STARTINVOP);
        _support_sm(SM_STARTINPUT);
        _support_sm(SM_SHOWSECUREDITEMLIST);
        _support_sm(SM_TEAMCANDIDATE);
        _support_sm(SM_TEAMMEMBERLIST);
        _support_sm(SM_QUESTDESPUPDATE);
        _support_sm(SM_QUESTDESPLIST);
        _support_sm(SM_CHATMESSAGELIST);
        _support_sm(SM_CREATECHATGROUP);
        _support_sm(SM_ADDFRIENDACCEPTED);
        _support_sm(SM_ADDFRIENDREJECTED);
#undef _support_sm

    public:
        bool canMove(bool, int, int, int);
        bool canMove(bool, int, int, int, int, int);

    public:
        uint64_t getFocusUID(int, bool /* allowMyHero */ = false) const;
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
        void addCBLog   (int, const char8_t *, ...);
        void addCBParLog(     const char8_t *, ...);

    public:
        void RegisterUserCommand();

    public:
        void registerLuaExport(ClientLuaModule *);

    public:
        ClientCreature *findUID(uint64_t, bool checkVisible = true) const;

    public:
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

        uint64_t getMyHeroDBID() const
        {
            return uidf::getPlayerDBID(m_myHeroUID);
        }

        MyHero *getMyHero() const
        {
            if(auto myHeroPtr = dynamic_cast<MyHero *>(findUID(getMyHeroUID()))){
                return myHeroPtr;
            }
            throw fflerror("failed to get MyHero pointer: uid = %llu", to_llu(getMyHeroUID()));
        }

        SDChatPeer getMyHeroChatPeer() const
        {
            return m_defaultChatPeer;
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
        int checkPathGrid(int, int) const;
        std::optional<double> oneStepCost(const ClientPathFinder *, bool, int, int, int, int, int, int) const;

    private:
        std::tuple<int, int> getRandLoc(uint32_t);

    public:
        void requestKillPets();
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
        auto getWidget(this auto && self, const std::string_view &widgetName)
        {
            return self.getGUIManager()->getWidget(widgetName);
        }

    public:
        void sendNPCEvent(uint64_t, std::string, std::string, std::optional<std::string> = {});

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
        auto getGUIManager(this auto && self)
        {
            return std::addressof(self.m_guiManager);
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
        void requestBuy(uint64_t, uint32_t, uint32_t, size_t);
        void requestMakeItem(uint32_t, size_t);
        void requestConsumeItem(uint32_t, uint32_t, size_t);
        void requestEquipWear(uint32_t, uint32_t, int);
        void requestGrabWear(int);
        void requestEquipBelt(uint32_t, uint32_t, int);
        void requestGrabBelt(int);
        void requestDropItem(uint32_t, uint32_t, size_t);
        void requestSetMagicKey(uint32_t, char);
        void requestRemoveSecuredItem(uint32_t, uint32_t);
        void requestJoinTeam(uint64_t);
        void requestLeaveTeam(uint64_t);
        void requestLatestChatMessage(const std::vector<uint64_t> &, size_t, bool, bool);

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

    public:
        std::shared_ptr<SDLSoundEffectChannel> playSoundEffectAt(uint32_t, int, int, size_t repeats = 1) const;

    public:
        void setCursor(int);

    public:
        template<int CfgIndex> auto getRuntimeConfig(this auto && self)
        {
            return SDRuntimeConfig_getConfig<CfgIndex>(dynamic_cast<const RuntimeConfigBoard *>(self.getWidget("RuntimeConfigBoard"))->getConfig());
        }
};
