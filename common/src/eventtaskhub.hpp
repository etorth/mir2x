/*
 * =====================================================================================
 *
 *       Filename: eventtaskhub.hpp
 *        Created: 04/03/2016 22:55:21
 *    Description: this class support event executation after a delay
 *                 so don't think too much of performance
 *
 *                 this class support operations:
 *                 1. Launch()          : state the whole hub and make it's running
 *                 2. Dismiss(nID)      : cancel one handler if possible
 *                 3. Shutdown(bClear)  : terminated the hub, clear all handlers
 *
 *                 I don't want to make a Suspend() / Restart() since doesn't make
 *                 sense
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

#include <mutex>
#include <queue>
#include <unordered_set>
#include <condition_variable>

#include "basehub.hpp"
#include "eventtask.hpp"
#include "memoryblockpn.hpp"

using EventTaskBlockPN = MemoryBlockPN<sizeof(EventTask), 1024, 4>;
class EventTaskHub: public BaseHub
{
    private:
        struct TaskCycleComp
        {
            bool operator()(const EventTask* pLHS, const EventTask* pRHS) const
            {
                return (pLHS && pRHS) ? (pLHS->Cycle() > pRHS->Cycle()) : true;
            }
        };

    protected:
        uint32_t                         m_LastEventID;
        std::mutex                       m_EventLock;
        std::condition_variable          m_EventCV;
        std::unordered_set<uint32_t>     m_EventIDRecord;
        std::function<void(EventTask *)> m_Exec;
        EventTaskBlockPN                 m_EventTaskBlockPN;

    protected:
        std::priority_queue<EventTask*, std::deque<EventTask *>, TaskCycleComp> m_EventQ;

    public:
        EventTaskHub(const std::function<void(EventTask *)> &fnExec = [](EventTask *pTask){ if(pTask){ (*pTask)(); } })
            : BaseHub()
            , m_LastEventID(0)
            , m_EventLock()
            , m_EventCV()
            , m_EventIDRecord()
            , m_Exec(fnExec)
            , m_EventTaskBlockPN()
        {}

        // 1. do shutdown manually
        // 2. call the destructor, I didn't call it inside
        virtual ~EventTaskHub() = default;

    public:
        bool Dismiss(uint32_t nID)
        {
            if(nID){
                std::lock_guard<std::mutex> stLockGuard(m_EventLock);
                auto pRecord = m_EventIDRecord.find(nID);
                if(pRecord != m_EventIDRecord.end()){
                    m_EventIDRecord.erase(pRecord);
                    return true;
                }
            }
            return false;
        }

    protected:
        EventTask *CreateEventTask(uint32_t nDelayMS, std::function<void()> &&fnOp)
        {
            if(auto pData = m_EventTaskBlockPN.Get()){
                return new (pData) EventTask(nDelayMS, std::move(fnOp));
            }
            return nullptr;
        }

        EventTask *CreateEventTask(uint32_t nDelayMS, const std::function<void()> &fnOp)
        {
            return CreateEventTask(nDelayMS, std::function<void()>(fnOp));
        }

        void DeleteEventTask(EventTask *pTask)
        {
            if(pTask){
                pTask->~EventTask();
                m_EventTaskBlockPN.Free(pTask);
            }
        }

    public:
        // stop the hub and clean all un-invoked handlers
        // last function call before destruction, don't support restart
        void Shutdown()
        {
            State(false);
            {
                std::lock_guard<std::mutex> stLockGuard(m_EventLock);

                // 1. clean the priority queue of handlers
                while(!m_EventQ.empty()){
                    DeleteEventTask(m_EventQ.top());
                    m_EventQ.pop();
                }

                // 2. clean the ID record for all handlers
                m_EventIDRecord.clear();
            }
            m_EventCV.notify_one();
        }

    public:
        uint32_t Add(uint32_t nDelayMS, std::function<void()> &&fnOp)
        {
            return Add(CreateEventTask(nDelayMS, std::move(fnOp)));
        }

        uint32_t Add(uint32_t nDelayMS, const std::function<void()> &fnOp)
        {
            return Add(nDelayMS, std::function<void()>(fnOp));
        }

    protected:
        uint32_t Add(EventTask *pTask)
        {
            if(pTask){
                bool     bNotify  = false;
                uint32_t nValidID = 0;

                m_EventLock.lock();

                if(State()){
                    // check if the event has a valid id
                    if(pTask->ID() == 0){
                        // if not generate one

                        // for most case m_EventIDRecord.size() == m_EventQ.size()
                        // but when trying to dismiss an event we only called m_EventIDRecord.erase()
                        // then during the executation of event we check if it's dismissed already
                        //
                        // so here we use (m_EventIDRecord.empty() && m_EventQ.empty())
                        // otherwise take the following case:
                        //
                        // 0. empty event queue
                        // 1. add(e0) -> get handler 1
                        // 2. dismiss(1)
                        // 3. add(e1) -> get handler 1
                        // 4. error
                        //
                        // drawback is if we push an event with a very large delay and dismissed it
                        // the m_LastEventID won't reset to 1 unless we reach that time

                        m_LastEventID = ((m_EventIDRecord.empty() && m_EventQ.empty()) ? 1 : (m_LastEventID + 1));

                        // overflowed, find a valid ID here by testing in a loop
                        if(!m_LastEventID){
                            for(uint32_t nID = 1; nID; ++nID){
                                if(m_EventIDRecord.find(nID) == m_EventIDRecord.end()){
                                    m_LastEventID = nID;
                                    break;
                                }
                            }

                            // still we can't get a valid ID, could merely happen
                            if(!m_LastEventID){
                                m_EventLock.unlock();
                                DeleteEventTask(pTask);
                                return 0;
                            }
                        }
                        pTask->ID(m_LastEventID);
                    }

                    // ok now pTask has a valid ID
                    // put the handler and its ID to record

                    nValidID = pTask->ID();
                    m_EventIDRecord.insert(nValidID);
                    m_EventQ.push(pTask);

                    // if the list was empty or this event is the top in the list
                    // means we need to take care of the newly added handler, we have to notify
                    bNotify = (pTask == m_EventQ.top());
                }else{
                    // the hub is temerminated or stopped
                    m_EventLock.unlock();
                    DeleteEventTask(pTask);
                    return 0;
                }

                m_EventLock.unlock();
                if(bNotify){ m_EventCV.notify_one(); }

                // return pTask->ID();
                // this ``return pTask->ID()" I think it's a bug since after the ``unlock()"
                // there is no guarantee that the pointer pTask would stay valid
                return nValidID;
            }
            return 0;
        }

    protected:
        void MainLoop()
        {
            std::unique_lock<std::mutex> stUniqueLock(m_EventLock, std::defer_lock);
            while(State()){
                std::cv_status stRet = std::cv_status::no_timeout;
                stUniqueLock.lock();
                if(m_EventQ.empty()){
                    // wait until new handler added in
                    m_EventCV.wait(stUniqueLock);
                }else{
                    // or wait for a shorter time
                    stRet = m_EventCV.wait_until(stUniqueLock, m_EventQ.top()->Cycle());
                }

                // the mutex is locked again now...
                if(stRet == std::cv_status::timeout){
                    // ok we had a timeout
                    // so there has to be an event we *have to* execute
                    // because time is up!
                    auto pTask = m_EventQ.top();
                    m_EventQ.pop();

                    // check if the event was dismissed
                    auto pRecord = m_EventIDRecord.find(pTask->ID());
                    if(pRecord == m_EventIDRecord.end()){
                        stUniqueLock.unlock();
                        DeleteEventTask(pTask);
                        continue;
                    }
                    m_EventIDRecord.erase(pRecord);
                    stUniqueLock.unlock();

                    // it's time to execute it
                    if(m_Exec){ m_Exec(pTask); }
                }else{
                    stUniqueLock.unlock();
                }
            }
        }
};
