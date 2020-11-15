/*
 * =====================================================================================
 *
 *       Filename: fltableimpl.hpp
 *        Created: 12/04/2018 22:59:06
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
#include <Fl/Fl_Table_Row.H>
#include "strf.hpp"
#include "fflerror.hpp"

class Fl_TableImpl: public Fl_Table_Row
{
    private:
        int  m_sortByCol = -1;
        bool m_sortOrder =  false;

    public:
        Fl_TableImpl(int, int, int, int, const char *szLabel = nullptr);
        
    public:
        int getFontWidth() const;

    private:
        void drawData(int, int, int, int, int, int);
        void drawHeader(const char *, int, int, int, int);
        void draw_cell(TableContext, int, int, int, int, int, int) override;

    public:
        virtual std::string getGridData(int, int) const = 0;

    public:
        bool sortOrder() const
        {
            return m_sortOrder;
        }

        void setSortOrder(bool sortOrderArg)
        {
            m_sortOrder = sortOrderArg;
        }

        void flipSortOrder()
        {
            m_sortOrder = !m_sortOrder;
        }

    public:
        int sortByCol() const
        {
            return m_sortByCol;
        }

        void setSortByCol(int sortByOrderArg)
        {
            m_sortByCol = sortByOrderArg;
        }

    private:
        void onClick();

    public:
        virtual void sortTable() = 0;

    protected:
        virtual bool rowSelected(int) const = 0;

    protected:
        bool colSelected(int col) const
        {
            checkColEx(col);
            return col == sortByCol();
        }

    public:
        static std::string getTimeString(uint64_t msec)
        {
            const int h = (int)(msec / 3600000ULL); msec -= (h * 3600000ULL);
            const int m = (int)(msec /   60000ULL); msec -= (m *   60000ULL);
            const int s = (int)(msec /    1000ULL); msec -= (s *    1000ULL);

            char timeBuf[128];
            std::sprintf(timeBuf, "%dh:%02dm:%02ds:%03dms", h, m, s, (int)(msec));
            return timeBuf;
        }

    public:
        virtual void onClickOff() {}
        virtual void onClickCol(int) {}
        virtual void onClickCell(int, int) {}
        virtual void onDoubleClickCell(int, int) {}

    public:
        virtual std::string getColName(int) const = 0;

    public:
        virtual int getRowCount() const = 0;
        virtual int getColCount() const = 0;

    public:
        bool checkRow(int row) const
        {
            return (row >= 0 && row < getRowCount());
        }

        bool checkCol(int col) const
        {
            return (col >= 0 && col < getColCount());
        }

    public:
        void checkRowEx(int row) const
        {
            if(checkRow(row)){
                return;
            }
            throw fflerror("invalid row: %d", row);
        }

        void checkColEx(int col) const
        {
            if(checkCol(col)){
                return;
            }
            throw fflerror("invalid col: %d", col);
        }

    public:
        virtual void updateTable() = 0;
};
