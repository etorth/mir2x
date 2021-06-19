/*
 * =====================================================================================
 *
 *       Filename: inndb.hpp
 *        Created: 02/26/2016 21:48:43
 *    Description: Basic class of all integral based map cache
 *
 *                 Internal Database support for 
 *                 1. LRU
 *                 2. Easy for extension
 *
 *                 this class load resources with a external handler function
 *                 store it in a hash-table based cache
 *
 *                 to instantiation this class
 *                 1. define loadResource()
 *                 2. define freeResource()
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
#include <mutex>
#include <tuple>
#include <concepts>
#include <optional>
#include <type_traits>
#include <unordered_map>

template<std::unsigned_integral KeyT, typename ResT, bool ThreadSafe = false> class innDB
{
    private:
        struct DummyMutex
        {
            void   lock() {}
            void unlock() {}
        };

    private:
        using InnMutex = std::conditional_t<ThreadSafe, std::mutex, DummyMutex>;

    private:
        struct ResElement
        {
            size_t weight = 0;
            std::optional<ResT> res {};
            std::list<KeyT>::iterator keyiter {};
        };

    private:
        std::list<KeyT> m_keyList;
        std::unordered_map<KeyT, ResElement> m_elemList;

    private:
        InnMutex m_lock;

    private:
        size_t m_resSum = 0;

    private:
        const size_t m_resMax;

    public:
        innDB(size_t resMax)
            : m_resMax(resMax)
        {}

    public:
        virtual ~innDB()
        {
            clear();
        }

    public:
        virtual std::optional<std::tuple<ResT, size_t>> loadResource(KeyT  ) = 0;
        virtual void                                    freeResource(ResT &) = 0;

    public:
        void clear()
        {
            std::lock_guard<InnMutex> lockGurad(m_lock);
            for(auto &elemp: m_elemList){
                if(elemp.second.res.has_value()){
                    freeResource(elemp.second.res.value());
                }
            }

            m_resSum = 0;
            m_keyList.clear();
            m_elemList.clear();
        }

    protected:
        ResT *innLoad(KeyT key)
        {
            std::lock_guard<InnMutex> lockGurad(m_lock);
            if(auto p = m_elemList.find(key); p != m_elemList.end()){
                if(m_resMax > 0){
                    m_keyList.erase(p->second.keyiter);
                    m_keyList.push_front(key);
                    p->second.keyiter = m_keyList.begin();
                }

                if(p->second.res.has_value()){
                    return &(p->second.res.value());
                }
                return nullptr;
            }

            if(m_resMax > 0){
                m_keyList.push_front(key);
            }

            auto newRes = loadResource(key);
            auto emplaced = m_elemList.emplace(key, newRes.has_value() ? ResElement
            {
                .weight  = std::get<1>(newRes.value()),
                .res     = std::move(std::get<0>(newRes.value())),
                .keyiter = m_keyList.begin(),
            }

            : ResElement
            {
                .keyiter = m_keyList.begin(),
            }).first;

            if(m_resMax > 0){
                m_resSum += emplaced->second.weight;
                while(m_resSum > m_resMax){
                    fflassert(!m_keyList.empty());
                    fflassert(!m_elemList.empty());

                    auto poldest = m_elemList.find(m_keyList.back());

                    fflassert(poldest != m_elemList.end());
                    fflassert(m_resSum >= poldest->second.weight);

                    if(poldest->second.res.has_value()){
                        freeResource(poldest->second.res.value());
                    }
                    m_resSum -= poldest->second.weight;
                }
            }

            if(emplaced->second.res.has_value()){
                return &(emplaced->second.res.value());
            }
            return nullptr;
        }
};
