#pragma once
#include "widget.hpp"
#include "minimapboard.hpp"
#include "skillboard.hpp"
#include "npcchatboard.hpp"
#include "controlboard.hpp"
#include "purchaseboard.hpp"
#include "inventoryboard.hpp"
#include "quickaccessboard.hpp"
#include "playerstateboard.hpp"
#include "inputstringboard.hpp"
#include "runtimeconfigboard.hpp"
#include "secureditemlistboard.hpp"

class ProcessRun;
class GUIManager: public WidgetGroup
{
    private:
        ProcessRun *m_processRun;

    private:
        NPCChatBoard m_NPCChatBoard;
        ControlBoard m_controlBoard;

    private:
        SkillBoard m_skillBoard;
        MiniMapBoard m_miniMapBoard;
        PurchaseBoard m_purchaseBoard;
        InventoryBoard m_inventoryBoard;
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
        bool processEvent(const SDL_Event &, bool) override;

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
