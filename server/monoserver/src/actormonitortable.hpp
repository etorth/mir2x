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
#include "actorpool.hpp"
#include "fltableimpl.hpp"

class ActorMonitorTable: public Fl_TableImpl
{
    private:
        const std::vector<std::string> m_ColumnName;

    private:
        std::vector<ActorPool::ActorMonitor> m_ActorMonitorList;

    public:
        ActorMonitorTable(int, int, int, int, const char * = nullptr);

    public:
        virtual void draw_cell(TableContext, int, int, int, int, int, int);

    protected:
        void DrawData  (const char *, int, int, int, int);
        void DrawHeader(const char *, int, int, int, int);

    protected:
        std::string GetGridData(int, int);
};
