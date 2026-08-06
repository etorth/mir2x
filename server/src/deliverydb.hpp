#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "serdesmsg.hpp"

struct DBDelivery
{
    std::string record {};
    SDChatMessage message {};
};

DBDelivery dbCreateDelivery(uint32_t, std::vector<SDItem>, std::string);
DBDelivery dbCreateDeliveryInTransaction(uint32_t, std::vector<SDItem>, std::string);
