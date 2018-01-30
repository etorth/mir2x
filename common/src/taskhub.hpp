/*
 * =====================================================================================
 *
 *       Filename: taskhub.hpp
 *        Created: 04/03/2016 22:14:46
 *    Description: this makes me very confused, std::function may use internally
 *                 dynamically allocated memory, if so, it's nonsense of using
 *                 the MemoryBlockPN to allocated std::function itself
 *
 *                 OK do as much as you can
 *                 I am not confident enough to update this class
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

#include <list>
#include <condition_variable>

#include "task.hpp"
#include "basehub.hpp"
#include "memoryblockpn.hpp"

using TaskBlockPN = MemoryBlockPN<sizeof(Task), 1024, 4>;
class TaskHub: public BaseHub
{
    protected:
        std::mutex              m_TaskLock;
        std::condition_variable m_TaskCV;
        std::list<Task*>        m_TaskList;
        TaskBlockPN             m_TaskBlockPN;

    public:
        TaskHub()
            : BaseHub()
            , m_TaskLock()
            , m_TaskCV()
            , m_TaskList()
            , m_TaskBlockPN()
        {}

        // 1. do shutdown manually
        // 2. call the destructor, I didn't call it inside
        virtual ~TaskHub() = default;
        
    protected:
        void Add(Task* pTask, bool bPushHead = false)
        {
            if(pTask){
                bool bNeedNotify = false;
                {
                    std::lock_guard<std::mutex> stLockGuard(m_TaskLock);
                    // still we are running
                    if(State()){
                        bNeedNotify = m_TaskList.empty();

                        if(bPushHead){
                            m_TaskList.push_front(pTask);
                        }else{
                            m_TaskList.push_back(pTask);
                        }
                    }else{
                        DeleteTask(pTask);
                        return;
                    }
                }

                if(bNeedNotify){ m_TaskCV.notify_one(); }
            }
        }

    public:
        // c++ can't support this signature for the function:
        //      template<typename... Args> void Add(Args &&... args, bool bPushHead = true)
        // the variadic template arugment pack should stay at the end
        // so we have to provide Add() and AddHead()

        template<typename... Args> void Add(Args &&... args)
        {
            Add(CreateTask(std::forward<Args>(args)...), false);
        }

        template<typename... Args> void AddHead(Args &&... args)
        {
            Add(CreateTask(std::forward<Args>(args)...), true);
        }

    protected:
        template<typename... Args> Task *CreateTask(Args &&... args)
        {
            if(auto pData = m_TaskBlockPN.Get()){
                // passing null argument to placement new is undefined behavior
                return new (pData) Task(std::forward<Args>(args)...);
            }
            return nullptr;
        }

        void DeleteTask(Task *pTask)
        {
            if(pTask){
                pTask->~Task();
                m_TaskBlockPN.Free(pTask);
            }
        }

    public:
        void Shutdown()
        {
            State(false);
            std::lock_guard<std::mutex> stLockGuard(m_TaskLock);

            for(auto pTask: m_TaskList){
                DeleteTask(pTask);
            }

            m_TaskList.clear();
            m_TaskCV.notify_one();
        }

    protected:
        void MainLoop()
        {
            // NOTE: second argument defer_lock is to prevent from immediate locking
            std::unique_lock<std::mutex> stTaskUniqueLock(m_TaskLock, std::defer_lock);

            while(State()){
                // check if there are tasks waiting
                stTaskUniqueLock.lock();

                if(m_TaskList.empty()){
                    //if the list is empty, then wait for signal
                    m_TaskCV.wait(stTaskUniqueLock);
                }

                // for spurious wake-up
                if(!m_TaskList.empty()){
                    auto pTask = m_TaskList.front();
                    m_TaskList.pop_front();
                    stTaskUniqueLock.unlock();

                    if(!pTask->Expired()){ (*pTask)(); }

                    DeleteTask(pTask);
                }else{
                    stTaskUniqueLock.unlock();
                }
            }
        }
};
