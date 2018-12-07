/*
 * =====================================================================================
 *
 *       Filename: actormonitortable.hpp
 *        Created: 12/04/2018 23:03:26
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
#include <vector>
#include <string>
#include "uidfunc.hpp"
#include "actorpool.hpp"
#include "raiitimer.hpp"
#include "fltableimpl.hpp"

class ActorMonitorTable: public Fl_TableImpl
{
    private:
        struct MonitorDataDiags
        {
            uint32_t MaxMessageDone;
            uint32_t MaxMessagePending;
            uint32_t UIDTypeList[UID_MAX];

        }m_MonitorDataDiags;

    private:
        const std::vector<std::string> m_ColumnName;

    private:
        std::vector<ActorPool::ActorMonitor> m_ActorMonitorList;

    private:
        hres_timer m_QueryTimer;

    private:
        int  m_SortByCol;
        bool m_SortOrder;

    private:
        uint64_t m_SelectedUID;

    public:
        ActorMonitorTable(int, int, int, int, const char * = nullptr);

    public:
        virtual void draw_cell(TableContext, int, int, int, int, int, int);

    protected:
        void DrawData(int, int, int, int, int, int);

    protected:
        void DrawHeader(const char *, int, int, int, int);

    protected:
        std::string GetGridData(int, int);

    private:
        static MonitorDataDiags GetMonitorDataDiags(const std::vector<ActorPool::ActorMonitor> &);

    private:
        void ResetSort();
        void SetupHeaderWidth();
        void OnActorMonitorListUpdate();

    private:
        int FindUIDRow(uint64_t);

    private:
        int GetDataRow() const
        {
            return m_ActorMonitorList.size();
        }

        int GetDataCol() const
        {
            return m_ColumnName.size();
        }

    private:
        void OnClick();

    private:
        int SelectUIDRow(uint64_t);
};
