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
#include <algorithm>
#include <FL/fl_draw.H>

#include "strf.hpp"
#include "uidf.hpp"
#include "typecast.hpp"
#include "actorpool.hpp"
#include "actormonitortable.hpp"

extern ActorPool *g_actorPool;

ActorMonitorTable::ActorMonitorTable(int nX, int nY, int nW, int nH, const char *szLabel)
    : Fl_TableImpl(nX, nY, nW, nH, szLabel)
    , m_columnName
      {
          "UID", "TYPE", "GROUP", "LIVE", "BUSY", "MSG_DONE", "MSG_PENDING"
      }
    , m_actorMonitorList()
    , m_sortByCol(-1)
    , m_selectedUID(0)
{
    // begin
    {
        rows(0);
        row_header(0);
        row_height_all(20);
        row_resize(0);

        cols(m_columnName.size());

        col_header(1);
        col_resize_min(80);
        col_width_all(100);
        col_resize(0);
    }
    end();

    m_actorMonitorList.clear();
    SetupHeaderWidth();

    // register callbacks
    callback([](Fl_Widget *, void *pData) -> void
    {
        ((ActorMonitorTable *)(pData))->OnClick();
    }, this);
    when(FL_WHEN_RELEASE);
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

std::string ActorMonitorTable::GetGridData(int nRow, int nCol) const
{
    if(nRow >= (int)(m_actorMonitorList.size()) || nCol >= (int)(m_columnName.size())){
        return "";
    }

    auto fnAdjustLength = [](std::string szString, size_t nNewLength) -> std::string
    {
        if(nNewLength <= szString.length()){
            return szString;
        }
        return std::string(nNewLength - szString.size(), ' ') + szString;
    };

    const auto &monitor = m_actorMonitorList.at(nRow);
    switch(nCol){
        case 0: // UID
            {
                return str_printf("%016llx", to_llu(monitor.uid));
            }
        case 1: // TYPE
            {
                return uidf::getUIDTypeString(monitor.uid);
            }
        case 2: // GROUP
            {
                return std::to_string(g_actorPool->getBucketID(monitor.uid));
            }
        case 3: // LIVE
            {
                return GetTimeString(monitor.liveTick);
            }
        case 4: // BUSY
            {
                return GetTimeString(monitor.busyTick);
            }
        case 5: // MSG_DONE
            {
                return fnAdjustLength(std::to_string(monitor.messageDone), std::to_string(m_monitorDataDiags.MaxMessageDone).size());
            }
        case 6: // MSG_PENDING
            {
                return fnAdjustLength(std::to_string(monitor.messagePending), std::to_string(m_monitorDataDiags.MaxMessagePending).size());
            }
        default:
            {
                return "???";
            }
    }
}

void ActorMonitorTable::draw_cell(TableContext nContext, int nRow, int nCol, int nX, int nY, int nW, int nH)
{
    switch(nContext){
        case CONTEXT_STARTPAGE:
            {
                return; 
            }
        case CONTEXT_COL_HEADER:
            {
                DrawHeader(m_columnName[nCol].c_str(), nX, nY, nW, nH);
                return;
            }
        case CONTEXT_ROW_HEADER:
            {
                DrawHeader("???", nX, nY, nW, nH);
                return;
            }
        case CONTEXT_CELL:
            {
                DrawData(nRow, nCol, nX, nY, nW, nH);
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

void ActorMonitorTable::DrawData(int nRow, int nCol, int nX, int nY, int nW, int nH)
{
    int fg_color = FL_BLACK;
    int bg_color = FL_WHITE;

    bool bSelectedRow = (m_actorMonitorList[nRow].uid == m_selectedUID);
    bool bSelectedCol = (m_sortByCol >= 0 && m_sortByCol == nCol);

    if(bSelectedRow){
        fg_color = FL_WHITE;
        bg_color = 0xaa4444;
    }

    if(bSelectedCol){
        fg_color = FL_WHITE;
        bg_color = 0x44aa44;
    }

    if(bSelectedRow && bSelectedCol){
        fg_color = FL_WHITE;
        bg_color = 0x4444aa;
    }

    fl_push_clip(nX, nY, nW, nH);
    {
        fl_color(bg_color);
        fl_rectf(nX, nY, nW, nH);

        fl_color(fg_color);
        fl_draw(GetGridData(nRow, nCol).c_str(), nX, nY, nW, nH, FL_ALIGN_CENTER);

        fl_color(color());
        fl_rect(nX, nY, nW, nH);
    }
    fl_pop_clip();
}

ActorMonitorTable::MonitorDataDiags ActorMonitorTable::GetMonitorDataDiags(const std::vector<ActorPool::ActorMonitor> &rstMonitorList)
{
    MonitorDataDiags stDiags;
    std::memset(&stDiags, 0, sizeof(stDiags));

    for(const auto &rstMonitor: rstMonitorList){
        stDiags.MaxMessageDone    = (std::max<uint32_t>)(stDiags.MaxMessageDone,    rstMonitor.messageDone);
        stDiags.MaxMessagePending = (std::max<uint32_t>)(stDiags.MaxMessagePending, rstMonitor.messagePending);

        ++stDiags.UIDTypeList[uidf::getUIDType(rstMonitor.uid)];
    }
    return stDiags;
}

void ActorMonitorTable::SetupHeaderWidth()
{
    auto fnHeaderWidth = [this](int nCol) -> int
    {
        if(nCol >= 0 && nCol < (int)(m_columnName.size())){
            return 10 + m_columnName[nCol].size() * 10;
        }
        return 100;
    };

    if(m_actorMonitorList.empty()){
        for(size_t nIndex = 0; nIndex < m_columnName.size(); ++nIndex){
            col_width(nIndex, fnHeaderWidth(nIndex));
        }
    }else{
        col_width(0, (std::max<int>)(fnHeaderWidth(0), 150)); // UID
        col_width(1, (std::max<int>)(fnHeaderWidth(1),  80)); // TYPE
        col_width(2, (std::max<int>)(fnHeaderWidth(2),  30)); // GROUP
        col_width(3, (std::max<int>)(fnHeaderWidth(3), 160)); // LIVE
        col_width(4, (std::max<int>)(fnHeaderWidth(4), 160)); // BUSY
        col_width(5, (std::max<int>)(fnHeaderWidth(5), 120)); // MSG_DONE
        col_width(6, (std::max<int>)(fnHeaderWidth(6),  30)); // MSG_PENDING
    }
}

void ActorMonitorTable::UpdateTable()
{
    m_actorMonitorList = g_actorPool->getActorMonitor();
    m_monitorDataDiags = GetMonitorDataDiags(m_actorMonitorList);

    if(rows() != (int)(m_actorMonitorList.size())){
        rows(m_actorMonitorList.size());
    }

    ResetSort();
    if(SelectUIDRow(m_selectedUID) < 0){
        m_selectedUID = 0;
    }
    SetupHeaderWidth();
}

int ActorMonitorTable::FindUIDRow(uint64_t nUID) const
{
    for(int nIndex = 0; nIndex < (int)(m_actorMonitorList.size()); ++nIndex){
        if(m_actorMonitorList[nIndex].uid == nUID){
            return nIndex;
        }
    }
    return -1;
}

void ActorMonitorTable::ResetSort()
{
    if(m_sortByCol < 0){
        return;
    }

    std::sort(m_actorMonitorList.begin(), m_actorMonitorList.end(), [this](const ActorPool::ActorMonitor &lhs, const ActorPool::ActorMonitor &rhs) -> bool
    {
        auto fnArgedCompare = [this](const auto &x, const auto &y) -> bool
        {
            if(m_sortOrder){
                return x < y;
            }
            else{
                return x > y;
            }
        };

        switch(m_sortByCol){
            case 0 : return fnArgedCompare(lhs.uid, rhs.uid);
            case 1 : return fnArgedCompare(uidf::getUIDType(lhs.uid), uidf::getUIDType(rhs.uid));
            case 2 : return fnArgedCompare(g_actorPool->getBucketID(lhs.uid), g_actorPool->getBucketID(rhs.uid));
            case 3 : return fnArgedCompare(lhs.liveTick, rhs.liveTick);
            case 4 : return fnArgedCompare(lhs.busyTick, rhs.busyTick);
            case 5 : return fnArgedCompare(lhs.messageDone, rhs.messageDone);
            case 6 : return fnArgedCompare(lhs.messagePending, rhs.messagePending);
            default: return fnArgedCompare(&lhs, &rhs); // keep everything as it or reversed
        }
    });
}

int ActorMonitorTable::SelectUIDRow(uint64_t nUID)
{
    if(!nUID){
        return -1;
    }

    int nUIDRow = -1;
    for(int nIndex = 0; nIndex < GetDataRow(); ++nIndex){
        if(m_actorMonitorList[nIndex].uid == nUID){
            select_row(nIndex, 1);
            nUIDRow = nIndex;
        }else{
            select_row(nIndex, 0);
        }
    }
    return nUIDRow;
}

void ActorMonitorTable::OnClick()
{
    // I don't know the reason
    // the document says if setup when(FL_WHEN_RELEASE) we should only receive on release the button
    // but I get event on both press and release

    if(Fl::event() != FL_RELEASE){
        return;
    }

    auto nRow = callback_row();
    auto nCol = callback_col();
    auto nCtx = callback_context();

    // setup new UID to hightlight
    // seems if click on table's dead zone, this will return row and col both as ZERO
    // but should I relay on this???
    if((nCtx == CONTEXT_TABLE) /* ((nRow == 0 ) && (nCol == 0)) */){
        m_sortByCol   = -1;
        m_sortOrder   =  true;
        m_selectedUID =  0;

        ResetSort();
        redraw();
        return;
    }

    // setup new UID to hightlight
    if((nCtx == CONTEXT_CELL) && (nRow >= 0 && nRow < GetDataRow()) && (nCol >= 0) && (nCol < GetDataCol())){
        m_selectedUID = m_actorMonitorList[nRow].uid;
        redraw();
        return;
    }

    // setup sort col
    // need to make sure this is the click on col header

    if((nCtx == CONTEXT_COL_HEADER) && (nRow == 0) && (nCol >= 0 && nCol < GetDataCol())){
        if(m_sortByCol == nCol){
            m_sortOrder = !m_sortOrder;
        }else{
            m_sortByCol = nCol;
            m_sortOrder = true;
        }

        ResetSort();
        redraw();
        return;
    }
}
