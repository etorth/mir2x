#pragma once
#include <cstdint>
#include "serdesmsg.hpp"

SDInventory dbLoadInventory(uint32_t);
void dbStoreInventory(uint32_t, const SDInventory &);
void dbUpdateInventoryItem(uint32_t, const SDItem &);
bool dbRemoveInventoryItem(uint32_t, const SDItem &);
bool dbRemoveInventoryItem(uint32_t, uint32_t, uint32_t);
