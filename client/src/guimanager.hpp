#pragma once
#include "widget.hpp"
#include "minimapboard.hpp"
#include "horseboard.hpp"
#include "gui/skillboard/skillboard.hpp"
#include "gui/acutionboard/acutionboard.hpp"
#include "guildboard.hpp"
#include "gui/npcchatboard/npcchatboard.hpp"
#include "gui/friendchatboard/friendchatboard.hpp"
#include "gui/controlboard/controlboard.hpp"
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
        AcutionBoard m_acutionBoard;
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
        bool processEventDefault(const SDL_Event &, bool, int, int) override;

    public:
        Widget *getWidget(const std::string_view &);

    public:
        const Widget *getWidget(const std::string_view &name) const
        {
            return const_cast<GUIManager *>(this)->getWidget(name);
        }

    private:
        void onWindowResize();
};
