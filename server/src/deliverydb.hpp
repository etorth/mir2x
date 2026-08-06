#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>
#include "serdesmsg.hpp"

struct DBDelivery
{
    std::string record {};
    SDChatMessage message {};
};

std::tuple<uint64_t, uint64_t> dbSaveChatMessage(const SDChatPeerID &, const SDChatPeerID &, const std::string_view &, std::optional<uint64_t>);
DBDelivery dbCreateDelivery(uint32_t, std::vector<SDItem>, std::string);
DBDelivery dbCreateDeliveryInTransaction(uint32_t, std::vector<SDItem>, std::string);
