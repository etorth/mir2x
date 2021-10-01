#include "strf.hpp"
#include "uidf.hpp"
#include "fflerror.hpp"
#include "combatnode.hpp"
#include "dbcomrecord.hpp"

CombatNode getCombatNode(const SDWear & wear, uint64_t uid, int level)
{
    fflassert(uidf::isPlayer(uid));
    fflassert(level > 0);

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

            node.mdc[0] += ir.equip.mdc[0];
            node.mdc[1] += ir.equip.mdc[1];

            node.sdc[0] += ir.equip.sdc[0];
            node.sdc[1] += ir.equip.sdc[1];

            node.hit += ir.equip.hit;
            node.dodge += ir.equip.dodge;
            node.speed += ir.equip.speed;
            node.comfort += ir.equip.comfort;
        }
    }

    node. ac[0] += (level / 2);
    node. ac[1] += (level / 2 + level % 2);
    node.mac[0] += (level / 2);
    node.mac[1] += (level / 2 + level % 2);

    if(uidf::hasPlayerJob(uid, JOB_WARRIOR)){
        node.dc[0] += (level / 2);
        node.dc[1] += (level / 2 + level % 2);
    }

    if(uidf::hasPlayerJob(uid, JOB_TAOIST)){
        node.sdc[0] += (level / 2);
        node.sdc[1] += (level / 2 + level % 2);
    }

    if(uidf::hasPlayerJob(uid, JOB_WIZARD)){
        node.mdc[0] += (level / 2);
        node.mdc[1] += (level / 2 + level % 2);
    }
    return node;
}
