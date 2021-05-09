/*
 * =====================================================================================
 *
 *       Filename: podmonitortable.hpp
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
#include "uidf.hpp"
#include "actorpool.hpp"
#include "raiitimer.hpp"
#include "fltableimpl.hpp"

class PodMonitorTable: public Fl_TableImpl
{
    private:
        struct PodMonitorDrawHelper
        {
            size_t maxSendCount   = 0;
            size_t maxRecvCount   = 0;
            size_t maxRecvAvgTime = 0;

            struct PodMonitorHelper
            {
                int amType;
                AMProcMonitor procMonitor;
            };
            std::vector<PodMonitorHelper> amProcMonitorList;
        };

    private:
        uint64_t m_podUID = 0;
        PodMonitorDrawHelper m_podDrawHelper;

    private:
        const std::vector<std::string> m_columnName;

    private:
        int  m_sortByCol;
        bool m_sortOrder;

    private:
        int m_selectedAMType = AM_NONE;

    public:
        PodMonitorTable(int argX, int argY, int argW, int argH, const char *labelCPtr = nullptr)
            : Fl_TableImpl(argX, argY, argW, argH, labelCPtr)
        {}

    private:
        bool rowSelected(int row) const
        {
            checkRowEx(row);
            return m_podDrawHelper.amProcMonitorList.at(row).amType == m_selectedAMType;
        }

    protected:
        void drawData(int, int, int, int, int, int);

    protected:
        void drawHeader(const char *, int, int, int, int);

    protected:
        std::string getGridData(int, int) const;

    private:
        static PodMonitorDrawHelper getPodMonitorDrawHelper(const ActorPodMonitor &);

    private:
        void sortTable();
        void setupColWidth();

    private:
        int selectAMTypeRow(int);

    public:
        void onClickOff() override
        {
            m_selectedAMType = AM_NONE;
        }

        void onClickCell(int row, int) override
        {
            checkRowEx(row);
            m_selectedAMType = m_podDrawHelper.amProcMonitorList.at(row).amType;
        }

    public:
        void updateTable() override;

    public:
        void setPodUID(uint64_t);

    public:
        int getRowCount() const override
        {
            return static_cast<int>(m_podDrawHelper.amProcMonitorList.size());
        }

        int getColCount() const override
        {
            return 5;
        }

        std::string getColName(int col) const override
        {
            switch(col){
                case 0: return "TYPE";
                case 1: return "BUSY";
                case 2: return "SEND";
                case 3: return "RECV";
                case 4: return "RECV_AVG";
                default: throw fflerror("invalid col: %d", col);
            }
        }
};
