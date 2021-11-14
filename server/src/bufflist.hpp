#pragma once
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
        std::vector<std::unique_ptr<BaseBuff>> m_buffList;

    public:
        /* ctor */  BuffList() = default;
        /* dtor */ ~BuffList() = default;

    public:
        BaseBuff *addBuff(std::unique_ptr<BaseBuff> buffPtr)
        {
            m_buffList.push_back(std::move(buffPtr));
            return m_buffList.back().get();
        }

    public:
        bool update()
        {
            // for update()/done()/runOnUpdate()
            // follow the same design way as BaseMagic in client/src/basemagic.hpp

            bool changed = false;
            for(size_t i = 0; i < m_buffList.size();){
                if(m_buffList[i]->update(m_timer.diff_msecf())){
                    std::swap(m_buffList[i], m_buffList.back());
                    m_buffList.pop_back();
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
        void runOnTrigger(int btgr)
        {
            fflassert(validBuffActTrigger(btgr));
            for(auto &p: m_buffList){
                p->runOnTrigger(btgr);
            }
        }

    public:
        std::vector<uint32_t> getIDList(bool showIconOnly = true) const
        {
            std::vector<uint32_t> idList;
            idList.reserve(m_buffList.size());

            for(const auto &p: m_buffList){
                if(!showIconOnly || p->getBR().icon.show){
                    idList.push_back(p->id());
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
                if(p->getBR().isBuff(name)){
                    result.push_back(p.get());
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
                for(auto &actr: p->m_actList){
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
