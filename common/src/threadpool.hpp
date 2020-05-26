/*
 * =====================================================================================
 *
 *       Filename: threadpool.hpp
 *        Created: 02/06/2016 13:43:29
 *    Description:
 *
 *                from: https://github.com/progschj/ThreadPool
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: g++ -std=c++14
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
#include <vector>
#include <memory>
#include <thread>
#include <cstddef>
#include <utility>
#include <stdexcept>
#include <functional>
#include <condition_variable>

// interface
// 1. ThreadPool(size_t threadNum)  // create pool with a specified threadNum
//                                  // if threadNum == 0 it uses std::thread::hardware_concurrency()
//
// 2. ThreadPool(initializer_list)  // create pool with size same as task count
//                                  // each task gets automatically added by addTask()
//
// 3. addTask(Callable && task)     // add a task, note:
//                                  // a. don't call addTask() after finish()
//                                  // b. don't call finish() inside the Callable
//
// 4. finish()                      // wait all added tasks done and join threads
//                                  // after this function the thread pool can NOT be reused, pool is dead
//
// 5. ~ThreadPool()                 // call finish() if not explicitly get called
//
// debug-interface
// 1. DISABLE_THREAD_POOL           // disable thread pool
//                                  // task get executed locally at addTask call

class ThreadPool 
{
    public:
        const size_t poolSize;
        const bool   useThreadPool;

    private:
        bool m_stop = false;

    private:
        std::vector<std::thread> m_workerList;
        std::queue<std::function<void(int)>> m_taskQueue;

    private:
        std::mutex m_lock;
        std::condition_variable m_condition;

    public:
        ThreadPool(size_t threadNum)
            : poolSize(getThreadCount(threadNum))
            , useThreadPool([]() -> bool
              {
                  const static bool useThreadPool = std::getenv("DISABLE_THREAD_POOL") ? false : true;
                  return useThreadPool;
              }())
        {
            if(!useThreadPool){
                return;
            }

            for(size_t i = 0; i < poolSize; ++i){
                m_workerList.emplace_back([this, i]()
                {
                    while(true){
                        std::function<void(int)> currTask;
                        {
                            std::unique_lock<std::mutex> lockGuard(m_lock);
                            m_condition.wait(lockGuard, [this]() -> bool
                            {
                                // let it wait if:
                                // 1. running
                                // 2. and no task in the queue
                                return m_stop || !m_taskQueue.empty();
                            });

                            if(m_stop && m_taskQueue.empty()){
                                return;
                            }

                            currTask = std::move(m_taskQueue.front());
                            m_taskQueue.pop();
                        }
                        currTask(i);
                    }
                });
            }
        }

    public:
        ThreadPool(std::initializer_list<std::function<void(int)>> taskList)
            : ThreadPool(taskList.size())
        {
            for(auto task: taskList){
                addTask(std::move(task));
            }
        }

    public:
        virtual ~ThreadPool()
        {
            try{
                finish();
            }
            catch(...){
                //
            }
        }

    public:
        template<typename Callable> void addTask(Callable && task)
        {
            if(!useThreadPool){
                if(m_stop){
                    throw std::runtime_error("adding new task to stopped pool");
                }
                task(0);
                return;
            }

            // in thread pool mode
            // m_stop need to be protected before access
            {
                std::unique_lock<std::mutex> lockGuard(m_lock);
                if(m_stop){
                    throw std::runtime_error("adding new task to stopped pool");
                }
                m_taskQueue.emplace(std::forward<Callable>(task));
            }
            m_condition.notify_one();
        }

    public:
        void finish()
        {
            if(!useThreadPool){
                m_stop = true;
                return;
            }

            // in thread pool mode
            // m_stop need to be protected before access
            {
                std::unique_lock<std::mutex> lockGuard(m_lock);
                if(m_stop){
                    return;
                }
                m_stop = true;
            }

            // notify all and join
            // this implementation doesn't take care of exception
            // TODO 1. should I use std::async
            //      2. or always capture exception inside Callable?

            m_condition.notify_all();
            for(auto &worker: m_workerList){
                worker.join();
            }
        }

    public:
        static size_t getThreadCount(size_t threadCount) 
        {
            return (threadCount > 0) ? threadCount : hwThreadCount();
        }

        static size_t limitedThreadCount(size_t limitedCount)
        {
            const auto hwThreadCnt = hwThreadCount();
            if(limitedCount > 0){
                return std::min<size_t>(limitedCount, hwThreadCnt);
            }
            return hwThreadCnt;
        }

    private:
        static size_t hwThreadCount()
        {
            const size_t hwThreadCnt = std::thread::hardware_concurrency();
            return hwThreadCnt > 0 ? hwThreadCnt : 16;
        }
};
