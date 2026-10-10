#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include "serdesmsg.hpp"

namespace
{
    SDItem qualityItem(uint32_t itemID, int quality)
    {
        return SDItem
        {
            .itemID = itemID,
            .extAttrList{SDItem::build_EA_QUALITY(quality)},
        };
    }
}

TEST_CASE("Materials receive per-copy quality independent of durability", "[itemquality]")
{
    const auto [itemID, threshold] = GENERATE(
        std::pair{DBCOM_ITEMID(u8"铁矿"), 13},
        std::pair{DBCOM_ITEMID(u8"牛肉"), 10},
        std::pair{DBCOM_ITEMID(u8"鸡肉"), 4});
    CAPTURE(itemID, threshold);
    bool below = false;
    bool qualifies = false;
    const auto generated = SDItem::buildItemList(itemID, 512);
    REQUIRE(generated.size() == 512);
    for(const auto &item: generated){
        const auto quality = item.getExtAttr<SDItem::EA_QUALITY_t>();
        REQUIRE(quality.has_value());
        REQUIRE(quality.value() >= 1);
        REQUIRE(quality.value() <= 20);
        REQUIRE(item.duration[0] == 0);
        REQUIRE(item.duration[1] == 0);
        below |= quality.value() < threshold;
        qualifies |= quality.value() >= threshold;
    }
    REQUIRE(below);
    REQUIRE(qualifies);
    REQUIRE_FALSE(SDItem::buildItemList(DBCOM_ITEMID(u8"木剑"), 1).front().getExtAttr<SDItem::EA_QUALITY_t>());
}

TEST_CASE("Meat thresholds are inclusive and missing quality never qualifies", "[itemquality]")
{
    const auto [itemID, threshold] = GENERATE(
        std::pair{DBCOM_ITEMID(u8"牛肉"), 10},
        std::pair{DBCOM_ITEMID(u8"鸡肉"), 4});
    CAPTURE(itemID, threshold);
    SDInventory inventory;
    inventory.add(qualityItem(itemID, threshold - 1), false);
    inventory.add(qualityItem(itemID, threshold), false);
    inventory.add(SDItem{.itemID = itemID}, false);
    const auto qualified = inventory.getQualityItemList(itemID, threshold, 1);
    REQUIRE(qualified.has_value());
    REQUIRE(qualified->size() == 1);
    REQUIRE(qualified->front().seqID == 2);
    REQUIRE_FALSE(inventory.getQualityItemList(itemID, threshold, 2));
    REQUIRE(inventory.has(itemID, 0) == 3);
}

TEST_CASE("Quality survives serialization and Lua conversion", "[itemquality]")
{
    const auto iron = qualityItem(DBCOM_ITEMID(u8"铁矿"), 13);
    const auto serialized = cerealf::deserialize<SDItem>(cerealf::serialize(iron));
    REQUIRE(serialized.getExtAttr<SDItem::EA_QUALITY_t>() == 13);
    const auto luaItem = SDItem::fromLuaVar(iron.asLuaVar());
    REQUIRE(luaItem.extAttrList == iron.extAttrList);
}

TEST_CASE("Item descriptions show ore purity and meat quality", "[itemquality]")
{
    REQUIRE(qualityItem(DBCOM_ITEMID(u8"铁矿"), 13).getXMLLayout().find(u8"【纯度】13") != std::u8string::npos);
    REQUIRE(qualityItem(DBCOM_ITEMID(u8"牛肉"), 10).getXMLLayout().find(u8"【品质】10") != std::u8string::npos);
}

TEST_CASE("Qualified ore removal preserves low-quality and unrelated items", "[itemquality]")
{
    const auto ironID = DBCOM_ITEMID(u8"铁矿");
    const auto beefID = DBCOM_ITEMID(u8"牛肉");
    SDInventory inventory;
    inventory.add(qualityItem(ironID, 12), false);
    for(int i = 0; i < 5; ++i){
        inventory.add(qualityItem(ironID, 13 + i), false);
    }
    inventory.add(SDItem{.itemID = ironID}, false);
    inventory.add(qualityItem(beefID, 20), false);
    inventory = cerealf::deserialize<SDInventory>(cerealf::serialize(inventory));

    REQUIRE_FALSE(inventory.getQualityItemList(ironID, 13, 6));
    REQUIRE(inventory.has(ironID, 0) == 7);
    const auto plan = inventory.getQualityItemList(ironID, 13, 5);
    REQUIRE(plan.has_value());
    REQUIRE(plan->size() == 5);
    for(const auto &item: plan.value()){
        CAPTURE(item.seqID);
        REQUIRE(item.seqID >= 2);
        REQUIRE(item.seqID <= 6);
        const auto [removed, seqID, remaining] = inventory.remove(item.itemID, item.seqID, item.count, true);
        REQUIRE(removed == 1);
        REQUIRE(seqID == item.seqID);
        REQUIRE_FALSE(remaining);
    }
    REQUIRE(inventory.has(ironID, 0) == 2);
    REQUIRE(inventory.find(ironID, 1));
    REQUIRE(inventory.find(ironID, 7));
    REQUIRE(inventory.has(beefID, 0) == 1);
    REQUIRE(inventory.getQualityItemList(ironID, 0, 1).has_value());
    REQUIRE_FALSE(inventory.getQualityItemList(ironID, 0, 2));
}
