#pragma once
#include <map>
#include <memory>
#include <vector>
#include "buff.hpp"
#include "buffactaura.hpp"
#include "buffactcontroller.hpp"
#include "buffacttrigger.hpp"
#include "buffactattributemodifier.hpp"
#include "buffactattackmodifier.hpp"
#include "buffactspellmodifier.hpp"
#include "raiitimer.hpp"

class BuffList final
{
    private:
        hres_timer m_timer;

    private:
        std::map<int, std::unique_ptr<BaseBuff>> m_buffList;

    public:
        /* ctor */  BuffList() = default;
        /* dtor */ ~BuffList() = default;

    public:
        std::tuple<int, BaseBuff *> addBuff(std::unique_ptr<BaseBuff> buffPtr)
        {
            if(m_buffList.empty()){
                m_buffList[1] = std::move(buffPtr);
            }
            else{
                m_buffList[m_buffList.rbegin()->first + 1] = std::move(buffPtr);
            }

            return
            {
                m_buffList.rbegin()->first,
                m_buffList.rbegin()->second.get(),
            };
        }

    public:
        void sendAura(uint64_t);

    public:
        bool update()
        {
            // for update()/done()/runOnUpdate()
            // follow the same design way as BaseMagic in client/src/basemagic.hpp

            bool changed = false;
            for(auto p = m_buffList.begin(); p != m_buffList.end();){
                if(p->second->update(m_timer.diff_msecf())){
                    p = m_buffList.erase(p);
                    changed = true;
                }
                else{
                    p++;
                }
            }

            m_timer.reset();
            return changed;
        }

    public:
        void erase(int tag)
        {
            m_buffList.erase(tag);
        }

    public:
        void runOnTrigger(int btgr)
        {
            fflassert(validBuffActTrigger(btgr));
            for(auto &p: m_buffList){
                p.second->runOnTrigger(btgr);
            }
        }

    public:
        std::vector<uint32_t> getIDList(bool showIconOnly = true) const
        {
            std::vector<uint32_t> idList;
            idList.reserve(m_buffList.size());

            for(const auto &p: m_buffList){
                if(!showIconOnly || p.second->getBR().icon.show){
                    idList.push_back(p.second->id());
                }
            }
            return idList;
        }

    public:
        std::vector<BaseBuff *> hasBuff(const char8_t *name)
        {
            fflassert(str_haschar(name));
            std::vector<BaseBuff *> result;

            for(auto &p: m_buffList){
                if(p.second->getBR().isBuff(name)){
                    result.push_back(p.second.get());
                }
            }
            return result;
        }

    public:
        std::vector<BaseBuffAct *> hasBuffAct(const char8_t *name)
        {
            fflassert(str_haschar(name));
            std::vector<BaseBuffAct *> result;

            for(auto &p: m_buffList){
                for(auto &actr: p.second->m_runList){
                    if(actr.ptr->getBAR().isBuffAct(name)){
                        result.push_back(actr.ptr.get());
                    }
                }
            }
            return result;
        }

    public:
        std::tuple<uint32_t, uint32_t> rollAttackModifier();

    public:
        std::vector<BaseBuffActAura              *> hasAura             (const char8_t *);
        std::vector<BaseBuffActController        *> hasController       (const char8_t *);
        std::vector<BaseBuffActTrigger           *> hasTrigger          (const char8_t *);
        std::vector<BaseBuffActAttributeModifier *> hasAttributeModifier(const char8_t *);
        std::vector<BaseBuffActAttackModifier    *> hasAttackModifier   (const char8_t *);
        std::vector<BaseBuffActSpellModifier     *> hasSpellModifier    (const char8_t *);
};
