/*
 * =====================================================================================
 *
 *       Filename: eventtaskhub.hpp
 *        Created: 04/03/2016 22:55:21
 *  Last Modified: 05/24/2016 17:02:25
 *
 *    Description: this class support event executation after a delay
 *                 so don't think too much of performance
 *
 *                 this class support operations:
 *                 1. Launch()          : state the whole hub and make it's running
 *                 2. Dismiss(nID)      : cancel one handler when 
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
class EventTaskHub: public BaseHub<EventTaskHub>
{
    private:
        struct TaskComp
        {
            // won't check empty pointer
            bool operator()(const EventTask* pLHS, const EventTask* pRHS) const
            {
                return pLHS->Cycle() > pRHS->Cycle();
            }
        };

    protected:
        uint32_t                         m_LastEventID;
        std::mutex                       m_EventLock;
        std::condition_variable          m_EventCV;
        std::unordered_set<uint32_t>     m_EventIDV;
        std::function<void(EventTask *)> m_Exec;
        EventTaskBlockPN                 m_EventTaskBlockPN;

    protected:
        std::priority_queue<EventTask*, std::deque<EventTask *>, TaskComp> m_EventList;

    public:
        EventTaskHub(const std::function<void(EventTask *)>
                &fnExec = [](EventTask *pTask){ if(pTask){ (*pTask)(); } })
            : BaseHub<EventTaskHub>()
            , m_LastEventID(0)
            , m_Exec(fnExec)
        {}

        virtual ~EventTaskHub() = default;

    public:
        // try to cancel one operation before its invocation
        // here m_EventIDV is actually an unordered_set, if in demand of perfrmance
        // we can limit the number of handlers and put them in a real vector
        bool Dismiss(uint32_t nID)
        {
            if(nID == 0){ return false; }

            std::lock_guard<std::mutex> stLockGuard(m_EventLock);
            auto pRecord = m_EventIDV.find(nID);

            if(pRecord == m_EventIDV.end()){ return false; }

            m_EventIDV.erase(pRecord);
            return true;
        }

    public:
        EventTask *CreateEventTask(uint32_t nDelayMS, std::function<void()> && fnOp)
        {
            auto pData = m_EventTaskBlockPN.Get();
            if(!pData){ return nullptr; }

            return new (pData) EventTask(nDelayMS, std::move(fnOp));
        }

        EventTask *CreateEventTask(uint32_t nDelayMS, const std::function<void()> & fnOp)
        {
            return CreateEventTask(nDelayMS, std::function<void()>(fnOp));
        }

        void DeleteEventTask(EventTask *pTask)
        {
            if(!pTask){ return; }

            pTask->~EventTask();
            m_EventTaskBlockPN.Free(pTask);
        }

    public:
        // this function will clear the un-invoked handlers and stop the hub
        // by stopping the executing thread, previously I was thinking of adding
        // a parameter to decide if clear the handler list or not, but finally I
        // decide to always clear it
        void Shutdown()
        {
            // 1. set it as terminated
            State(false);

            // 2. clear all un-invoked handlers
            m_EventLock.lock();

            //this list should already be empty
            while(!m_EventList.empty()){
                DeleteEventTask(m_EventList.top());
                m_EventList.pop();
            }

            m_EventIDV.clear();
            m_EventLock.unlock();
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

        uint32_t Add(EventTask *pTask)
        {
            if(!pTask){return 0;}

            bool bDoSignal = false;
            uint32_t nValidID = 0;
            m_EventLock.lock();

            if(State()){
                // check if the event has a valid id
                if(pTask->ID() == 0){
                    // if not generate one
                    m_LastEventID = (m_EventIDV.empty() ? 1 : (m_LastEventID + 1));

                    // overflowed, find a valid ID here by testing in a loop
                    if(!m_LastEventID){
                        for(uint32_t nID = 1; nID; ++nID){
                            if(m_EventIDV.find(nID) == m_EventIDV.end()){
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

                nValidID = pTask->ID();
                m_EventIDV.insert(nValidID);
                m_EventList.push(pTask);

                // if the list was empty or this event is the top in the list
                // means we need to take care of the newly added handler, we have to notify
                bDoSignal = (pTask == m_EventList.top());
            }else{
                // the hub is temerminated or stopped
                m_EventLock.unlock();
                DeleteEventTask(pTask);
                return 0;
            }

            m_EventLock.unlock();
            if(bDoSignal){ m_EventCV.notify_one(); }

            // return pTask->ID();
            // this ``return pTask->ID()" I think it's a bug since after the ``unlock()"
            // there is no guarantee that the pointer pTask would stay valid
            return nValidID;
        }

    public:
        void MainLoop()
        {
            std::unique_lock<std::mutex> stUniqueLock(m_EventLock, std::defer_lock);
            while(State()){
                std::cv_status stRet = std::cv_status::no_timeout;
                stUniqueLock.lock();
                if(m_EventList.empty()){
                    // wait until new handler added in
                    m_EventCV.wait(stUniqueLock);
                }else{
                    // or wait for a shorter time
                    stRet = m_EventCV.wait_until(stUniqueLock, m_EventList.top()->Cycle());
                }

                // the mutex is locked again now...
                if(stRet == std::cv_status::timeout){
                    // ok we had a timeout
                    // so there has to be an event we *have to* execute
                    // because time is up!
                    auto pTask = m_EventList.top();
                    m_EventList.pop();

                    // check if the event was stopped
                    auto pRecord = m_EventIDV.find(pTask->ID());
                    if(pRecord == m_EventIDV.end()){
                        stUniqueLock.unlock();
                        DeleteEventTask(pTask);
                        continue;
                    }
                    m_EventIDV.erase(pRecord);
                    stUniqueLock.unlock();

                    // it's time to execute it
                    if(m_Exec){ m_Exec(pTask); }
                }else{
                    stUniqueLock.unlock();
                }
            }
        }
};
