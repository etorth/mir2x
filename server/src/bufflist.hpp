#pragma once
#include <memory>
#include <vector>
#include <algorithm>
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

        void updateAura(uint64_t uid)
        {
            sendAura(uid);
            for(auto &[buffSeq, pbuff]: m_buffList){
                if((pbuff->fromUID() == uid) && pbuff->fromAuraBAREF()){
                    pbuff->runOnBOMove(); // behave same as BO itself moves
                }
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
        void runOnBOMove()
        {
            // TODO BaseBuff::runOnBOMove() can call removeBuff()
            // which destruct the Buff itself and this causes dangling pointer

            for(auto p = m_buffList.begin(); p != m_buffList.end();){
                auto pnext = std::next(p);
                auto pbuff = p->second.get();

                pbuff->runOnBOMove();
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

            std::sort(idList.begin(), idList.end());
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

        std::vector<BaseBuff *> hasFromBuff(uint64_t fromUID, uint64_t fromBuffSeq, uint32_t buffID = 0)
        {
            std::vector<BaseBuff *> result;
            for(auto &elemp: m_buffList){
                if((elemp.second->fromUID() == fromUID) && (elemp.second->fromBuffSeq() == fromBuffSeq) && (buffID == 0 || buffID == elemp.second->id())){
                    result.push_back(elemp.second.get());
                }
            }
            return result;
        }

    public:
        BaseBuff *hasBuffSeq(uint64_t buffSeq) const
        {
            if(m_buffList.count(buffSeq)){
                return m_buffList.at(buffSeq).get();
            }
            return nullptr;
        }

    public:
        std::vector<BaseBuffAct *> hasBuffAct(const char8_t *name)
        {
            fflassert(str_haschar(name));
            std::vector<BaseBuffAct *> result;

            for(auto &elemp: m_buffList){
                for(auto &actPtr: elemp.second->m_actList){
                    if(actPtr->getBAR().isBuffAct(name)){
                        result.push_back(actPtr.get());
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
