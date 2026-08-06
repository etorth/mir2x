#pragma once
#include <cstddef>
#include <cstdint>
#include <expected>
#include "serdesmsg.hpp"

struct DBAuctionBuyResult
{
    size_t buyerGold = 0;
    uint32_t sellerDBID = 0;
    SDChatMessage buyerMessage {};
    SDChatMessage sellerMessage {};
};

struct DBAuctionUnregisterResult
{
    SDChatMessage sellerMessage {};
};

SDAuctionItemList dbQueryAuctionItemList(int);
bool dbRegisterAuctionItem(uint32_t, const SDItem &, const std::string &, uint64_t);

std::expected<DBAuctionBuyResult       , int> dbBuyAuctionItem       (uint32_t, uint64_t);
std::expected<DBAuctionUnregisterResult, int> dbUnregisterAuctionItem(uint32_t, uint64_t);
