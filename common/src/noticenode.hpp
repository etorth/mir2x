/*
 * =====================================================================================
 *
 *       Filename: noticenode.hpp
 *        Created: 04/06/2017 13:03:56
 *  Last Modified: 05/25/2017 16:36:58
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
    const int Notice;
    const int NoticeParam;

    const int X;
    const int Y;

    NoticeNode(int nNotice, int nNoticeParam, int nX, int nY)
        : Notice(nNotice)
        , NoticeParam(nNoticeParam)
        , X(nX)
        , Y(nY)
    {}
};
