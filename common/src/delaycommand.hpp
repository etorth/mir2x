#pragma once
#include <queue>
#include <utility>
#include <cstdint>
#include <functional>
#include "fflerror.hpp"

class DelayCommand final
{
    private:
        uint64_t m_tick;
        uint64_t m_index; // to stablize heap sort

    private:
        std::function<void()> m_cmd;

    public:
        DelayCommand(uint64_t tick, uint64_t index, std::function<void()> cmd)
            : m_tick(tick)
            , m_index(index)
            , m_cmd(std::move(cmd))
        {
            fflassert(m_cmd);
        }

        bool operator < (const DelayCommand &other) const
        {
            if(m_tick != other.m_tick){
                return m_tick > other.m_tick;
            }
            return m_index > other.m_index;
        }

        uint32_t tick() const
        {
            return m_tick;
        }

    public:
        void operator () () const
        {
            m_cmd();
        }
};

// to support ServerObject::addDelay()
// if use std::priority_queue directly then I can not support nested addDelay():
//
//     addDelay(100, []()
//     {
//         ...
//         addDelay(15, []()
//         {
//             ...
//         });
//     });
//
// reason is when we calling the outer addDelay, it's accessing std::priority_queue::top()
// then if we call the inner addDelay, it changes the priority_queue while it's top() is being accessed
//
class DelayCommandQueue final
{
    private:
        uint64_t m_delayCmdIndex = 0;

    private:
        std::vector<DelayCommand> m_addedCmdQ;
        std::priority_queue<DelayCommand> m_delayCmdQ;

    public:
        DelayCommandQueue() = default;

    public:
        void addDelay(uint32_t, std::function<void()>);

    public:
        void exec();
};
