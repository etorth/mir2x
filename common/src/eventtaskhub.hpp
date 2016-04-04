/*
 * =====================================================================================
 *
 *       Filename: eventtaskhub.hpp
 *        Created: 04/03/2016 22:55:21
 *  Last Modified: 04/04/2016 00:29:14
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

#include <mutex>
#include <queue>
#include <unordered_set>
#include <condition_variable>

#include "basehub.hpp"
#include "eventtask.hpp"

class EventTaskHub: public BaseHub<EventTaskHub>
{
    private:
        struct TaskComp
        {
            bool operator()(const EventTask* pLHS, const EventTask* pRHS) const
            {
                return pLHS->Cycle() > pRHS->Cycle();
            }
        };

    protected:
        uint32_t                         m_LastEventID;
        std::mutex                       m_EventLock;
        std::condition_variable          m_EventSignal;
        std::unordered_set<uint32_t>     m_EventIDV;
        std::function<void(EventTask *)> m_Exec;

    protected:
        std::priority_queue<EventTask*, std::deque<EventTask *>, TaskComp> m_EventList;

    public:
        EventTaskHub(std::function<void(EventTask *)>
                fnExec = [](EventTask *pTask){ if(pTask){ (*pTask)(); } })
            : BaseHub<EventTaskHub>()
            , m_LastEventID(0)
            , m_Exec(fnExec)
        {}

        virtual ~EventTaskHub() = default;

    public:
        bool Dismiss(uint32_t nID)
        {
            if(nID == 0){
                return false;
            }

            std::lock_guard<std::mutex> stLockGuard(m_EventLock);
            auto stInst = m_EventIDV.find(nID);
            if(stInst == m_EventIDV.end()){
                return false;
            }

            m_EventIDV.erase(stInst);
            return true;
        }

    public:
        void DeleteEventTask(EventTask *pTask)
        {
            delete pTask;
        }

    public:
        void Shutdown()
        {
            State(0);
            m_EventLock.lock();

            //this list should already be empty
            while(!m_EventList.empty()){
                DeleteEventTask(m_EventList.top());
                m_EventList.pop();
            }

            m_EventIDV.clear();
            m_EventLock.unlock();
            m_EventSignal.notify_one();
        }

    public:
        uint32_t Add(EventTask *pTask)
        {
            if(!pTask){return 0;}

            bool bDoSignal = false;
            m_EventLock.lock();

            if(State() == 2){
                // check if the event has a valid id
                if(pTask->ID() == 0){
                    // if not generate one
                    m_LastEventID++;
                    if(m_LastEventID == 0){
                        m_LastEventID = 1;
                    }

                    pTask->ID(m_LastEventID);
                }

                m_EventIDV.insert(pTask->ID());
                m_EventList.push(pTask);

                // if the list was empty or this event is the top in the list
                // we have to signal stInst
                bDoSignal = (pTask == m_EventList.top());
            }else{
                m_EventLock.unlock();
                DeleteEventTask(pTask);
                return 0;
            }

            m_EventLock.unlock();

            if (bDoSignal) {
                m_EventSignal.notify_one();
            }

            return pTask->ID();
        }

    public:
        void MainLoop()
        {
            std::unique_lock<std::mutex> stEventUniqueLock(m_EventLock, std::defer_lock);
            while(State()){
                std::cv_status stRet = std::cv_status::no_timeout;
                stEventUniqueLock.lock();
                if(m_EventList.empty()){
                    m_EventSignal.wait(stEventUniqueLock);
                }else{
                    // wait for a shorter time
                    stRet = m_EventSignal.wait_until(stEventUniqueLock, m_EventList.top()->Cycle());
                }

                // the mutex is locked again now...
                if(stRet == std::cv_status::timeout){
                    // ok we had a timeout
                    // so there has to be an event we *have to* execute
                    // because time is up!
                    auto pTask = m_EventList.top();
                    m_EventList.pop();

                    // check if the event was stopped
                    auto stInst = m_EventIDV.find(pTask->ID());
                    if(stInst == m_EventIDV.end()){
                        stEventUniqueLock.unlock();
                        DeleteEventTask(pTask);
                        continue;
                    }
                    m_EventIDV.erase(stInst);
                    stEventUniqueLock.unlock();

                    // it's time to execute it
                    // pTask->Expire(0);
                    // extern TaskHub *g_TaskHub;
                    // g_TaskHub->Add(pTask, true);
                    if(m_Exec){ m_Exec(pTask); }
                }else{
                    stEventUniqueLock.unlock();
                }
            }
        }
};
