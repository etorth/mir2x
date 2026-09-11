#pragma once
#include <cstdint>
#include <optional>
#include "serdesmsg.hpp"

SDInventory dbLoadInventory(uint32_t);
void dbStoreInventory(uint32_t, const SDInventory &);
void dbUpdateInventoryItem(uint32_t, const SDItem &);
bool dbRemoveInventoryItem(uint32_t, const SDItem &);
bool dbRemoveInventoryItem(uint32_t, uint32_t, uint32_t);
std::optional<SDItemGrant> dbGrantItemList(uint32_t, const std::vector<SDItem> &);
