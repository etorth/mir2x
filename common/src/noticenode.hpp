/*
 * =====================================================================================
 *
 *       Filename: noticenode.hpp
 *        Created: 04/06/2017 13:03:56
 *  Last Modified: 05/15/2017 14:34:47
 *
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

#include <cstdint>
#include <cassert>
#include "protocoldef.hpp"

struct NoticeNode
{
    int Notice;
    int NoticeParam;

    int X;
    int Y;

    NoticeNode(int nNotice, int nNoticeParam, int nX, int nY)
        : Notice(nNotice)
        , NoticeParam(nNoticeParam)
        , X(nX)
        , Y(nY)
    {}

    void Print() const;
};
