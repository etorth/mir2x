#include "mathf.hpp"
#include "dbcomid.hpp"
#include "fflerror.hpp"
#include "serdesmsg.hpp"

SDChatPeerID::SDChatPeerID(uint64_t argData)
    : m_data(argData)
{
    fflassert(id());
    fflassert(type() >= CPR_BEGIN, type());
    fflassert(type() <  CPR_END  , type());
}

SDChatPeerID::SDChatPeerID(ChatPeerType argType, uint32_t argID)
    : SDChatPeerID((to_u64(argType) << 32) | argID)
{}

void SDWear::setWLItem(int i, SDItem item)
{
    if(!(i >= WLG_BEGIN && i < WLG_END)){
        throw fflpanic("bad wltype: {}", i);
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
    fflassert(i >= WLG_BEGIN);
    fflassert(i < WLG_END);

    if(const auto p = m_list.find(i); p != m_list.end()){
        return p->second;
    }

    const static SDItem s_item{};
    return s_item;
}

std::unordered_set<uint64_t> SDInventory::getItemIDSeqSet() const
{
    std::unordered_set<uint64_t> result;
    result.reserve(m_list.size());

    for(const auto &item: m_list){
        if(!result.insert(item.itemIDSeq()).second){
            throw fflpanic("found duplicated item: itemID = {}, seqID = {}", item.itemID, item.seqID);
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

const SDItem *SDInventory::find(uint32_t itemID, uint32_t seqID) const
{
    const auto &ir = DBCOM_ITEMRECORD(itemID);

    fflassert(ir);
    fflassert(!ir.isGold());

    for(const auto &item: m_list){
        if((item.itemID == itemID) && ((seqID == 0) || (item.seqID == seqID))){
            return &item;
        }
    }
    return nullptr;
}

SDItem *SDInventory::find(uint32_t itemID, uint32_t seqID)
{
    return const_cast<SDItem *>(static_cast<const SDInventory *>(this)->find(itemID, seqID));
}

const SDItem &SDInventory::add(SDItem newItem, bool keepSeqID)
{
    fflassert(newItem);
    const auto itemIDSeqSet = getItemIDSeqSet();

    if(keepSeqID){
        if(itemIDSeqSet.contains(newItem.itemIDSeq())){
            throw fflpanic("found duplication with given item: itemID = {}, seqID = {}", newItem.itemID, newItem.seqID);
        }

        m_list.push_back(std::move(newItem));
        return m_list.back();
    }

    if(DBCOM_ITEMRECORD(newItem.itemID).packable()){
        for(auto &item: m_list){
            if(item.itemID != newItem.itemID){
                continue;
            }

            // we only support change one item
            // currently can't do automatically merge: (55 + 56) -> (99, 12)
            if(item.count + newItem.count <= SYS_INVGRIDMAXHOLD){
                item.count += newItem.count;
                return item;
            }
        }
    }

    for(uint32_t seqID = 1;; ++seqID){
        newItem.seqID = seqID;
        if(!itemIDSeqSet.contains(newItem.itemIDSeq())){
            m_list.push_back(std::move(newItem));
            return m_list.back();
        }
    }

    throw fflvalue(newItem, keepSeqID);
}

std::tuple<size_t, uint32_t, const SDItem *> SDInventory::remove(uint32_t itemID, uint32_t seqID, size_t count, bool strict)
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
        else if(!strict || (count == item.count)){
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
        throw fflpanic("invalid fromSeqID = {}, toSeqID = {}", fromSeqID, toSeqID);
    }

    const auto &ir = DBCOM_ITEMRECORD(itemID);
    if(!ir){
        throw fflpanic("invalid itemID = {}", itemID);
    }

    if(!ir.packable()){
        throw fflpanic("item is not packable: itemID = {}", itemID);
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
