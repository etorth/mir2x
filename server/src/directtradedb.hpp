#pragma once
#include <array>
#include <expected>
#include "serdesmsg.hpp"

// Atomic persistence boundary for direct trade. The function re-reads both
// inventories and gold balances inside one transaction, verifies that the
// prepared offers still match owned items, exchanges both sides, and returns
// complete authoritative results. Any validation or write failure leaves
// neither participant partially updated.
std::expected<std::array<SDDirectTradeResult, 2>, int> dbCommitDirectTrade(const SDDirectTradeCommit &);
