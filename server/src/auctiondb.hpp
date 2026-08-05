#pragma once
#include "serdesmsg.hpp"

SDAuctionItemList dbQueryAuctionItemList(int);
bool dbRegisterAuctionItem(uint32_t, const SDItem &, const std::string &, uint64_t);
