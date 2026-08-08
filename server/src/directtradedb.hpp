#pragma once
#include <array>
#include <expected>
#include "serdesmsg.hpp"

// Re-read and validate both offers, then exchange gold and inventory in one
// transaction. Any validation or write failure leaves both players unchanged.
std::expected<std::array<SDDirectTradeResult, 2>, int> dbCommitDirectTrade(
        const SDDirectTradeOffer &,
        const SDDirectTradeOffer &);
