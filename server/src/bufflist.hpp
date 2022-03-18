#pragma once
#include <memory>
#include <vector>
#include <unordered_map>
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
        std::unordered_map<uint32_t, uint32_t> m_buffSeqBase;

    private:
        std::unordered_map<uint64_t, std::unique_ptr<BaseBuff>> m_buffList;

    public:
        /* ctor */  BuffList() = default;
        /* dtor */ ~BuffList() = default;

    public:
        uint32_t rollSeqID(uint32_t buffID)
        {
            return m_buffSeqBase[buffID] = std::max<uint32_t>(1, m_buffSeqBase[buffID] + 1);
        }

    public:
        BaseBuff * addBuff(std::unique_ptr<BaseBuff> buffPtr)
        {
            fflassert(buffPtr);
            fflassert(!m_buffList.count(buffPtr->buffSeq()), buffPtr->id(), buffPtr->seqID());

            const auto buffSeq = buffPtr->buffSeq();
            return (m_buffList[buffSeq] = std::move(buffPtr)).get();
        }

    public:
        void sendAura(uint64_t targetUID)
        {
            for(auto &p: m_buffList){
                p.second->sendAura(targetUID);
            }
        }

    public:
        void dispatchAura()
        {
            for(auto &p: m_buffList){
                p.second->dispatchAura();
            }
        }

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
        void erase(uint64_t buffSeq)
        {
            m_buffList.erase(buffSeq);
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
        void runOnMove()
        {
            // TODO BaseBuff::runOnMove() can call removeBuff()
            // which destruct the Buff itself and this causes dangling pointer

            for(auto p = m_buffList.begin(); p != m_buffList.end();){
                auto pnext = std::next(p);
                auto tag = p->first;
                auto pbuff = p->second.get();

                pbuff->runOnMove(tag);
                p = pnext;
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
