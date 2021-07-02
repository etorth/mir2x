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

SDItem SDItem::buildGoldItem(size_t count)
{
    fflassert(count > 0);
    return SDItem
    {
        .itemID = [count]() -> uint32_t
        {
            if(count < 10){
                return DBCOM_ITEMID(u8"金币（小）");
            }
            else if(count < 100){
                return DBCOM_ITEMID(u8"金币（中）");
            }
            else if(count < 500){
                return DBCOM_ITEMID(u8"金币（大）");
            }
            else if(count < 2000){
                return DBCOM_ITEMID(u8"金币（特）");
            }
            else{
                return DBCOM_ITEMID(u8"金币（超）");
            }
        }(),
        .count = count,
    };
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
        if(!item){
            throw fflerror("bad item: %s", to_cstr(item.str()));
        }
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
        if(item.itemID != itemID){
            continue;
        }

        if(!(seqID && (seqID == item.seqID))){
            continue;
        }
        count += item.count;
    }
    return count;
}

const SDItem &SDInventory::find(uint32_t itemID, uint32_t seqID) const
{
    const auto &ir = DBCOM_ITEMRECORD(itemID);

    fflassert(ir);
    fflassert(!ir.isGold());

    for(const auto &item: m_list){
        if(item.itemID != itemID){
            continue;
        }

        if(!(seqID && (seqID == item.seqID))){
            continue;
        }
        return item;
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

        if(seqID && (item.seqID != seqID)){
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
