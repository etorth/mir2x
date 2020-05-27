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
#include "npcchatboard.hpp"
#include "controlboard.hpp"
#include "inventoryboard.hpp"
#include "quickaccessboard.hpp"

class ProcessRun;
class GUIManager: public WidgetGroup
{
    private:
        ProcessRun *m_proc;

    private:
        NPCChatBoard m_NPCChatBoard;
        ControlBoard m_controlBoard;

    private:
        InventoryBoard m_inventoryBoard;
        QuickAccessBoard m_quickAccessBoard;

    public:
        GUIManager(ProcessRun *);

    public:
        void drawEx(int, int, int, int, int, int) override;
        bool processEvent(const SDL_Event &, bool) override;

    public:
        Widget *getWidget(const std::string &);
};
