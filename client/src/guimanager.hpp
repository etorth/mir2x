/*
 * =====================================================================================
 *
 *       Filename: guimanager.hpp
 *        Created: 08/12/2015 09:59:15
 *    Description: public API for class client only
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
#include "widget.hpp"
#include "skillboard.hpp"
#include "npcchatboard.hpp"
#include "controlboard.hpp"
#include "purchaseboard.hpp"
#include "inventoryboard.hpp"
#include "quickaccessboard.hpp"
#include "playerstatusboard.hpp"
#include "purchasecountboard.hpp"

class ProcessRun;
class GUIManager: public WidgetGroup
{
    private:
        ProcessRun *m_proc;

    private:
        NPCChatBoard m_NPCChatBoard;
        ControlBoard m_controlBoard;

    private:
        SkillBoard m_skillBoard;
        PurchaseBoard m_purchaseBoard;
        InventoryBoard m_inventoryBoard;
        QuickAccessBoard m_quickAccessBoard;
        PlayerStatusBoard m_playerStatusBoard;
        PurchaseCountBoard m_purchaseCountBoard;

    public:
        GUIManager(ProcessRun *);

    public:
        void update(double) override;
        void drawEx(int, int, int, int, int, int) override;
        bool processEvent(const SDL_Event &, bool) override;

    public:
        Widget *getWidget(const std::string &);
};
