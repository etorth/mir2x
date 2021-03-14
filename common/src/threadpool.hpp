/*
 * =====================================================================================
 *
 *       Filename: threadpool.hpp
 *        Created: 01/23/2019 13:46:24
 *    Description:
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: g++ -std=c++14
 *
 *         Author: ANHONG
 *          Email:
 *   Organization:
 *
 * =====================================================================================
 */

#pragma once
#include <mutex>
#include <queue>
#include <memory>
#include <atomic>
#include <string>
#include <cstring>
#include <vector>
#include <thread>
#include <future>
#include <cstddef>
#include <utility>
#include <stdexcept>
#include <exception>
#include <functional>
#include <type_traits>
#include <condition_variable>

// INTERFACE:
// const int threadPool::poolSize
//                                      // how many threads used in the pool, zero if pool is disabled
//
// const bool threadPool::disablePool
//                                      // true if pool is disabled
//                                      // when disabled all added task to the pool get ran in threadPool::addTask()
//
// size_t threadPool::hwThreadCount()
//                                      // thread count helper function
//                                      // returns std::hardware_concurrency() if it's non-zero, otherwise returns 16
//
// size_t threadPool::limitedThreadCount(size_t limitedNumThread)
//                                      // thread count helper function
//                                      // returns std::min<size_t>(limitedNumThread, threadPool::hwThreadCount()) if limitedNumThread is not zero, otherwise return threadPool::hwThreadCount()
//
// threadPool(size_t numThread, bool disable = false)
//                                      // create pool with a specified numThread, uses threadPool::hwThreadCount() if numThread == 0
//                                      // example:
//                                      //
//                                      //     threadPool pool(12, std::getenv("DISABLE_POOL"));
//                                      //     pool.addTask(...);
//                                      //
//                                      // notes:
//                                      // 1. specify numThread = 1 to force all posted tasks ranning sequentially in the single pool thread, won't block the calling thread
//                                      // 2. if disabled, all tasks sequentially get executed in addTask(), this blocks the calling thread
//
// threadPool(std::initializer_list<std::function<void(int)>, bool disable = false)
// threadPool(threadPool::abortedTag &hasError, std::initializer_list<std::function<void(int)>, bool disable = false)
//                                      // create pool with size same as list size, each task gets automatically added by threadPool::addTask(), disable is as above
//                                      // example:
//                                      //
//                                      //     threadPool
//                                      //     {
//                                      //         // list all tasks to get ran in parallel
//                                      //         // each task get 1 thread
//                                      //         {
//                                      //             [](int) { call_task1(); },
//                                      //             [](int) { call_task2(); },
//                                      //             [](int) { call_task3(); },
//                                      //             [](int) { call_task4(); },
//                                      //         },
//                                      // 
//                                      //         std::getenv("DISABLE_POOL"))
//                                      //     };
//                                      //
//
// void threadPool::addTask(Callable && task)
// void threadPool::addTask(threadPool::abortedTag &, Callable && task)
//                                      // add a task to the pool to execute, task get blocking-ran in addTask() if pool is disabled
//                                      // the signature of the task added should be equivalent to the following:
//                                      //
//                                      //     void task(int threadId);
//                                      //
//                                      // all added task will get an internal copy by copy/move-constructor if using thread pool
//                                      // example:
//                                      //
//                                      //     threadPool pool(12, std::getenv("DISABLE_POOL"));
//                                      //     threadPool::abortedTag hasError;
//                                      //
//                                      //     pool.addTask([](int){ call_non_throw_task(); });
//                                      //     pool.addTask(hasError, [](int){ call_task1(); });
//                                      //     pool.addTask(hasError, [](int){ call_task2(); });
//                                      //
//                                      //     pool.finish();
//                                      //     hasError.checkError();
//                                      //
//                                      // notes:
//                                      // 1. don't call addTask() after threadPool::finish()
//                                      // 2. don't call threadPool::finish() inside the Callable
//
// void threadPool::addForTask(size_t begin, size_t end, ForCallable && task)
// void threadPool::addForTask(threadPool::abortedTag &, size_t begin, size_t end, ForCallable && task)
//                                      // call task(threadId, i) with i in [begin, end) in parallel
//                                      // the signature of the task added should be equivalent to the following:
//                                      //
//                                      //     void task(int threadId, size_t index);
//                                      //
//                                      // task will have an unique internal copy, all i in [begin, end) get called based on this copy
//                                      // example:
//                                      //
//                                      //     struct call_counter
//                                      //     {
//                                      //         std::atomic<int> count = 0;
//                                      //         void operator () (int, size_t)
//                                      //         {
//                                      //             count++;
//                                      //         }
//                                      //     
//                                      //         ~call_counter()
//                                      //         {
//                                      //             std::printf("%p get called %d times.\n", this, count.load());
//                                      //         }
//                                      //     };
//                                      //
//                                      //     pool.addTask(10, 299, call_counter());
//                                      //
//
// void threadPool::finish() noexcept
//                                      // join and release all threads in the pool, when returns all added tasks should be done
//                                      // after this function the thread pool can NOT be reused, pool is dead
//                                      // notes:
//                                      // 1. this function can be called multiple times, only the first call takes effect
//                                      // 2. this function get called automatically in threadPool destructor
//
// ~threadPool()
//                                      // call finish() if not explicitly get called

// INTERFACE
// env: GLOBAL_THREAD_POOL_SIZE
//                                      // define the number of threads in the internal global pool
//                                      // if not set then uses threadPool::hwThreadCount()
//
// env: GLOBAL_THREAD_POOL_DISABLE
//                                      // disable the global pool
//                                      // all posted task get blocking-executed at post time
//
// std::future<T> globalThreadPool::postEvalTask(EvalCallable && evalTask)
//                                      // post an eval-task to the global pool and return a std::future<T>, evalTask get an internal copy
//                                      // the signature of evalTask posted should be equivalent to the following:
//                                      //
//                                      //     T evalTask(int threadId);
//                                      //
//                                      // evalTask only accepts an integral parameter to pass the thread id
//                                      // any extra prameters should be passed as lambda captures
//                                      // example:
//                                      //
//                                      //     const int parm = 128;
//                                      //     auto f = globalThreadPool::postEvalTask([parm](int threadId) -> std::string
//                                      //     {
//                                      //         if(parm < 0){
//                                      //             throw std::runtime_error(std::string("invalid parm = ") + std::to_string(parm));
//                                      //         }
//                                      //
//                                      //         int sum = 0;
//                                      //         for(int i = 0; i < parm; ++i){
//                                      //             sum += i;
//                                      //         }
//                                      //         return std::string("threadId ") + std::to_string(threadId) + "gets sum: " + std::to_string(sum);
//                                      //     });
//                                      //
//                                      //     // some other logic
//                                      //     // ...
//                                      //
//                                      //     std::cout<< f.get() << std::endl;
//                                      //
//                                      // notes:
//                                      // 1. all exceptions will be forward to thread calling std::future<T>::get()
//                                      // 2. if discard the returned std::future<T>, the postEvalTask() won't block, this is not like std::async
//
// globalThreadPool::parallelFor(size_t begin, size_t end, const ForCallable & forTask)
//                                      // if the global pool is not disabled, splits the given range [begin, end) evenly into N groups, where N = globalThreadPool::getGlobalPool().poolSize
//                                      // post each group by globalThreadPool::postEvalTask(), it blocks till all N groups finishes or throws
//                                      // example:
//                                      //
//                                      //     globalThreadPool::parallelFor(102, 299, [](int threadId, size_t i)
//                                      //     {
//                                      //         std::printf("thread id = %d, i = %zu\n", threadId, i);
//                                      //         if(i % 128 == 0){
//                                      //             throw std::runtime_error(std::string("throw in thread id = ") + std::to_string(threadId));
//                                      //         }
//                                      //     });
//                                      //
//                                      // notes:
//                                      // 1. it's blocking all till calls on [begin, end) finishes
//                                      // 2. all calls access the forTask const-ref, there is no copy of it
//                                      // 3. all exception thrown by forTask will be forward to calling thread automatically
//

class threadPool
{
    public:
        class abortedTag final
        {
            private:
                friend class threadPool;

            private:
                std::exception_ptr m_except;

            private:
                std::atomic<bool> m_tag {false};

            public:
                void checkError() const
                {
                    if(!m_tag.load()){
                        return;
                    }

                    if(!m_except){
                        throw std::runtime_error("threadPool::abortedTag: missing exception");
                    }
                    std::rethrow_exception(m_except);
                }
        };

    public:
        const size_t poolSize;
        const bool   disablePool;

    private:
        bool m_stop = false;

    private:
        std::vector<std::thread> m_workers;
        std::queue<std::function<void(int)>> m_taskQ;

    private:
        std::mutex m_lock;
        std::condition_variable m_condition;

    public:
        threadPool(size_t numThread, bool disable = false)
            : poolSize([numThread, disable]() -> size_t
              {
                  if(disable){
                      return 0;
                  }

                  if(numThread > 0){
                      return numThread;
                  }
                  return threadPool::hwThreadCount();
              }())
            , disablePool(disable)
        {
            if(disablePool){
                return;
            }

            m_workers.reserve(poolSize);
            for(size_t threadId = 0; threadId < poolSize; ++threadId){
                m_workers.emplace_back([this, threadId]()
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
                                return m_stop || !m_taskQ.empty();
                            });

                            if(m_stop && m_taskQ.empty()){
                                return;
                            }

                            currTask = std::move(m_taskQ.front());
                            m_taskQ.pop();
                        }

                        // no exception handling
                        // use threadCBWrapper() if task may throw, otherwise the exception breaks the pool
                        currTask((int)(threadId));
                    }
                });
            }
        }

    public:
        threadPool(std::initializer_list<std::function<void(int)>> taskList, bool disable = false)
            : threadPool(taskList.size(), disable)
        {
            for(auto task: taskList){
                addTask(std::move(task));
            }
        }

        threadPool(threadPool::abortedTag &hasError, std::initializer_list<std::function<void(int)>> taskList, bool disable = false)
            : threadPool(taskList.size(), disable)
        {
            for(auto task: taskList){
                addTask(hasError, std::move(task));
            }
        }

    public:
        virtual ~threadPool()
        {
            finish();
        }

    public:
        template<typename Callable> void addTask(Callable && task)
        {
            if(disablePool){
                if(m_stop){
                    throw std::runtime_error("adding task to stopped pool");
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
                m_taskQ.emplace(std::forward<Callable>(task));
            }
            m_condition.notify_one();
        }

        template<typename Callable> void addTask(threadPool::abortedTag &hasError, Callable && task)
        {
            addTask(threadPool::threadCBWrapper(hasError, std::forward<Callable>(task)));
        }

    private:
        template<typename ForCallable> void addForTaskHelper(threadPool::abortedTag *hasErrorPtr, size_t beginIndex, size_t endIndex, ForCallable && forTask)
        {
            if(beginIndex >= endIndex){
                return;
            }

            // best effort to make all calls accessing same functor
            // otherwise if use capture by value then different group accesses different functor

            const size_t groupSize = (endIndex - beginIndex + poolSize - 1) / poolSize;
            const auto taskObjPtr = std::make_shared<std::function<void(int, size_t)>>(std::forward<ForCallable>(forTask));

            for(size_t group = 0; group < poolSize; ++group){
                const size_t groupBegin = beginIndex + group * groupSize;
                const size_t groupEnd = std::min<size_t>(groupBegin + groupSize, endIndex);

                if(groupBegin >= groupEnd){
                    break;
                }

                const auto fnLoop = [groupBegin, groupEnd, taskObjPtr](int threadId)
                {
                    for(size_t i = groupBegin; i < groupEnd; ++i){
                        (*taskObjPtr)(threadId, i);
                    }
                };

                if(hasErrorPtr){
                    addTask(*hasErrorPtr, fnLoop);
                }
                else{
                    addTask(fnLoop);
                }
            }
        }

    public:
        template<typename ForCallable> void addForTask(size_t beginIndex, size_t endIndex, ForCallable && forTask)
        {
            addForTaskHelper(nullptr, beginIndex, endIndex, std::forward<ForCallable>(forTask));
        }

        template<typename ForCallable> void addForTask(threadPool::abortedTag &hasError, size_t beginIndex, size_t endIndex, ForCallable && forTask)
        {
            addForTaskHelper(&hasError, beginIndex, endIndex, std::forward<ForCallable>(forTask));
        }

    public:
        void finish() noexcept
        {
            if(disablePool){
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

            m_condition.notify_all();
            for(auto &worker: m_workers){
                worker.join();
            }

            // release thread objects back to system
            // after threadPool::finish() the pool is dead, can't restart it
            std::vector<std::thread>().swap(m_workers);
        }

    public:
        static size_t limitedThreadCount(size_t limitedNumThread)
        {
            const auto hwThreadCnt = hwThreadCount();
            if(limitedNumThread > 0){
                return std::min<size_t>(limitedNumThread, hwThreadCnt);
            }
            return hwThreadCnt;
        }

        static size_t hwThreadCount()
        {
            const size_t hwThreadCnt = std::thread::hardware_concurrency();
            return hwThreadCnt > 0 ? hwThreadCnt : 16;
        }

    public:
        // helper wrapper to handle exception in threadPool
        // example:
        //
        //     threadPool::abortedTag hasError;
        //
        //     pool.addTask(threadPool::threadCBWrapper(hasError, [](int){ /* code */ }));
        //     pool.addTask(threadPool::threadCBWrapper(hasError, [](int){ /* code */ }));
        //     pool.addTask(threadPool::threadCBWrapper(hasError, [](int){ /* code */ }));
        //
        //     pool.finish();
        //     hasError.checkError();
        //
        template<typename Callable> static auto threadCBWrapper(threadPool::abortedTag &hasError, Callable && task)
        {
            return [&hasError, taskCpy = std::forward<Callable>(task)](int threadId)
            {
                if(hasError.m_tag.load()){
                    return;
                }

                try{
                    taskCpy(threadId);
                }
                catch(...){
                    bool expected = false;
                    if(hasError.m_tag.compare_exchange_strong(expected, true)){
                        hasError.m_except = std::current_exception();
                    }
                    else{
                        // only capture and forward the first exception
                        // ignore all the rest
                    }
                }
            };
        }
};

class globalThreadPool final
{
    private:
        globalThreadPool() = delete;

    private:
        static threadPool &getGlobalPool()
        {
            static threadPool globalPool([]() -> int
            {
                if(const auto p = std::getenv("GLOBAL_THREAD_POOL_SIZE")){
                    int result = -1;
                    try{
                        result = std::stoi(p);
                    }
                    catch(...){
                        //
                    }

                    if(result >= 0){
                        return result;
                    }
                    throw std::runtime_error(std::string("invalid GLOBAL_THREAD_POOL_SIZE: ") + p);
                }
                return 0;
            }(),

            std::getenv("GLOBAL_THREAD_POOL_DISABLE"));
            return globalPool;
        }

    public:
        template<typename EvalCallable> static auto postEvalTask(EvalCallable && evalTask)
        {
#if __cplusplus >= 201703L
            using EvalResultType = typename std::invoke_result_t<EvalCallable, int>;
#else
            using EvalResultType = typename std::result_of_t<EvalCallable(int)>;
#endif

            // use shared_ptr instead of instance/move
            // otherwise need to mark posted lambda through pool.addTask() as mutable

            auto packedTaskPtr = std::make_shared<std::packaged_task<EvalResultType(int)>>([taskCpy = std::forward<EvalCallable>(evalTask)](int threadId) -> EvalResultType
            {
                return taskCpy(threadId);
            });

            getGlobalPool().addTask([packedTaskPtr](int threadId)
            {
                (*packedTaskPtr)(threadId);
            });
            return packedTaskPtr->get_future();
        }

    public:
        template<typename ForCallable> static void parallelFor(size_t beginIndex, size_t endIndex, const ForCallable & forTask)
        {
            if(beginIndex >= endIndex){
                return;
            }

            auto &pool = getGlobalPool();
            if(pool.disablePool){
                for(size_t i = beginIndex; i < endIndex; ++i){
                    forTask(0, i);
                }
                return;
            }

            const size_t groupSize = (endIndex - beginIndex + pool.poolSize - 1) / pool.poolSize;
            std::vector<std::future<void>> forTaskResult;
            forTaskResult.reserve(pool.poolSize);

            for(size_t group = 0; group < pool.poolSize; ++group){
                const size_t groupBegin = beginIndex + group * groupSize;
                const size_t groupEnd = std::min<size_t>(groupBegin + groupSize, endIndex);

                if(groupBegin >= groupEnd){
                    break;
                }

                forTaskResult.push_back(postEvalTask([groupBegin, groupEnd, &forTask](int threadId) -> void
                {
                    for(size_t i = groupBegin; i < groupEnd; ++i){
                        forTask(threadId, i);
                    }
                }));
            }

            for(auto &taskResult: forTaskResult){
                taskResult.get();
            }
        }
};
