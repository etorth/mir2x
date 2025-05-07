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
        std::vector<std::unique_ptr<BaseBuff>> m_deadBufList;
        std::unordered_map<uint64_t, std::unique_ptr<BaseBuff>> m_activeBuffList;

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
            fflassert(!m_activeBuffList.contains(buffPtr->buffSeq()), buffPtr->id(), buffPtr->seqID());

            const auto buffSeq = buffPtr->buffSeq();
            return (m_activeBuffList[buffSeq] = std::move(buffPtr)).get();
        }

    public:
        corof::awaitable<> sendAura(uint64_t targetUID)
        {
            for(auto &p: m_activeBuffList){
                if(p.second){
                    co_await p.second->sendAura(targetUID);
                }
            }
        }

        corof::awaitable<> updateAura(uint64_t uid)
        {
            co_await sendAura(uid);
            for(auto &p: m_activeBuffList){
                if(p.second){
                    if((p.second->fromUID() == uid) && p.second->fromAuraBAREF()){
                        co_await p.second->runOnBOMove(); // behave same as BO itself moves
                    }
                }
            }
        }

    public:
        corof::awaitable<> dispatchAura()
        {
            for(auto &p: m_activeBuffList){
                if(p.second){
                    co_await p.second->dispatchAura();
                }
            }
        }

    public:
        void removeBuff(uint64_t buffSeq)
        {
            if(auto p = m_activeBuffList.find(buffSeq); p != m_activeBuffList.end() && p->second){
                m_deadBufList.push_back(std::move(p->second));
            }
        }

    public:
        void runOnTrigger(int btgr)
        {
            fflassert(validBuffActTrigger(btgr));
            for(auto &p: m_activeBuffList){
                if(p.second){
                    p.second->runOnTrigger(btgr);
                }
            }
        }

    public:
        void runOnBOMove()
        {
            for(auto &p: m_activeBuffList){
                if(p.second){
                    p.second->runOnBOMove();
                }
            }
        }

    public:
        std::vector<uint32_t> getIDList(bool showIconOnly = true) const
        {
            std::vector<uint32_t> idList;
            idList.reserve(m_activeBuffList.size());

            for(const auto &p: m_activeBuffList){
                if(p.second){
                    if(!showIconOnly || p.second->getBR().icon.show){
                        idList.push_back(p.second->id());
                    }
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

            for(auto &p: m_activeBuffList){
                if(p.second && p.second->getBR().isBuff(name)){
                    result.push_back(p.second.get());
                }
            }
            return result;
        }

        std::vector<BaseBuff *> hasFromBuff(uint64_t fromUID, uint64_t fromBuffSeq, uint32_t buffID = 0)
        {
            std::vector<BaseBuff *> result;
            for(auto &p: m_activeBuffList){
                if(p.second){
                    if((p.second->fromUID() == fromUID) && (p.second->fromBuffSeq() == fromBuffSeq) && (buffID == 0 || buffID == p.second->id())){
                        result.push_back(p.second.get());
                    }
                }
            }
            return result;
        }

    public:
        BaseBuff *hasBuffSeq(uint64_t buffSeq) const
        {
            if(m_activeBuffList.contains(buffSeq)){
                return m_activeBuffList.at(buffSeq).get();
            }
            return nullptr;
        }

    public:
        std::vector<BaseBuffAct *> hasBuffAct(const char8_t *name)
        {
            fflassert(str_haschar(name));
            std::vector<BaseBuffAct *> result;

            for(auto &p: m_activeBuffList){
                if(p.second){
                    for(auto &actPtr: p.second->m_actList){
                        if(actPtr->getBAR().isBuffAct(name)){
                            result.push_back(actPtr.get());
                        }
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
