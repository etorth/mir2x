/*
 * =====================================================================================
 *
 *       Filename: actormonitor.hpp
 *        Created: 09/02/2018 18:20:15
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
#include "actormessage.hpp"

struct ActorMonitor
{
    uint64_t uid = 0;

    uint32_t liveTick = 0;
    uint32_t busyTick = 0;

    uint32_t messageDone    = 0;
    uint32_t messagePending = 0;

    operator bool () const
    {
        return uid != 0;
    }
};

struct ActorThreadMonitor
{
    int      threadId;
    uint64_t actorCount;

    uint32_t liveTick;
    uint32_t busyTick;
};

struct AMProcMonitor
{
    uint64_t procTick  = 0;
    uint32_t sendCount = 0;
    uint32_t recvCount = 0;
};

struct TriggerMonitor
{
    uint64_t procTick = 0;
};

struct ActorPodMonitor
{
    uint64_t uid = 0;
    TriggerMonitor triggerMonitor;
    std::array<AMProcMonitor, MPK_MAX> amProcMonitorList;

    operator bool () const
    {
        return uid != 0;
    }
};
