#pragma once
#include <cstddef>
#include <cstdint>
#include <optional>

std::optional<size_t> dbLoadGold(uint32_t);
void dbUpdateGold(uint32_t, size_t);
std::optional<size_t> dbRemoveGold(uint32_t, size_t);
