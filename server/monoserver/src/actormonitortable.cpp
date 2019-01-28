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

#include "strfunc.hpp"
#include "uidfunc.hpp"
#include "actorpool.hpp"
#include "actormonitortable.hpp"

extern ActorPool *g_ActorPool;

ActorMonitorTable::ActorMonitorTable(int nX, int nY, int nW, int nH, const char *szLabel)
    : Fl_TableImpl(nX, nY, nW, nH, szLabel)
    , m_ColumnName
      {
          "UID", "TYPE", "GROUP", "LIVE", "BUSY", "MSG_DONE", "MSG_PENDING"
      }
    , m_ActorMonitorList()
    , m_QueryTimer()
    , m_SortByCol(-1)
    , m_SelectedUID(0)
{
    // begin
    {
        rows(0);
        row_header(0);
        row_height_all(20);
        row_resize(0);

        cols(m_ColumnName.size());

        col_header(1);
        col_resize_min(80);
        col_width_all(100);
        col_resize(0);
    }
    end();

    m_ActorMonitorList.clear();
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

std::string ActorMonitorTable::GetGridData(int nRow, int nCol)
{
    if(nRow >= (int)(m_ActorMonitorList.size()) || nCol >= (int)(m_ColumnName.size())){
        return "";
    }

    auto fnAdjustLength = [](std::string szString, size_t nNewLength) -> std::string
    {
        if(nNewLength <= szString.length()){
            return szString;
        }
        return std::string(nNewLength - szString.size(), ' ') + szString;
    };

    const auto &rstMonitor = m_ActorMonitorList[nRow];
    switch(nCol){
        case 0: // UID
            {
                return str_printf("%016" PRIx64, rstMonitor.UID);
            }
        case 1: // TYPE
            {
                return UIDFunc::GetUIDTypeString(rstMonitor.UID);
            }
        case 2: // GROUP
            {
                return std::to_string(g_ActorPool->UIDGroup(rstMonitor.UID));
            }
        case 3: // LIVE
            {
                return GetTimeString(rstMonitor.LiveTick);
            }
        case 4: // BUSY
            {
                return GetTimeString(rstMonitor.BusyTick);
            }
        case 5: // MSG_DONE
            {
                return fnAdjustLength(std::to_string(rstMonitor.MessageDone), std::to_string(m_MonitorDataDiags.MaxMessageDone).size());
            }
        case 6: // MSG_PENDING
            {
                return fnAdjustLength(std::to_string(rstMonitor.MessagePending), std::to_string(m_MonitorDataDiags.MaxMessagePending).size());
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
                if(m_QueryTimer.diff_msec() >= 1300){
                    OnActorMonitorListUpdate();
                    m_QueryTimer.reset();
                }

                if(SelectUIDRow(m_SelectedUID) < 0){
                    m_SelectedUID = 0;
                }
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

    if(m_ActorMonitorList[nRow].UID == m_SelectedUID){
        fg_color = FL_WHITE;
        bg_color = 0xaa4444;
    }

    if(m_SortByCol >= 0 && m_SortByCol == nCol){
        fg_color = FL_WHITE;
        bg_color = 0x44aa44;
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
        stDiags.MaxMessageDone    = (std::max<uint32_t>)(stDiags.MaxMessageDone,    rstMonitor.MessageDone);
        stDiags.MaxMessagePending = (std::max<uint32_t>)(stDiags.MaxMessagePending, rstMonitor.MessagePending);

        ++stDiags.UIDTypeList[UIDFunc::GetUIDType(rstMonitor.UID)];
    }
    return stDiags;
}

void ActorMonitorTable::SetupHeaderWidth()
{
    auto fnHeaderWidth = [this](int nCol) -> int
    {
        if(nCol >= 0 && nCol < (int)(m_ColumnName.size())){
            return 10 + m_ColumnName[nCol].size() * 10;
        }
        return 100;
    };

    if(m_ActorMonitorList.empty()){
        for(size_t nIndex = 0; nIndex < m_ColumnName.size(); ++nIndex){
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

void ActorMonitorTable::OnActorMonitorListUpdate()
{
    m_ActorMonitorList = g_ActorPool->GetActorMonitor();
    m_MonitorDataDiags = GetMonitorDataDiags(m_ActorMonitorList);

    rows(m_ActorMonitorList.size());

    ResetSort();
    if(SelectUIDRow(m_SelectedUID) < 0){
        m_SelectedUID = 0;
    }
    SetupHeaderWidth();
}

int ActorMonitorTable::FindUIDRow(uint64_t nUID)
{
    for(int nIndex = 0; nIndex < (int)(m_ActorMonitorList.size()); ++nIndex){
        if(m_ActorMonitorList[nIndex].UID == nUID){
            return nIndex;
        }
    }
    return -1;
}

void ActorMonitorTable::ResetSort()
{
    if(m_SortByCol < 0){
        return;
    }

    std::sort(m_ActorMonitorList.begin(), m_ActorMonitorList.end(), [this](const ActorPool::ActorMonitor &lhs, const ActorPool::ActorMonitor &rhs) -> bool
    {
        bool bSortRes = false;
        switch(m_SortByCol){
            case 0:
                {
                    bSortRes = (lhs.UID < rhs.UID);
                    break;
                }
            case 1:
                {
                    bSortRes = (UIDFunc::GetUIDType(lhs.UID) < UIDFunc::GetUIDType(rhs.UID));
                    break;
                }
            case 2:
                {
                    bSortRes = (g_ActorPool->UIDGroup(lhs.UID) < g_ActorPool->UIDGroup(rhs.UID));
                    break;
                }
            case 3:
                {
                    bSortRes = (lhs.LiveTick < rhs.LiveTick);
                    break;
                }
            case 4:
                {
                    bSortRes = (lhs.BusyTick < rhs.BusyTick);
                    break;
                }
            case 5:
                {
                    bSortRes = (lhs.MessageDone < rhs.MessageDone);
                    break;
                }
            case 6:
                {
                    bSortRes = (lhs.MessagePending < rhs.MessagePending);
                    break;
                }
            default:
                {
                    break;
                }
        }
        return m_SortOrder ? bSortRes : !bSortRes;
    });
}

int ActorMonitorTable::SelectUIDRow(uint64_t nUID)
{
    if(!nUID){
        return -1;
    }

    int nUIDRow = -1;
    for(int nIndex = 0; nIndex < GetDataRow(); ++nIndex){
        if(m_ActorMonitorList[nIndex].UID == nUID){
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
        m_SortByCol   = -1;
        m_SortOrder   =  true;
        m_SelectedUID =  0;

        ResetSort();
        redraw();
        return;
    }

    // setup new UID to hightlight
    if((nCtx == CONTEXT_CELL) && (nRow >= 0 && nRow < GetDataRow()) && (nCol >= 0) && (nCol < GetDataCol())){
        m_SelectedUID = m_ActorMonitorList[nRow].UID;
        redraw();
        return;
    }

    // setup sort col
    // need to make sure this is the click on col header

    if((nCtx == CONTEXT_COL_HEADER) && (nRow == 0) && (nCol >= 0 && nCol < GetDataCol())){
        if(m_SortByCol == nCol){
            m_SortOrder = !m_SortOrder;
        }else{
            m_SortByCol = nCol;
            m_SortOrder = true;
        }

        ResetSort();
        redraw();
        return;
    }
}
