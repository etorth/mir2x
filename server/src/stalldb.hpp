#pragma once
#include <vector>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include "serdesmsg.hpp"

// stall items live in tbl_stallitem, not in the inventory, like belt items
// database is the source of truth, it works for both online and offline sellers
//
// all functions changing inventory take an inventory copy of the player
// caller applies the copy to player memory only when the function succeeds

struct DBStallBuyResult
{
    size_t buyerGold = 0;
    SDItem buyerItem {}; // inventory item updated by this purchase

    SDChatMessage sellerMessage {};
    bool empty = false; // stall sold out and got deleted
};

std::optional<SDStall> dbLoadStall(uint32_t);
std::vector<SDOfflineStall> dbLoadOfflineStallList(uint32_t);

std::expected<SDStallItem, int> dbAddStallItem   (uint32_t, const SDItem &, size_t);
std::expected<SDItem     , int> dbRemoveStallItem(uint32_t, SDInventory &, uint32_t, uint32_t);

bool dbStartStall(uint32_t, const std::string &, uint32_t, int, int, const SDOfflineStall &);
std::vector<SDItem> dbCloseStall(uint32_t, SDInventory &);

std::expected<DBStallBuyResult, int> dbBuyStallItem(uint32_t, SDInventory &, uint32_t, uint32_t, int, int, uint32_t, uint32_t, size_t, size_t);
