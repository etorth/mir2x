#include <cmath>
#include <vector>
#include <string_view>
#include "mathf.hpp"
#include "dbcomid.hpp"
#include "fflerror.hpp"
#include "serdesmsg.hpp"
#include "jobf.hpp"

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

std::optional<std::vector<SDItem>> SDInventory::getQualityItemList(uint32_t itemID, int minQuality, size_t count) const
{
    const auto &ir = DBCOM_ITEMRECORD(itemID);
    fflassert(ir);
    fflassert(!ir.isGold());
    fflassert(minQuality >= 0);
    fflassert(count > 0);

    std::vector<SDItem> result;
    for(const auto &item: m_list){
        if(item.itemID != itemID){
            continue;
        }

        const auto quality = item.getExtAttr<SDItem::EA_QUALITY_t>();
        if(!quality.has_value() || quality.value() < minQuality){
            continue;
        }

        auto selected = item;
        selected.count = std::min<size_t>(count, item.count);
        count -= selected.count;
        result.push_back(std::move(selected));
        if(count == 0){
            return result;
        }
    }
    return {};
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

std::vector<std::array<int, 2>> SDSafeZone::edgeGridList(int mapW, int mapH) const
{
    fflassert(mapW > 0, mapW);
    fflassert(mapH > 0, mapH);

    // scan line: covered spans of each row, half-open [x0, x1)
    // a circle row covers |x - cx| <= k with the largest k that k * k <= r * r - dy * dy, same grids as contains()
    std::vector<std::vector<std::pair<int, int>>> rowSpanList(mapH);
    const auto fnAddSpan = [mapW, mapH, &rowSpanList](int64_t y, int64_t x0, int64_t x1)
    {
        x0 = std::max<int64_t>(x0, 0);
        x1 = std::min<int64_t>(x1, mapW);

        if(y >= 0 && y < mapH && x0 < x1){
            rowSpanList[y].emplace_back(int(x0), int(x1));
        }
    };

    for(const auto &[x, y, w, h]: rectList){
        for(int64_t ry = std::max<int64_t>(y, 0); ry < std::min<int64_t>(int64_t(y) + h, mapH); ++ry){
            fnAddSpan(ry, x, int64_t(x) + w);
        }
    }

    for(const auto &[cx, cy, r]: circleList){
        for(int64_t ry = std::max<int64_t>(int64_t(cy) - r, 0); ry <= std::min<int64_t>(int64_t(cy) + r, mapH - 1); ++ry){
            const int64_t remain = int64_t(r) * r - (ry - cy) * (ry - cy);
            auto k = int64_t(std::sqrt(double(remain)));

            while(k * k > remain){
                --k;
            }

            while((k + 1) * (k + 1) <= remain){
                ++k;
            }
            fnAddSpan(ry, cx - k, cx + k + 1);
        }
    }

    for(auto &spanList: rowSpanList){
        std::ranges::sort(spanList);
        std::vector<std::pair<int, int>> mergedList;

        for(const auto &span: spanList){
            if(!mergedList.empty() && span.first <= mergedList.back().second){ // merge touching spans too, so a span end is next to an uncovered grid
                mergedList.back().second = std::max<int>(mergedList.back().second, span.second);
            }
            else{
                mergedList.push_back(span);
            }
        }
        spanList = std::move(mergedList);
    }

    const std::vector<std::pair<int, int>> fullRow {{0, mapW}}; // rows out of map count as in the zone

    std::vector<int> edgeXList;
    std::vector<std::array<int, 2>> result;

    for(int y = 0; y < mapH; ++y){
        const auto &spanList = rowSpanList[y];
        edgeXList.clear();

        const auto fnAddUncovered = [&spanList, &edgeXList](const std::vector<std::pair<int, int>> &nbrSpanList)
        {
            auto nbr = nbrSpanList.begin();
            for(const auto &[x0, x1]: spanList){
                for(int x = x0; x < x1;){
                    while(nbr != nbrSpanList.end() && nbr->second <= x){
                        ++nbr;
                    }

                    if(nbr == nbrSpanList.end() || nbr->first >= x1){
                        for(; x < x1; ++x){
                            edgeXList.push_back(x);
                        }
                    }
                    else if(nbr->first > x){
                        for(; x < nbr->first; ++x){
                            edgeXList.push_back(x);
                        }
                    }
                    else{
                        x = nbr->second;
                    }
                }
            }
        };

        for(const auto &[x0, x1]: spanList){
            if(x0 > 0){
                edgeXList.push_back(x0);
            }

            if(x1 < mapW){
                edgeXList.push_back(x1 - 1);
            }
        }

        fnAddUncovered((y > 0       ) ? rowSpanList[y - 1] : fullRow);
        fnAddUncovered((y + 1 < mapH) ? rowSpanList[y + 1] : fullRow);

        std::ranges::sort(edgeXList);
        const auto [uniqueBegin, uniqueEnd] = std::ranges::unique(edgeXList);
        edgeXList.erase(uniqueBegin, uniqueEnd);

        for(const auto x: edgeXList){
            result.push_back({x, y});
        }
    }
    return result;
}

bool SDMagicKeyList::setMagicKey(uint32_t magicID, char key)
{
    fflassert(DBCOM_MAGICRECORD(magicID));
    fflassert(validKey(key), key);

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

bool SDTeamReq::valid() const
{
    if(maxMember < 0 || maxMember > SYS_MAXTEAMMEMBER){
        return false;
    }

    if(!std::ranges::any_of(jobReqList, [](const auto &req){ return req.has_value(); })){
        return false;
    }

    for(size_t i = 0; i < jobReqList.size(); ++i){
        if(!jobReqList.at(i).has_value()){
            continue;
        }

        if(!jobReqList.at(i)->valid()){
            return false;
        }

        for(const auto magicID: jobReqList.at(i)->magicIDList){
            if(const auto &mr = DBCOM_MAGICRECORD(magicID)){
                if(str_haschar(mr.req.job)){
                    if(to_u8sv(mr.req.job) == jobf::jobName(jobf::jobFromGfxIndex(i)).front()){
                        continue;
                    }
                }
            }
            return false;
        }
    }
    return true;
}

bool SDTeamReq::accept(int job, int level, int capacity, const std::function<bool(uint32_t)> &hasMagic) const
{
    for(const auto i: jobf::jobGfxIndex(job)){
        if(!jobReqList.at(i).has_value()){
            continue;
        }

        if(jobReqList.at(i)->minLevel >= 0 && level < jobReqList.at(i)->minLevel){
            return false;
        }

        if(jobReqList.at(i)->maxLevel >= 0 && level > jobReqList.at(i)->maxLevel){
            return false;
        }

        if(jobReqList.at(i)->minCapacity > 0 && capacity < jobReqList.at(i)->minCapacity){
            return false;
        }

        return std::ranges::all_of(jobReqList.at(i)->magicIDList, [&hasMagic](uint32_t magicID){ return hasMagic && hasMagic(magicID); });
    }
    return false;
}
