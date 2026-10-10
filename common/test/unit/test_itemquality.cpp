#include <cstdio>
#include <stdexcept>
#include "serdesmsg.hpp"

namespace
{
    void require(bool condition, const char *message)
    {
        if(!condition){
            throw std::runtime_error(message);
        }
    }

    SDItem qualityItem(uint32_t itemID, int quality)
    {
        return SDItem
        {
            .itemID = itemID,
            .extAttrList{SDItem::build_EA_QUALITY(quality)},
        };
    }

    void checkGeneration(uint32_t itemID, int threshold)
    {
        bool below = false;
        bool qualifies = false;
        const auto generated = SDItem::buildItemList(itemID, 512);
        require(generated.size() == 512, "quality-bearing copies were stacked");
        for(const auto &item: generated){
            const auto quality = item.getExtAttr<SDItem::EA_QUALITY_t>();
            require(quality.has_value() && quality.value() >= 1 && quality.value() <= 20, "generated quality out of range");
            require(item.duration[0] == 0 && item.duration[1] == 0, "quality replaced durability");
            below |= quality.value() < threshold;
            qualifies |= quality.value() >= threshold;
        }
        require(below && qualifies, "quality generation did not cover both sides of the quest threshold");
    }

    void checkMeat(uint32_t itemID, int threshold)
    {
        SDInventory inventory;
        inventory.add(qualityItem(itemID, threshold - 1), false);
        inventory.add(qualityItem(itemID, threshold), false);
        inventory.add(SDItem{.itemID = itemID}, false);
        const auto qualified = inventory.getQualityItemList(itemID, threshold, 1);
        require(qualified && qualified->size() == 1 && qualified->front().seqID == 2, "meat threshold was not inclusive");
        require(!inventory.getQualityItemList(itemID, threshold, 2), "low-quality meat qualified");
        require(inventory.has(itemID, 0) == 3, "failed meat query changed inventory");
    }
}

int main()
{
    try{
        const auto ironID = DBCOM_ITEMID(u8"铁矿");
        const auto beefID = DBCOM_ITEMID(u8"牛肉");
        const auto chickenID = DBCOM_ITEMID(u8"鸡肉");
        checkGeneration(ironID, 13);
        checkGeneration(beefID, 10);
        checkGeneration(chickenID, 4);
        checkMeat(beefID, 10);
        checkMeat(chickenID, 4);

        const auto iron = qualityItem(ironID, 13);
        const auto serialized = cerealf::deserialize<SDItem>(cerealf::serialize(iron));
        require(serialized.getExtAttr<SDItem::EA_QUALITY_t>() == 13, "serialized purity changed");
        const auto luaItem = SDItem::fromLuaVar(iron.asLuaVar());
        require(luaItem.extAttrList == iron.extAttrList, "Lua quality round trip changed metadata");
        require(iron.getXMLLayout().find(u8"【纯度】13") != std::u8string::npos, "ore XML does not display purity");
        require(qualityItem(beefID, 10).getXMLLayout().find(u8"【品质】10") != std::u8string::npos, "meat XML does not display quality");
        require(!SDItem::buildItemList(DBCOM_ITEMID(u8"木剑"), 1).front().getExtAttr<SDItem::EA_QUALITY_t>(), "equipment received material quality");

        SDInventory inventory;
        inventory.add(qualityItem(ironID, 12), false);
        for(int i = 0; i < 5; ++i){
            inventory.add(qualityItem(ironID, 13 + i), false);
        }
        inventory.add(SDItem{.itemID = ironID}, false);
        inventory.add(qualityItem(beefID, 20), false);
        inventory = cerealf::deserialize<SDInventory>(cerealf::serialize(inventory));

        require(!inventory.getQualityItemList(ironID, 13, 6), "insufficient qualifying ore produced a removal plan");
        require(inventory.has(ironID, 0) == 7, "failed ore preflight changed inventory");
        const auto plan = inventory.getQualityItemList(ironID, 13, 5);
        require(plan && plan->size() == 5, "five qualifying ore copies were not selected");
        for(const auto &item: plan.value()){
            require(item.seqID >= 2 && item.seqID <= 6, "removal selected a nonqualifying copy");
            const auto [removed, seqID, remaining] = inventory.remove(item.itemID, item.seqID, item.count, true);
            require(removed == 1 && seqID == item.seqID && !remaining, "exact-sequence removal failed");
        }
        require(inventory.has(ironID, 0) == 2 && inventory.find(ironID, 1) && inventory.find(ironID, 7), "removal consumed low or missing quality");
        require(inventory.has(beefID, 0) == 1, "ore removal consumed another item type");
        require(inventory.getQualityItemList(ironID, 0, 1).has_value(), "present quality did not qualify at zero");
        require(!inventory.getQualityItemList(ironID, 0, 2), "missing quality qualified at zero");

        std::puts("Item quality passed: generation, serialization, Lua, XML, inclusive thresholds, and exact-copy removal.");
        return 0;
    }
    catch(const std::exception &e){
        std::fprintf(stderr, "%s\n", e.what());
        return 1;
    }
}
