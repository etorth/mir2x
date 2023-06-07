#pragma once
#include <map>
#include <utility>
#include <cstdint>
#include <functional>
#include "raiitimer.hpp"

class DelayCommandQueue final
{
    private:
        uint64_t m_delayCmdIndex = 1; // 0 used as invalid key

    private:
        std::map<std::pair<uint64_t, uint64_t>, std::function<void()>> m_delayCmdQ;

    public:
        DelayCommandQueue() = default;

    public:
        std::pair<uint64_t, uint64_t> addDelay(uint32_t delayTick, std::function<void()> fnCmd)
        {
            if(fnCmd){
                return m_delayCmdQ.insert(std::make_pair(std::make_pair(delayTick + hres_tstamp().to_msec(), m_delayCmdIndex++), std::move(fnCmd))).first->first;
            }
            return {0, 0};
        }

        void removeDelay(const std::pair<uint64_t, uint64_t> &key)
        {
            m_delayCmdQ.erase(key);
        }

    public:
        void exec()
        {
            while(!m_delayCmdQ.empty()){
                auto topiter = m_delayCmdQ.begin();
                if(hres_tstamp().to_msec() < topiter->first.first){
                    return;
                }

                // need to use an iterator instead of begin() when deleting
                // because addDelay can support nested addDelay() which may be returned by next begin()
                //
                //     addDelay(100, []()
                //     {
                //         ...
                //         addDelay(15, []()
                //         {
                //             ...
                //         });
                //     });

                topiter->second();
                m_delayCmdQ.erase(topiter);
            }
        }
};
