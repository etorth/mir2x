#pragma once
#include <array>
#include <expected>
#include "serdesmsg.hpp"

std::expected<std::array<SDDirectTradeResult, 2>, int> dbCommitDirectTrade(const SDDirectTradeOffer &, const SDDirectTradeOffer &);
