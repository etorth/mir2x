#pragma once
#include <memory>
#include <vector>
#include "basebuff.hpp"
#include "raiitimer.hpp"

class BuffList final
{
    private:
        hres_timer m_timer;

    private:
        std::vector<std::unique_ptr<BaseBuff>> m_list;

    public:
        /* ctor */  BuffList() = default;
        /* dtor */ ~BuffList() = default;

    public:
        BaseBuff *addBuff(std::unique_ptr<BaseBuff> buffPtr)
        {
            m_list.push_back(std::move(buffPtr));
            return m_list.back().get();
        }

    public:
        bool update()
        {
            // for update()/done()/runOnUpdate()
            // follow the same design way as BaseMagic in client/src/basemagic.hpp

            bool changed = false;
            for(size_t i = 0; i < m_list.size();){
                if(m_list[i]->update(m_timer.diff_msecf())){
                    std::swap(m_list[i], m_list.back());
                    m_list.pop_back();
                    changed = true;
                }
                else{
                    i++;
                }
            }

            m_timer.reset();
            return changed;
        }

    public:
        std::vector<uint32_t> getIDList() const
        {
            std::vector<uint32_t> idList;
            idList.reserve(m_list.size());

            for(const auto &p: m_list){
                idList.push_back(p->id());
            }
            return idList;
        }
};
