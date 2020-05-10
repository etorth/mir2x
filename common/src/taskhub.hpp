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
        std::mutex              m_taskLock;
        std::condition_variable m_taskCV;
        std::list<Task*>        m_taskList;
        TaskBlockPN             m_taskBlockPN;

    public:
        TaskHub()
            : BaseHub()
            , m_taskLock()
            , m_taskCV()
            , m_taskList()
            , m_taskBlockPN()
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
                    std::lock_guard<std::mutex> stLockGuard(m_taskLock);
                    // still we are running
                    if(State()){
                        bNeedNotify = m_taskList.empty();

                        if(bPushHead){
                            m_taskList.push_front(pTask);
                        }else{
                            m_taskList.push_back(pTask);
                        }
                    }else{
                        DeleteTask(pTask);
                        return;
                    }
                }

                if(bNeedNotify){ m_taskCV.notify_one(); }
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
            if(auto pData = m_taskBlockPN.Get()){
                // passing null argument to placement new is undefined behavior
                return new (pData) Task(std::forward<Args>(args)...);
            }
            return nullptr;
        }

        void DeleteTask(Task *pTask)
        {
            if(pTask){
                pTask->~Task();
                m_taskBlockPN.Free(pTask);
            }
        }

    public:
        void Shutdown()
        {
            State(false);
            std::lock_guard<std::mutex> stLockGuard(m_taskLock);

            for(auto pTask: m_taskList){
                DeleteTask(pTask);
            }

            m_taskList.clear();
            m_taskCV.notify_one();
        }

    protected:
        void MainLoop()
        {
            // NOTE: second argument defer_lock is to prevent from immediate locking
            std::unique_lock<std::mutex> stTaskUniqueLock(m_taskLock, std::defer_lock);

            while(State()){
                // check if there are tasks waiting
                stTaskUniqueLock.lock();

                if(m_taskList.empty()){
                    //if the list is empty, then wait for signal
                    m_taskCV.wait(stTaskUniqueLock);
                }

                // for spurious wake-up
                if(!m_taskList.empty()){
                    auto pTask = m_taskList.front();
                    m_taskList.pop_front();
                    stTaskUniqueLock.unlock();

                    if(!pTask->Expired()){ (*pTask)(); }

                    DeleteTask(pTask);
                }else{
                    stTaskUniqueLock.unlock();
                }
            }
        }
};
