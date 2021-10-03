#include "strf.hpp"
#include "uidf.hpp"
#include "fflerror.hpp"
#include "combatnode.hpp"
#include "dbcomrecord.hpp"

CombatNode getCombatNode(const SDWear & wear, uint64_t uid, int level)
{
    fflassert(uidf::isPlayer(uid));
    fflassert(level >= 0);

    CombatNode node;
    for(size_t i = WLG_BEGIN; i < WLG_END; ++i){
        if(const auto &item = wear.getWLItem(i)){
            const auto &ir = DBCOM_ITEMRECORD(item.itemID);
            fflassert(ir);

            node.ac[0] += ir.equip.ac[0];
            node.ac[1] += ir.equip.ac[1];

            node.dc[0] += ir.equip.dc[0];
            node.dc[1] += ir.equip.dc[1];

            node.mac[0] += ir.equip.mac[0];
            node.mac[1] += ir.equip.mac[1];

            node.mc[0] += ir.equip.mc[0];
            node.mc[1] += ir.equip.mc[1];

            node.sc[0] += ir.equip.sc[0];
            node.sc[1] += ir.equip.sc[1];

            node.hit += ir.equip.hit;
            node.dcDodge += ir.equip.dcDodge;
            node.mcDodge += ir.equip.mcDodge;
            node.speed += ir.equip.speed;
            node.comfort += ir.equip.comfort;

            node.load.hand += ir.equip.load.hand;
            node.load.body += ir.equip.load.body;
            node.load.inventory += ir.equip.load.inventory;
        }
    }

    node. ac[0] += (level / 2);
    node. ac[1] += (level / 2 + level % 2);
    node.mac[0] += (level / 2);
    node.mac[1] += (level / 2 + level % 2);

    node.load.hand += (10 + level * 2);
    node.load.body += (20 + level * 2);
    node.load.inventory += (100 + level * 10);

    if(uidf::hasPlayerJob(uid, JOB_WARRIOR)){
        node.dc[0] += (level / 2);
        node.dc[1] += (level / 2 + level % 2);
    }

    if(uidf::hasPlayerJob(uid, JOB_TAOIST)){
        node.sc[0] += (level / 2);
        node.sc[1] += (level / 2 + level % 2);
    }

    if(uidf::hasPlayerJob(uid, JOB_WIZARD)){
        node.mc[0] += (level / 2);
        node.mc[1] += (level / 2 + level % 2);
    }
    return node;
}
