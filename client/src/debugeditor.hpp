/*
 * =====================================================================================
 *
 *       Filename: debugeditor.hpp
 *        Created: 03/28/2020 23:00:26
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
#include "widget.hpp"

class debugEditor: public Widget
{
    private:
        int m_lineSpace;

    private:
        int m_gridW;
        int m_gridH;

    public:
        debugEditor();

    public:
        std::vector<std::unique_ptr<lineRawMono>> m_lines;

    public:
        void setGridSize(int w, int h)
        {
            m_gridW = w;
            m_gridH = h;
        }
};
