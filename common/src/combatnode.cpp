#include <ranges>
#include "strf.hpp"
#include "jobf.hpp"
#include "uidf.hpp"
#include "mathf.hpp"
#include "fflerror.hpp"
#include "combatnode.hpp"

bool CombatNode::randPickLC() const
{
    if(luckCurse == 0){
        return false; // most common case deserves fast return
    }
    return std::clamp<int>(std::abs(luckCurse), 0, 10) >= 1 + mathf::rand<int>(0, 10);
}

int CombatNode::randPickDC() const { return randPickLC() ? (luckCurse > 0 ? maxDC() : minDC()) : mathf::rand<int>(minDC(), maxDC() + 1); }
int CombatNode::randPickMC() const { return randPickLC() ? (luckCurse > 0 ? maxMC() : minMC()) : mathf::rand<int>(minMC(), maxMC() + 1); }
int CombatNode::randPickSC() const { return randPickLC() ? (luckCurse > 0 ? maxSC() : minSC()) : mathf::rand<int>(minSC(), maxSC() + 1); }

int CombatNode::randPickAC () const { return mathf::rand<int>(std::ranges::min( ac), std::ranges::max( ac) + 1); }
int CombatNode::randPickMAC() const { return mathf::rand<int>(std::ranges::min(mac), std::ranges::max(mac) + 1); }

CombatNode getCombatNode(const SDWear & wear, const SDLearnedMagicList &magicList, int job, int level)
{
    fflassert(jobf::jobValid(job), job);
    fflassert(level >= 0, job);

    CombatNode node;
    for(size_t i = WLG_BEGIN; i < WLG_END; ++i){
        if(const auto &item = wear.getWLItem(i)){
            const auto &ir = DBCOM_ITEMRECORD(item.itemID);
            fflassert(ir);

            node.ac[0] += ir.equip.ac[0];
            node.ac[1] += ir.equip.ac[1];

            node.dc[0] += ir.equip.dc[0];
            node.dc[1] += ir.equip.dc[1];
            node.dc[1] += item.getExtAttr<SDItem::EA_DC_t>().value_or(0);

            node.mac[0] += ir.equip.mac[0];
            node.mac[1] += ir.equip.mac[1];

            node.mc[0] += ir.equip.mc[0];
            node.mc[1] += ir.equip.mc[1];

            node.sc[0] += ir.equip.sc[0];
            node.sc[1] += ir.equip.sc[1];

            node.dcHit += ir.equip.dcHit;
            node.mcHit += ir.equip.mcHit;
            node.dcDodge += ir.equip.dcDodge;
            node.mcDodge += ir.equip.mcDodge;

            node.speed += ir.equip.speed;
            node.comfort += ir.equip.comfort;
            node.luckCurse += ir.equip.luckCurse;

            node.dcElem.fire    += ir.equip.dcElem.fire;
            node.dcElem.ice     += ir.equip.dcElem.ice;
            node.dcElem.light   += ir.equip.dcElem.light;
            node.dcElem.wind    += ir.equip.dcElem.wind;
            node.dcElem.holy    += ir.equip.dcElem.holy;
            node.dcElem.dark    += ir.equip.dcElem.dark;
            node.dcElem.phantom += ir.equip.dcElem.phantom;

            node.acElem.fire    += ir.equip.acElem.fire;
            node.acElem.ice     += ir.equip.acElem.ice;
            node.acElem.light   += ir.equip.acElem.light;
            node.acElem.wind    += ir.equip.acElem.wind;
            node.acElem.holy    += ir.equip.acElem.holy;
            node.acElem.dark    += ir.equip.acElem.dark;
            node.acElem.phantom += ir.equip.acElem.phantom;

            node.load.body += ir.equip.load.body;
            node.load.weapon += ir.equip.load.weapon;
            node.load.inventory += ir.equip.load.inventory;
        }
    }

    node. ac[0] += (level / 2);
    node. ac[1] += (level / 2 + level % 2);
    node.mac[0] += (level / 2);
    node.mac[1] += (level / 2 + level % 2);

    node.load.body += (20 + level * 2);
    node.load.weapon += (10 + level * 2);
    node.load.inventory += (100 + level * 10);

    if(job & JOB_WARRIOR){
        node.dc[0] += (level / 2);
        node.dc[1] += (level / 2 + level % 2);
    }

    if(job & JOB_TAOIST){
        node.sc[0] += (level / 2);
        node.sc[1] += (level / 2 + level % 2);
    }

    if(job & JOB_WIZARD){
        node.mc[0] += (level / 2);
        node.mc[1] += (level / 2 + level % 2);
    }

    if(magicList.has(DBCOM_MAGICID(u8"基本剑术"))){
        node.dcHit += 2;
    }
    return node;
}
