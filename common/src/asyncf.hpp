/*
 * =====================================================================================
 *
 *       Filename: asyncf.hpp
 *        Created: 11/05/20 20:46:48
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

#include <queue>
#include <thread>
#include <chrono>
#include <condition_variable>
#include "fflerror.hpp"
#include "parallel_hashmap/phmap.h"

namespace asyncf
{
    template<typename L> class tryLockGuard
    {
        private:
            L &m_lockRef;
            const bool m_locked;

        public:
            explicit tryLockGuard(L &lock)
                : m_lockRef(lock)
                , m_locked(lock.try_lock())
            {}

            ~tryLockGuard()
            {
                if(m_locked){
                    m_lockRef.unlock();
                }
            }

            operator bool () const
            {
                return m_locked;
            }

        private:
            tryLockGuard(const tryLockGuard &) = delete;
            tryLockGuard & operator = (const tryLockGuard &) = delete;
    };

    enum
    {
        E_DONE    = 0,  //
        E_QCLOSED = 1,  // queue is closed
        E_TIMEOUT = 2,  // wait timeout
    };

    template<typename T, typename Q> class taskQ
    {
        private:
            Q m_taskQ;
            bool m_closed = false;

        private:
            mutable std::mutex m_lock;
            mutable std::condition_variable m_cond;

        public:
            taskQ() = default;

        public:
            bool try_push(T task)
            {
                bool added = false;
                {
                    asyncf::tryLockGuard<decltype(m_lock)> lockGuard(m_lock);
                    if(!lockGuard){
                        return false;
                    }
                    added = m_taskQ.push(std::move(task));
                }

                if(added){
                    m_cond.notify_one();
                }
                return true;
            }

            bool try_pop(T &task)
            {
                asyncf::tryLockGuard<decltype(m_lock)> lockGuard(m_lock);
                if(!lockGuard || m_taskQ.empty()){
                    return false;
                }

                task = std::move(m_taskQ.pick_top());
                return true;
            }

        public:
            void push(T task)
            {
                bool added = false;
                {
                    std::lock_guard<decltype(m_lock)> lockGuard(m_lock);
                    added = m_taskQ.push(std::move(task));
                }

                if(added){
                    m_cond.notify_one();
                }
            }

            void pop(T &task, uint64_t msec, int &ec)
            {
                std::unique_lock<decltype(m_lock)> lockGuard(m_lock);
                if(msec > 0){
                    const bool wait_res = m_cond.wait_for(lockGuard, std::chrono::milliseconds(msec), [this]() -> bool
                    {
                        return m_closed || !m_taskQ.empty();
                    });

                    if(wait_res){
                        // pred returns true
                        // means either not expired, or even expired but the pred evals to true now

                        // when queue is closed AND there are still tasks in m_taskQ
                        // what I should do ???

                        // currently I returns the task pending in the m_taskQ
                        // so a taskQ can be closed but you can still pop task from it

                        if(!m_taskQ.empty()){
                            task = std::move(m_taskQ.pick_top());
                            ec = asyncf::E_DONE;
                        }
                        else if(m_closed){
                            ec = asyncf::E_QCLOSED;
                        }
                        else{
                            // taskQ is not closed and m_taskQ is empty
                            // then pred evals to true, can only be time expired
                            ec = asyncf::E_TIMEOUT;
                        }
                    }
                    else{
                        // by https://en.cppreference.com/w/cpp/thread/condition_variable_any/wait_for
                        // when wait_res returns false:
                        // 1. the time has been expired
                        // 2. the pred still returns false, means:
                        //      1. queue is not closed, and
                        //      2. m_taskQ is still empty
                        ec = asyncf::E_TIMEOUT;
                    }
                }
                else{
                    m_cond.wait(lockGuard, [this]() -> bool
                    {
                        return m_closed || !m_taskQ.empty();
                    });

                    // when there is task in m_taskQ
                    // we always firstly pick & return the task before report E_CLOSED

                    if(!m_taskQ.empty()){
                        ec = asyncf::E_DONE;
                        task = std::move(m_taskQ.pick_top());
                    }
                    else{
                        ec = asyncf::E_QCLOSED;
                    }
                }
            }

            bool pop(T &task)
            {
                int ec = 0;
                pop(task, 0, ec);
                return ec == asyncf::E_DONE;
            }

            bool pop_ready(T &task)
            {
                std::lock_guard<decltype(m_lock)> lockGuard(m_lock);
                if(m_closed || m_taskQ.empty()){
                    return false;
                }

                task = std::move(m_taskQ.pick_top());
                return true;
            }

            void close()
            {
                {
                    std::lock_guard<decltype(m_lock)> lockGuard(m_lock);
                    m_closed = true;
                }
                m_cond.notify_all();
            }

            size_t size_hint() const
            {
                std::lock_guard<decltype(m_lock)> lockGuard(m_lock);
                return m_taskQ.size();
            }
    };
}
