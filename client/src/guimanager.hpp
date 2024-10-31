#pragma once
#include "widget.hpp"
#include "minimapboard.hpp"
#include "horseboard.hpp"
#include "skillboard.hpp"
#include "guildboard.hpp"
#include "npcchatboard.hpp"
#include "friendchatboard.hpp"
#include "controlboard.hpp"
#include "purchaseboard.hpp"
#include "teamstateboard.hpp"
#include "inventoryboard.hpp"
#include "queststateboard.hpp"
#include "quickaccessboard.hpp"
#include "playerstateboard.hpp"
#include "inputstringboard.hpp"
#include "runtimeconfigboard.hpp"
#include "secureditemlistboard.hpp"

class ProcessRun;
class GUIManager: public Widget
{
    private:
        ProcessRun *m_processRun;

    private:
        NPCChatBoard m_NPCChatBoard;
        ControlBoard m_controlBoard;

    private:
        FriendChatBoard m_friendChatBoard;

    private:
        HorseBoard m_horseBoard;
        SkillBoard m_skillBoard;
        GuildBoard m_guildBoard;
        MiniMapBoard m_miniMapBoard;
        PurchaseBoard m_purchaseBoard;
        TeamStateBoard m_teamStateBoard;
        InventoryBoard m_inventoryBoard;
        QuestStateBoard m_questStateBoard;
        QuickAccessBoard m_quickAccessBoard;
        PlayerStateBoard m_playerStateBoard;
        InputStringBoard m_inputStringBoard;
        RuntimeConfigBoard m_runtimeConfigBoard;
        SecuredItemListBoard m_securedItemListBoard;

    public:
        GUIManager(ProcessRun *);

    public:
        void update(double) override;

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEventDefault(const SDL_Event &, bool) override;

    public:
        Widget *getWidget(const std::string &);

    public:
        const Widget *getWidget(const std::string &name) const
        {
            return const_cast<GUIManager *>(this)->getWidget(name);
        }

    private:
        void onWindowResize();
};
