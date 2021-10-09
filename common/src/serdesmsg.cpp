/*
 * =====================================================================================
 *
 *       Filename: serdesmsg.cpp
 *        Created: 01/24/2016 19:30:45
 *    Description: net message used by client and mono-server
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#include "mathf.hpp"
#include "dbcomid.hpp"
#include "fflerror.hpp"
#include "serdesmsg.hpp"
#include "dbcomrecord.hpp"

std::string SDInitNPChar::getFileName() const
{
    fflassert(DBCOM_MAPRECORD(mapID));
    if(filePath.empty()){
        return str_printf("%s.%s.lua", to_cstr(DBCOM_MAPRECORD(mapID).name), to_cstr(npcName));
    }
    else{
        return str_printf("%s/%s.%s.lua", to_cstr(filePath), to_cstr(DBCOM_MAPRECORD(mapID).name), to_cstr(npcName));
    }
}

std::u8string SDItem::getXMLLayout(const std::unordered_map<int, std::string> & params) const
{
    fflassert(*this);
    const auto &ir = DBCOM_ITEMRECORD(itemID);

    fflassert(ir);
    std::u8string xmlStr;

    xmlStr += str_printf(u8R"###( <layout> )###""\n");
    xmlStr += str_printf(u8R"###( <par>【名称】%s</par> )###""\n", ir.name);
    xmlStr += str_printf(u8R"###( <par>【类型】%s</par> )###""\n", ir.type);
    xmlStr += str_printf(u8R"###( <par>【重量】%d</par> )###""\n", ir.weight);

    if(const auto p = params.find(SDItem::XML_PRICE); p != params.end()){
        const auto priceColor = params.count(SDItem::XML_PRICECOLOR) ? to_cstr(params.at(SDItem::XML_PRICECOLOR)) : "green";
        xmlStr += str_printf(u8R"###( <par>【售价】<t color='%s'>%s</t></par> )###""\n", priceColor, to_cstr(p->second));
    }

    if(ir.equip.duration > 0){
        fflassert(duration[0] <= duration[1]);
        fflassert(duration[1] <= to_uz(ir.equip.duration));
        const auto duraColorStr = [this]() -> const char *
        {
            if(duration[0] <= to_uz(duration[1] / 4) || (duration[0] <= 1)){
                return "red";
            }
            else if(duration[0] <= to_uz(duration[1] / 2)){
                return "yellow";
            }
            else{
                return "white";
            }
        }();
        xmlStr += str_printf(u8R"###( <par>【持久】<t color='%s'>%zu/%zu/%d</t></par> )###""\n", duraColorStr, duration[0], duration[1], ir.equip.duration);
    }

    xmlStr += str_printf(u8R"###( <par></par> )###""\n");
    xmlStr += str_printf(u8R"###( <par>%s</par> )###""\n", str_haschar(ir.description) ? ir.description : u8"游戏处于开发阶段，暂无物品描述。");

    xmlStr += str_printf(u8R"###( <par></par> )###""\n");
    const auto fnAddAttrValuePair = [&xmlStr, this](int val0, int val1, const char8_t *valName, int extType)
    {
        if(const auto extAttr = getExtAttr<int>(extType); val0 > 0 || val1 > 0 || (extAttr.has_value() && extAttr.value() != 0)){
            const auto extAttrStr = str_printf("<t color='green'>（%+d）</t>", extAttr.value_or(0));
            xmlStr += str_printf(u8R"###( <par>%s %d - %d%s</par> )###""\n", to_cstr(valName), val0, val1 + extAttr.value_or(0), extAttr.has_value() ? extAttrStr.c_str() : "");
        }
    };

    const auto fnAddAttrValue = [&xmlStr, this](int val, const char8_t *valName, int extType)
    {
        if(const auto extAttr = getExtAttr<int>(extType); val || (extAttr.has_value() && extAttr.value() != 0)){
            const auto extAttrStr = str_printf("<t color='green'>（%+d）</t>", extAttr.value_or(0));
            xmlStr += str_printf(u8R"###( <par>%s %+d%s</par> )###""\n", to_cstr(valName), val + extAttr.value_or(0), extAttr.has_value() ? extAttrStr.c_str() : "");
        }
    };

    fnAddAttrValuePair(ir.equip. dc[0], ir.equip. dc[1], u8"攻击", SDItem::EA_DC);
    fnAddAttrValuePair(ir.equip. mc[0], ir.equip. mc[1], u8"魔法", SDItem::EA_MC);
    fnAddAttrValuePair(ir.equip. sc[0], ir.equip. sc[1], u8"道术", SDItem::EA_SC);
    fnAddAttrValuePair(ir.equip. ac[0], ir.equip. ac[1], u8"防御", SDItem::EA_AC);
    fnAddAttrValuePair(ir.equip.mac[0], ir.equip.mac[1], u8"魔防", SDItem::EA_MAC);

    fnAddAttrValue(ir.equip.dcHit,   u8"命中",     SDItem::EA_DCHIT);
    fnAddAttrValue(ir.equip.mcHit,   u8"魔法命中", SDItem::EA_MCHIT);
    fnAddAttrValue(ir.equip.dcDodge, u8"闪避",     SDItem::EA_DCDODGE);
    fnAddAttrValue(ir.equip.mcDodge, u8"魔法闪避", SDItem::EA_MCDODGE);
    fnAddAttrValue(ir.equip.speed,   u8"速度",     SDItem::EA_SPEED);
    fnAddAttrValue(ir.equip.comfort, u8"舒适度",   SDItem::EA_COMFORT);

    if(const auto extLuckCurseAttr = getExtAttr<int>(SDItem::EA_LUCKCURSE); ir.equip.luckCurse || (extLuckCurseAttr.value_or(0) != 0)){
        if(const auto luckCurseSum = ir.equip.luckCurse + extLuckCurseAttr.value_or(0); luckCurseSum >= 0){
            const auto extAttrStr = str_printf("<t color='%s'>（%+d）</t>", (extLuckCurseAttr.value_or(0) >= 0) ? "green" : "red", extLuckCurseAttr.value_or(0));
            xmlStr += str_printf(u8R"###( <par>幸运 %+d%s</par> )###""\n", luckCurseSum, extLuckCurseAttr.value_or(0) ? extAttrStr.c_str() : "");
        }
        else{
            const auto extAttrStr = str_printf("<t color='%s'>（%+d）</t>", (extLuckCurseAttr.value_or(0) >= 0) ? "red" : "green", -1 * extLuckCurseAttr.value_or(0));
            xmlStr += str_printf(u8R"###( <par>诅咒 %+d%s</par> )###""\n", std::abs(luckCurseSum), extLuckCurseAttr.value_or(0) ? extAttrStr.c_str() : "");
        }
    }

    fnAddAttrValue(ir.equip.hp.add,     u8"生命上限",   SDItem::EA_HPADD);
    fnAddAttrValue(ir.equip.hp.steal,   u8"生命盗取",   SDItem::EA_HPSTEAL);
    fnAddAttrValue(ir.equip.hp.recover, u8"生命恢复",   SDItem::EA_HPRECOVER);

    fnAddAttrValue(ir.equip.mp.add,     u8"魔法上限",   SDItem::EA_MPADD);
    fnAddAttrValue(ir.equip.mp.steal,   u8"魔法盗取",   SDItem::EA_MPSTEAL);
    fnAddAttrValue(ir.equip.mp.recover, u8"魔法恢复",   SDItem::EA_MPRECOVER);

    if(ir.equip.dcElem.fire    > 0){ xmlStr += str_printf(u8R"###( <par color='green'>攻击元素：火 %+d</par> )###""\n",   ir.equip.dcElem.fire   ); }
    if(ir.equip.dcElem.ice     > 0){ xmlStr += str_printf(u8R"###( <par color='green'>攻击元素：冰 %+d</par> )###""\n",   ir.equip.dcElem.ice    ); }
    if(ir.equip.dcElem.light   > 0){ xmlStr += str_printf(u8R"###( <par color='green'>攻击元素：雷 %+d</par> )###""\n",   ir.equip.dcElem.light  ); }
    if(ir.equip.dcElem.wind    > 0){ xmlStr += str_printf(u8R"###( <par color='green'>攻击元素：风 %+d</par> )###""\n",   ir.equip.dcElem.wind   ); }
    if(ir.equip.dcElem.holy    > 0){ xmlStr += str_printf(u8R"###( <par color='green'>攻击元素：神圣 %+d</par> )###""\n", ir.equip.dcElem.holy   ); }
    if(ir.equip.dcElem.dark    > 0){ xmlStr += str_printf(u8R"###( <par color='green'>攻击元素：暗黑 %+d</par> )###""\n", ir.equip.dcElem.dark   ); }
    if(ir.equip.dcElem.phantom > 0){ xmlStr += str_printf(u8R"###( <par color='green'>攻击元素：幻影 %+d</par> )###""\n", ir.equip.dcElem.phantom); }

    if(ir.equip.acElem.fire    > 0){ xmlStr += str_printf(u8R"###( <par color='green'>强防元素：火 %+d</par> )###""\n",   ir.equip.acElem.fire   ); }
    if(ir.equip.acElem.ice     > 0){ xmlStr += str_printf(u8R"###( <par color='green'>强防元素：冰 %+d</par> )###""\n",   ir.equip.acElem.ice    ); }
    if(ir.equip.acElem.light   > 0){ xmlStr += str_printf(u8R"###( <par color='green'>强防元素：雷 %+d</par> )###""\n",   ir.equip.acElem.light  ); }
    if(ir.equip.acElem.wind    > 0){ xmlStr += str_printf(u8R"###( <par color='green'>强防元素：风 %+d</par> )###""\n",   ir.equip.acElem.wind   ); }
    if(ir.equip.acElem.holy    > 0){ xmlStr += str_printf(u8R"###( <par color='green'>强防元素：神圣 %+d</par> )###""\n", ir.equip.acElem.holy   ); }
    if(ir.equip.acElem.dark    > 0){ xmlStr += str_printf(u8R"###( <par color='green'>强防元素：暗黑 %+d</par> )###""\n", ir.equip.acElem.dark   ); }
    if(ir.equip.acElem.phantom > 0){ xmlStr += str_printf(u8R"###( <par color='green'>强防元素：幻影 %+d</par> )###""\n", ir.equip.acElem.phantom); }

    if(ir.equip.acElem.fire    < 0){ xmlStr += str_printf(u8R"###( <par color='red'>弱防元素：火 %+d</par> )###""\n",   std::abs(ir.equip.acElem.fire   )); }
    if(ir.equip.acElem.ice     < 0){ xmlStr += str_printf(u8R"###( <par color='red'>弱防元素：冰 %+d</par> )###""\n",   std::abs(ir.equip.acElem.ice    )); }
    if(ir.equip.acElem.light   < 0){ xmlStr += str_printf(u8R"###( <par color='red'>弱防元素：雷 %+d</par> )###""\n",   std::abs(ir.equip.acElem.light  )); }
    if(ir.equip.acElem.wind    < 0){ xmlStr += str_printf(u8R"###( <par color='red'>弱防元素：风 %+d</par> )###""\n",   std::abs(ir.equip.acElem.wind   )); }
    if(ir.equip.acElem.holy    < 0){ xmlStr += str_printf(u8R"###( <par color='red'>弱防元素：神圣 %+d</par> )###""\n", std::abs(ir.equip.acElem.holy   )); }
    if(ir.equip.acElem.dark    < 0){ xmlStr += str_printf(u8R"###( <par color='red'>弱防元素：暗黑 %+d</par> )###""\n", std::abs(ir.equip.acElem.dark   )); }
    if(ir.equip.acElem.phantom < 0){ xmlStr += str_printf(u8R"###( <par color='red'>弱防元素：幻影 %+d</par> )###""\n", std::abs(ir.equip.acElem.phantom)); }

    if(false
            || ir.equip.load.body > 0
            || ir.equip.load.weapon > 0
            || ir.equip.load.inventory > 0){

        xmlStr += str_printf(u8R"###( <par></par> )###""\n");
        fnAddAttrValue(ir.equip.load.body,      u8"身体负重", SDItem::EA_LOADBODY);
        fnAddAttrValue(ir.equip.load.weapon,    u8"武器负重", SDItem::EA_LOADWEAPON);
        fnAddAttrValue(ir.equip.load.inventory, u8"包裹负重", SDItem::EA_LOADINVENTORY);
    }

    if(false
            || ir.equip.req.dc > 0
            || ir.equip.req.mc > 0
            || ir.equip.req.sc > 0
            || ir.equip.req.level > 0
            || str_haschar(ir.equip.req.job)){

        xmlStr += str_printf(u8R"###( <par></par> )###""\n");
        if(ir.equip.req.dc > 0){
            xmlStr += str_printf(u8R"###( <par>需要攻击 %d</par> )###""\n", ir.equip.req.dc);
        }

        if(ir.equip.req.mc > 0){
            xmlStr += str_printf(u8R"###( <par>需要魔法 %d</par> )###""\n", ir.equip.req.mc);
        }

        if(ir.equip.req.sc > 0){
            xmlStr += str_printf(u8R"###( <par>需要道术 %d</par> )###""\n", ir.equip.req.sc);
        }

        if(ir.equip.req.level > 0){
            xmlStr += str_printf(u8R"###( <par>需要等级 %d</par> )###""\n", ir.equip.req.level);
        }

        if(str_haschar(ir.equip.req.job)){
            xmlStr += str_printf(u8R"###( <par>需要职业 %s</par> )###""\n", ir.equip.req.job);
        }
    }

    xmlStr += str_printf(u8R"###( </layout> )###""\n");
    return xmlStr;
}

std::vector<SDItem> SDItem::buildGoldItem(size_t count)
{
    fflassert(count > 0);
    std::vector<SDItem> itemList;

    while(count > 0){
        if(count < 10){
            itemList.push_back(SDItem
            {
                .itemID = DBCOM_ITEMID(u8"金币（小）"),
                .count  = count,
            });
            break;
        }
        else if(count < 100){
            itemList.push_back(SDItem
            {
                .itemID = DBCOM_ITEMID(u8"金币（中）"),
                .count  = count,
            });
            break;
        }
        else if(count < 500){
            itemList.push_back(SDItem
            {
                .itemID = DBCOM_ITEMID(u8"金币（大）"),
                .count  = count,
            });
            break;
        }
        else if(count < 2000){
            itemList.push_back(SDItem
            {
                .itemID = DBCOM_ITEMID(u8"金币（特）"),
                .count  = count,
            });
            break;
        }
        else if(count < 5000){
            itemList.push_back(SDItem
            {
                .itemID = DBCOM_ITEMID(u8"金币（超）"),
                .count  = count,
            });
            break;
        }
        else{
            itemList.push_back(SDItem
            {
                .itemID = DBCOM_ITEMID(u8"金币（超）"),
                .count  = 5000,
            });
            count -= 5000;
        }
    }
    return itemList;
}

SDItem::operator bool () const
{
    if(!itemID){
        return false;
    }

    const auto &ir = DBCOM_ITEMRECORD(itemID);
    if(!ir){
        return false;
    }

    const auto maxCount = [&ir]() -> size_t
    {
        if(ir.isGold()){
            return SIZE_MAX;
        }

        if(ir.packable()){
            return SYS_INVGRIDMAXHOLD;
        }
        return 1;
    }();
    return count > 0 && count <= maxCount;
}

void SDWear::setWLItem(int i, SDItem item)
{
    if(!(i >= WLG_BEGIN && i < WLG_END)){
        throw fflerror("bad wltype: %d", i);
    }

    if(item.itemID){
        fflassert(item);
        m_list[i] = std::move(item);
    }
    else{
        m_list.erase(i);
    }
}

const SDItem &SDWear::getWLItem(int i) const
{
    if(!(i >= WLG_BEGIN && i < WLG_END)){
        throw fflerror("bad wltype: %d", i);
    }

    if(const auto p = m_list.find(i); p != m_list.end()){
        return p->second;
    }

    const static SDItem s_item{};
    return s_item;
}

std::unordered_set<uint64_t> SDInventory::getItemSeqIDSet() const
{
    std::unordered_set<uint64_t> result;
    result.reserve(m_list.size());

    for(const auto &item: m_list){
        if(!result.insert(buildItemSeqID(item.itemID, item.seqID)).second){
            throw fflerror("found duplicated item: itemID = %llu, seqID = %llu", to_llu(item.itemID), to_llu(item.seqID));
        }
    }
    return result;
}

size_t SDInventory::has(uint32_t itemID, uint32_t seqID) const
{
    const auto &ir = DBCOM_ITEMRECORD(itemID);

    fflassert(ir);
    fflassert(!ir.isGold());

    size_t count = 0;
    for(const auto &item: m_list){
        if((item.itemID == itemID) && ((seqID == 0) || (item.seqID == seqID))){
            count += item.count;
        }
    }
    return count;
}

const SDItem &SDInventory::find(uint32_t itemID, uint32_t seqID) const
{
    const auto &ir = DBCOM_ITEMRECORD(itemID);

    fflassert(ir);
    fflassert(!ir.isGold());

    for(const auto &item: m_list){
        if((item.itemID == itemID) && ((seqID == 0) || (item.seqID == seqID))){
            return item;
        }
    }

    const static SDItem s_item;
    return s_item;
}

const SDItem &SDInventory::add(SDItem newItem, bool keepSeqID)
{
    fflassert(newItem);
    const auto itemSeqIDSet = getItemSeqIDSet();

    if(keepSeqID){
        if(itemSeqIDSet.count(buildItemSeqID(newItem.itemID, newItem.seqID))){
            throw fflerror("found duplication with given item: itemID = %llu, seqID = %llu", to_llu(newItem.itemID), to_llu(newItem.seqID));
        }
        m_list.push_back(std::move(newItem));
        return m_list.back();
    }

    if(DBCOM_ITEMRECORD(newItem.itemID).packable()){
        for(auto &item: m_list){
            if(item.itemID != newItem.itemID){
                continue;
            }

            // TODO we only support change one item
            // currently can't do automatically merge: (55 + 56) -> (99, 12)
            if(item.count + newItem.count <= SYS_INVGRIDMAXHOLD){
                item.count += newItem.count;
                return item;
            }
        }
    }

    for(uint32_t seqID = 1;; ++seqID){
        if(!itemSeqIDSet.count(buildItemSeqID(newItem.itemID, seqID))){
            newItem.seqID = seqID;
            m_list.push_back(std::move(newItem));
            return m_list.back();
        }
    }
    throw bad_reach();
}

std::tuple<size_t, uint32_t, const SDItem *> SDInventory::remove(uint32_t itemID, uint32_t seqID, size_t count)
{
    const auto &ir = DBCOM_ITEMRECORD(itemID);

    fflassert(ir);
    fflassert(count > 0);
    fflassert(!ir.isGold());

    for(auto &item: m_list){
        if(item.itemID != itemID){
            continue;
        }

        if((seqID > 0) && (item.seqID != seqID)){
            continue;
        }

        if(count < item.count){
            item.count -= count;
            return {count, item.seqID, &item};
        }
        else{
            const auto removedCount = item.count;
            const auto removedSeqID = item.seqID;

            std::swap(m_list.back(), item);
            m_list.pop_back();
            return {removedCount, removedSeqID, nullptr};
        }
    }
    return {0, 0, nullptr};
}

void SDInventory::merge(uint32_t itemID, uint32_t fromSeqID, uint32_t toSeqID)
{
    if(!(fromSeqID && toSeqID)){
        throw fflerror("invalid fromSeqID = %llu, toSeqID = %llu", to_llu(fromSeqID), to_llu(toSeqID));
    }

    const auto &ir = DBCOM_ITEMRECORD(itemID);
    if(!ir){
        throw fflerror("invalid itemID = %llu", to_llu(itemID));
    }

    if(!ir.packable()){
        throw fflerror("item is not packable: itemID = %llu", to_llu(itemID));
    }

    int fromIndex = -1;
    int   toIndex = -1;

    for(int i = 0; to_uz(i) < m_list.size(); ++i){
        if(itemID == m_list.at(i).itemID){
            if(fromSeqID == m_list.at(i).seqID){
                fromIndex = i;
            }
            if(toSeqID == m_list.at(i).seqID){
                toIndex = i;
            }
        }
    }

    if(!(fromIndex >= 0 && toIndex >= 0)){
        return;
    }

    const size_t sum = m_list.at(fromIndex).count + m_list.at(toIndex).count;
    m_list.at(  toIndex).count = std::min<size_t>(sum, SYS_INVGRIDMAXHOLD);
    m_list.at(fromIndex).count = sum - m_list.at(toIndex ).count;

    if(m_list.at(fromIndex) <= 0){
        m_list.erase(m_list.begin() + fromIndex);
    }
}

bool SDMagicKeyList::setMagicKey(uint32_t magicID, char key)
{
    fflassert(DBCOM_MAGICRECORD(magicID));
    fflassert((key >= 'a' && key <= 'z') || (key >= '0' && key <= '9'));

    bool changed = false;
    for(auto p = keyList.begin(); p != keyList.end();){
        if(p->first != magicID && p->second == key){
            p = keyList.erase(p);
            changed = true;
        }
        else{
            p++;
        }
    }

    if(auto &magicKey = keyList[magicID]; magicKey != key){
        changed = true;
        magicKey = key;
    }
    return changed;
}
