/*
 * =====================================================================================
 *
 *       Filename: podmonitortable.cpp
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
#include "podmonitorwindow.hpp"
#include "actormonitortable.hpp"

extern ActorPool *g_actorPool;
extern PodMonitorWindow *g_podMonitorWindow;

std::string PodMonitorTable::getGridData(int nRow, int nCol) const
{
    if(!(checkRow(nRow) && checkCol(nCol))){
        return "";
    }

    const auto fnAdjustLength = [](std::string szString, size_t nNewLength) -> std::string
    {
        if(nNewLength <= szString.length()){
            return szString;
        }
        return std::string(nNewLength - szString.size(), ' ') + szString;
    };

    const auto &monitor = m_podDrawHelper.amProcMonitorList.at(nRow);
    switch(nCol){
        case 0: // TYPE
            {
                return MessagePack(monitor.amType).Name();
            }
        case 1: // BUSY
            {
                return getTimeString(monitor.procMonitor.procTick);
            }
        case 2: // SEND
            {
                return fnAdjustLength(std::to_string(monitor.procMonitor.sendCount), std::to_string(m_podDrawHelper.maxSendCount).size());
            }
        case 3: // RECV
            {
                return fnAdjustLength(std::to_string(monitor.procMonitor.recvCount), std::to_string(m_podDrawHelper.maxRecvCount).size());
            }
        default:
            {
                return "???";
            }
    }
}

void PodMonitorTable::setupHeaderWidth()
{
    const auto fnHeaderWidth = [this](int nCol) -> int
    {
        if(checkCol(nCol)){
            return 10 + getColName(nCol).size() * 10;
        }
        return 100;
    };

    cols(getColCount());

    if(m_podDrawHelper.amProcMonitorList.empty()){
        for(int i = 0; i < getColCount(); ++i){
            col_width(i, fnHeaderWidth(i));
        }
    }
    else{
        col_width(0, (std::max<int>)(fnHeaderWidth(0), 150)); // TYPE
        col_width(1, (std::max<int>)(fnHeaderWidth(1),  80)); // BUSY
        col_width(2, (std::max<int>)(fnHeaderWidth(2),  30)); // SEND
        col_width(3, (std::max<int>)(fnHeaderWidth(3), 160)); // RECV
    }
}

void PodMonitorTable::sortTable()
{
    if(sortByCol() < 0){
        return;
    }

    std::sort(m_podDrawHelper.amProcMonitorList.begin(), m_podDrawHelper.amProcMonitorList.end(), [this](const auto &lhs, const auto &rhs) -> bool
    {
        const auto fnArgedCompare = [this](const auto &x, const auto &y) -> bool
        {
            if(sortOrder()){
                return x < y;
            }
            else{
                return x > y;
            }
        };

        switch(sortByCol()){
            case 0 : return fnArgedCompare(lhs.amType, rhs.amType);
            case 1 : return fnArgedCompare(lhs.procMonitor.procTick, rhs.procMonitor.procTick);
            case 2 : return fnArgedCompare(lhs.procMonitor.sendCount, rhs.procMonitor.sendCount);
            case 3 : return fnArgedCompare(lhs.procMonitor.recvCount, rhs.procMonitor.recvCount);
            default: return fnArgedCompare(&lhs, &rhs); // keep everything as it or reversed
        }
    });
}

int PodMonitorTable::selectAMTypeRow(int amType)
{
    if(amType == MPK_NONE){
        return -1;
    }

    int amTypeRow = -1;
    for(int i = 0; i < getRowCount(); ++i){
        if(m_podDrawHelper.amProcMonitorList.at(i).amType == amType){
            select_row(i, 1);
            amTypeRow = i;
        }
        else{
            select_row(i, 0);
        }
    }
    return amTypeRow;
}

void PodMonitorTable::setPodUID(uint64_t uid)
{
    if(!uid){
        return;
    }

    m_podUID = uid;
    updateTable();
}

void PodMonitorTable::updateTable()
{
    const auto podMonitor = g_actorPool->getPodMonitor(m_podUID);
    m_podDrawHelper = getPodMonitorDrawHelper(podMonitor);

    setupLayout();
    sortTable();

    if(selectAMTypeRow(m_selectedAMType) < 0){
        m_selectedAMType = 0;
    }
    setupHeaderWidth();
}

PodMonitorTable::PodMonitorDrawHelper PodMonitorTable::getPodMonitorDrawHelper(const ActorPodMonitor &podMonitor)
{
    PodMonitorDrawHelper result;
    result.amProcMonitorList.reserve(MPK_MAX / 10);

    for(int amType = 0; const auto &monitor: podMonitor.amProcMonitorList){
        result.maxSendCount = std::max<size_t>(result.maxSendCount, monitor.sendCount);
        result.maxRecvCount = std::max<size_t>(result.maxRecvCount, monitor.recvCount);
        if(monitor.sendCount || monitor.recvCount){
            result.amProcMonitorList.push_back({amType, monitor});
        }
        amType++;
    }
    return result;
}
