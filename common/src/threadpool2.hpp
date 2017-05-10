/*
 * =====================================================================================
 *
 *       Filename: threadpool2.hpp
 *        Created: 02/06/2016 13:43:29
 *  Last Modified: 05/09/2017 12:56:06
 *
 *    Description: copy from https://github.com/progschj/ThreadPool, I editted it
 *                 to make it be simpler which is limited at
 *                      1. won't support arguments
 *                      2. won't support return value
 *                      3. no timeout support
 *
 *                 this pool doesn't has ``Launch()/Suspend()/Clear()" methods, it
 *                 start when creating it and stop when deleting it
 *
 *                 USAGE:
 *                      g_ThreadPool = new ThreadPool2();    // start the pool
 *                      g_ThreadPool->Add([](){ task(); });  // add new task
 *
 *                      delete g_ThreadPool;                 // stop the pool
 *
 *
 *                 TODO
 *                  current I have no plan to add ``Suspend()/Launch()" since it takes
 *                  too much work. Also I won't add ``Clear()", if need this, why not
 *                  just delete the pool and create a new one :)
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
#include <memory>
#include <thread>
#include <vector>
#include <stdexcept>
#include <functional>
#include <condition_variable>

#include "log.hpp"

class ThreadPool2
{
    private:
        bool                              m_Stop;
        std::mutex                        m_QueueMutex;
        std::vector<std::thread>          m_WorkThreadV;
        std::condition_variable           m_Condition;
        std::queue<std::function<void()>> m_TaskQ;

    public:
        // the constructor launches some amount of workers
        ThreadPool2(size_t nCount = 0)
            : m_Stop(false)
        {
            // TODO
            // try to guess a proper number of thread for the thread pool
            if(!nCount){ nCount = std::thread::hardware_concurrency(); }
            if(!nCount){ nCount = 4; }

            for(size_t nIndex = 0; nIndex < nCount; ++nIndex){
                m_WorkThreadV.emplace_back([this]()
                    {
                        while(true){
                            std::function<void()> fnTask;
                            {
                                std::unique_lock<std::mutex> stUniqueLock(m_QueueMutex);
                                // cv.wait(stLock, fnCheck()) is equivlent to 
                                //      while(!fnCheck()){
                                //          cv.wait(stLock);
                                //      }
                                // so here, if there are tasks in the queue, or m_Stop set as
                                // false in the dtor, cv.wait() won't take place, the while(true)
                                // loop will execute until there is no more tasks in the queue or
                                // m_Stop == true, then cv.wait() takes place
                                this->m_Condition.wait(stUniqueLock,
                                        [this](){ return (m_Stop || !m_TaskQ.empty()); });

                                // exit current thread i.i.f. there is no more tasks and
                                // current thread is required to exit, so it uses ``&&".
                                if(m_Stop && m_TaskQ.empty()){ return; };
                                fnTask = std::move(m_TaskQ.front());
                                m_TaskQ.pop();
                            }

                            // if we can be there, then fnTask must be executable
                            // so actually we don't need checking
                            if(fnTask){ fnTask(); }
                        }
                    });
            }
        }

        virtual ~ThreadPool2()
        {
            {
                std::unique_lock<std::mutex> stUniqueLock(m_QueueMutex);
                m_Stop = true;
            }
            m_Condition.notify_all();

            // before we exit all handler will take place, since the while(true)
            // loop only exit when m_Stop is false or queue is empty
            //
            // TODO
            // do we need a method to force to clear the current task queue?
            for(auto &stWorker: m_WorkThreadV){ stWorker.join(); }
        }

    public:
        // add a new task into the pool, return true if succeed
        bool Add(const std::function<void()> &fnOperate)
        {
            {
                std::unique_lock<std::mutex> stUniqueLock(m_QueueMutex);
                // TODO
                // only in the dtor we can set m_Stop = true
                // why the author put an m_Stop here? maybe the author put it to
                // enable the implementation of Suspend() which can suspend current 
                // pool without calling dtor ???
                //
                // can't put new tasks when pool stopped
                if(m_Stop){ return false; }

                // since we use const ref of fnOperate, so here it's the same for
                // std::queue::push() and std::queue::emplace()
                m_TaskQ.push(fnOperate);
            }
            // TODO understand notify_one() v.s. notify_all()
            //
            // here we only need to wake up one thread then it poll the task queue
            // and execute the task, then sleep again
            m_Condition.notify_one();
            return true;
        }
};
