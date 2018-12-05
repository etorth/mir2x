/*
 * =====================================================================================
 *
 *       Filename: actormonitortable.cpp
 *        Created: 12/04/2018 23:39:19
 *    Description: check FLTK/examples/table-*.cxx
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

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include "uidfunc.hpp"
#include "actorpool.hpp"
#include "actormonitortable.hpp"

extern ActorPool *g_ActorPool;

ActorMonitorTable::ActorMonitorTable(int nX, int nY, int nW, int nH, const char *szLabel)
    : Fl_TableImpl(nX, nY, nW, nH, szLabel)
    , m_ColumnName
      {
          "UID", "TYPE", "LIVE", "BUSY", "MSG_DONE", "MSG_PENDING"
      }
    , m_ActorMonitorList()
{
    rows(0);
    row_header(0);
    row_height_all(20);
    row_resize(0);

    cols(m_ColumnName.size());
    col_header(1);
    col_width_all(80);
    col_resize(1);
    end();
}

static std::string GetTimeString(uint32_t nMS)
{
    int nHour   = nMS / 3600000; nMS -= (nHour   * 3600000);
    int nMinute = nMS /   60000; nMS -= (nMinute *   60000);
    int nSecond = nMS /    1000; nMS -= (nSecond *    1000);

    char szTimeString[128];
    std::sprintf(szTimeString, "%dh:%02dm:%02ds:%03dms", nHour, nMinute, nSecond, nMS);
    return szTimeString;
}

std::string ActorMonitorTable::GetGridData(int nRow, int nCol)
{
    if(nRow >= (int)(m_ActorMonitorList.size()) || nCol >= (int)(m_ColumnName.size())){
        return "";
    }

    const auto &rstMonitor = m_ActorMonitorList[nRow];
    switch(nCol){
        case 0:
            {
                char szUIDHexString[64];
                std::sprintf(szUIDHexString, "%" PRIx64, rstMonitor.UID);
                return szUIDHexString;
            }
        case 1:
            {
                return UIDFunc::GetUIDTypeString(rstMonitor.UID);
            }
        case 2:
            {
                return GetTimeString(rstMonitor.LiveTick);
            }
        case 3:
            {
                return GetTimeString(rstMonitor.BusyTick);
            }
        case 4:
            {
                return std::to_string(rstMonitor.MessageDone);
            }
        case 5:
            {
                return std::to_string(rstMonitor.MessagePending);
            }
        default:
            {
                return "???";
            }
    }
}

void ActorMonitorTable::draw_cell(TableContext nContext, int nRow, int nCol, int nX, int nY, int nW, int nH)
{
    switch(nContext)
    {
        case CONTEXT_STARTPAGE:
            {
                m_ActorMonitorList = g_ActorPool->GetActorMonitor();
                rows(m_ActorMonitorList.size());
                return; 
            }
        case CONTEXT_COL_HEADER:
            {
                DrawHeader(m_ColumnName[nCol].c_str(), nX, nY, nW, nH);
                return;
            }
        case CONTEXT_ROW_HEADER:
            {
                DrawHeader("???", nX, nY, nW, nH);
                return;
            }
        case CONTEXT_CELL:
            {
                DrawData(GetGridData(nRow, nCol).c_str(), nX, nY, nW, nH);
                return;
            }
        default:
            {
                return;
            }
    }
}

void ActorMonitorTable::DrawHeader(const char *szInfo, int nX, int nY, int nW, int nH)
{
    fl_push_clip(nX, nY, nW, nH);
    {
        fl_draw_box(FL_THIN_UP_BOX, nX, nY, nW, nH, row_header_color());
        fl_color(FL_BLACK);
        fl_draw(szInfo, nX, nY, nW, nH, FL_ALIGN_CENTER);
    }
    fl_pop_clip();
}

void ActorMonitorTable::DrawData(const char *szInfo, int nX, int nY, int nW, int nH)
{
    fl_push_clip(nX, nY, nW, nH);
    {
        fl_color(FL_WHITE);
        fl_rectf(nX, nY, nW, nH);

        fl_color(FL_GRAY0);
        fl_draw(szInfo, nX, nY, nW, nH, FL_ALIGN_CENTER);

        fl_color(color());
        fl_rect(nX, nY, nW, nH);
    }
    fl_pop_clip();
}
