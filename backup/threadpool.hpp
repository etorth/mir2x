/*
 * =====================================================================================
 *
 *       Filename: threadpool.hpp
 *        Created: 02/06/2016 13:43:29
 *  Last Modified: 04/04/2016 16:07:00
 *
 *    Description: copy from https://github.com/progschj/ThreadPool
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
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

class ThreadPool
{
    private:
        bool                              m_Stop;
        std::mutex                        m_QueueMutex;
        std::vector<std::thread>          m_WorkThread;
        std::condition_variable           m_Condition;
        std::queue<std::function<void()>> m_TaskV;

    public:
        // the constructor launches some amount of workers
        ThreadPool::ThreadPool(size_t nCount)
            : m_Stop(false)
        {
            // TODO
            if(!nCount){ nCount = std::thread::hardware_concurrency(); }
            if(!nCount){ nCount = 4; }

            for(size_t nIndex = 0; nIndex < nCount; ++nIndex){
                m_WorkThread.emplace_back([this]()
                        {
                            while(true){
                                std::function<void()> fnTask;
                                {
                                    std::unique_lock<std::mutex> stUniqueLock(this->m_QueueMutex);
                                    this->m_Condition.wait(stUniqueLock,
                                        [this]{ return (this->m_Stop || !this->m_TaskV.empty()); });

                                    if(this->m_Stop && this->m_TaskV.empty()){ return; };
                                    fnTask = std::move(this->m_TaskV.front());
                                    this->m_TaskV.pop();
                                }
                                fnTask();
                            }
                        });
            }
        }

    public:
        ThreadPool::~ThreadPool()
        {
            {
                std::unique_lock<std::mutex> stUniqueLock(m_QueueMutex);
                m_Stop = true;
            }
            m_Condition.notify_all();
            for(std::thread &stWorker: m_WorkThread){
                stWorker.join();
            }
        }

    public:
        // add a new task into the pool, return std::future
        template<class Func, class... Args>
            auto Add(Func&& fnOp, Args&&... stArgs)
            -> std::future<typename std::result_of<Func(Args...)>::type>
        {
            using ReturnType = typename std::result_of<Func(Args...)>::type;

            auto pTask = std::make_shared<std::packaged_task<ReturnType()>>(
                    std::bind(std::forward<Func>(fnOp), std::forward<Args>(stArgs)...));

            std::future<ReturnType> stRes = task->get_future();
            {
                std::unique_lock<std::mutex> stUniqueLock(m_QueueMutex);
                // don't allow enqueueing after stopping the pool
                if(m_Stop){
                    throw std::runtime_error("enqueue on stopped ThreadPool");
                }

                m_TaskV.emplace([pTask](){ (*pTask)(); });
            }
            m_Condition.notify_one();
            return stRes;
        }
};
