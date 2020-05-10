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
        uint32_t                         m_lastEventID;
        std::mutex                       m_eventLock;
        std::condition_variable          m_eventCV;
        std::unordered_set<uint32_t>     m_eventIDRecord;
        std::function<void(EventTask *)> m_exec;
        EventTaskBlockPN                 m_eventTaskBlockPN;

    protected:
        std::priority_queue<EventTask*, std::deque<EventTask *>, TaskCycleComp> m_eventQ;

    public:
        EventTaskHub(const std::function<void(EventTask *)> &fnExec = [](EventTask *pTask){ if(pTask){ (*pTask)(); } })
            : BaseHub()
            , m_lastEventID(0)
            , m_eventLock()
            , m_eventCV()
            , m_eventIDRecord()
            , m_exec(fnExec)
            , m_eventTaskBlockPN()
        {}

        // 1. do shutdown manually
        // 2. call the destructor, I didn't call it inside
        virtual ~EventTaskHub() = default;

    public:
        bool Dismiss(uint32_t nID)
        {
            if(nID){
                std::lock_guard<std::mutex> stLockGuard(m_eventLock);
                auto pRecord = m_eventIDRecord.find(nID);
                if(pRecord != m_eventIDRecord.end()){
                    m_eventIDRecord.erase(pRecord);
                    return true;
                }
            }
            return false;
        }

    protected:
        EventTask *CreateEventTask(uint32_t nDelayMS, std::function<void()> &&fnOp)
        {
            if(auto pData = m_eventTaskBlockPN.Get()){
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
                m_eventTaskBlockPN.Free(pTask);
            }
        }

    public:
        // stop the hub and clean all un-invoked handlers
        // last function call before destruction, don't support restart
        void Shutdown()
        {
            State(false);
            {
                std::lock_guard<std::mutex> stLockGuard(m_eventLock);

                // 1. clean the priority queue of handlers
                while(!m_eventQ.empty()){
                    DeleteEventTask(m_eventQ.top());
                    m_eventQ.pop();
                }

                // 2. clean the ID record for all handlers
                m_eventIDRecord.clear();
            }
            m_eventCV.notify_one();
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

                m_eventLock.lock();

                if(State()){
                    // check if the event has a valid id
                    if(pTask->ID() == 0){
                        // if not generate one

                        // for most case m_eventIDRecord.size() == m_eventQ.size()
                        // but when trying to dismiss an event we only called m_eventIDRecord.erase()
                        // then during the executation of event we check if it's dismissed already
                        //
                        // so here we use (m_eventIDRecord.empty() && m_eventQ.empty())
                        // otherwise take the following case:
                        //
                        // 0. empty event queue
                        // 1. add(e0) -> get handler 1
                        // 2. dismiss(1)
                        // 3. add(e1) -> get handler 1
                        // 4. error
                        //
                        // drawback is if we push an event with a very large delay and dismissed it
                        // the m_lastEventID won't reset to 1 unless we reach that time

                        m_lastEventID = ((m_eventIDRecord.empty() && m_eventQ.empty()) ? 1 : (m_lastEventID + 1));

                        // overflowed, find a valid ID here by testing in a loop
                        if(!m_lastEventID){
                            for(uint32_t nID = 1; nID; ++nID){
                                if(m_eventIDRecord.find(nID) == m_eventIDRecord.end()){
                                    m_lastEventID = nID;
                                    break;
                                }
                            }

                            // still we can't get a valid ID, could merely happen
                            if(!m_lastEventID){
                                m_eventLock.unlock();
                                DeleteEventTask(pTask);
                                return 0;
                            }
                        }
                        pTask->ID(m_lastEventID);
                    }

                    // ok now pTask has a valid ID
                    // put the handler and its ID to record

                    nValidID = pTask->ID();
                    m_eventIDRecord.insert(nValidID);
                    m_eventQ.push(pTask);

                    // if the list was empty or this event is the top in the list
                    // means we need to take care of the newly added handler, we have to notify
                    bNotify = (pTask == m_eventQ.top());
                }else{
                    // the hub is temerminated or stopped
                    m_eventLock.unlock();
                    DeleteEventTask(pTask);
                    return 0;
                }

                m_eventLock.unlock();
                if(bNotify){ m_eventCV.notify_one(); }

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
            std::unique_lock<std::mutex> stUniqueLock(m_eventLock, std::defer_lock);
            while(State()){
                std::cv_status stRet = std::cv_status::no_timeout;
                stUniqueLock.lock();
                if(m_eventQ.empty()){
                    // wait until new handler added in
                    m_eventCV.wait(stUniqueLock);
                }else{
                    // or wait for a shorter time
                    stRet = m_eventCV.wait_until(stUniqueLock, m_eventQ.top()->Cycle());
                }

                // the mutex is locked again now...
                if(stRet == std::cv_status::timeout){
                    // ok we had a timeout
                    // so there has to be an event we *have to* execute
                    // because time is up!
                    auto pTask = m_eventQ.top();
                    m_eventQ.pop();

                    // check if the event was dismissed
                    auto pRecord = m_eventIDRecord.find(pTask->ID());
                    if(pRecord == m_eventIDRecord.end()){
                        stUniqueLock.unlock();
                        DeleteEventTask(pTask);
                        continue;
                    }
                    m_eventIDRecord.erase(pRecord);
                    stUniqueLock.unlock();

                    // it's time to execute it
                    if(m_exec){ m_exec(pTask); }
                }else{
                    stUniqueLock.unlock();
                }
            }
        }
};
