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

void SDItem::checkEx() const
{
    // called before insert record to database
    // make it super safe

    if(!this[0]){
        throw fflerror("invalid item: %s", to_cstr(str()));
    }
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

    if(const size_t maxCount = ir.packable() ? SYS_INVGRIDMAXHOLD : 1; count <= 0 || count > maxCount){
        return false;
    }
    return true;
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
    if(!ir){
        throw fflerror("invalid itemID: %llu", to_llu(itemID));
    }

    if(itemID == DBCOM_ITEMID(u8"金币")){
        throw fflerror("invalid type: 金币");
    }

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

const SDItem &SDInventory::add(SDItem newItem, bool keepSeqID)
{
    const auto itemSeqIDSet = getItemSeqIDSet();
    if(keepSeqID){
        if(itemSeqIDSet.count(buildItemSeqID(newItem.itemID, newItem.seqID))){
            throw fflerror("found duplication with given item: itemID = %llu, seqID = %llu", to_llu(newItem.itemID), to_llu(newItem.seqID));
        }
        m_list.push_back(std::move(newItem));
        return m_list.back();
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
    if(!(DBCOM_ITEMRECORD(itemID) && count > 0)){
        throw fflerror("invalid arguments: itemID = %llu, seqID = %llu, count = %zu", to_llu(itemID), to_llu(seqID), count);
    }

    if(itemID == DBCOM_ITEMID(u8"金币")){
        throw fflerror("invalid type: 金币");
    }

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
