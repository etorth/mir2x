#pragma once
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>
#include "serdesmsg.hpp"

std::tuple<uint64_t, uint64_t> dbSaveChatMessage(const SDChatPeerID &, const SDChatPeerID &, const std::string_view &, std::optional<uint64_t>);
std::optional<SDChatPeer> dbLoadChatPeer(uint64_t);
std::optional<SDChatMessage> dbQueryChatMessage(uint64_t);
std::vector<uint32_t> dbLoadChatGroupMemberList(uint32_t);
SDChatPeerList dbLoadChatGroupList(uint32_t);
SDChatPeerList dbQueryChatPeerList(const std::string &, bool, bool);
SDChatMessageList dbRetrieveLatestChatMessage(uint32_t, const std::span<const uint64_t> &, size_t, bool, bool);
SDChatPeer dbCreateChatGroup(uint32_t, const char *, const std::span<const uint32_t> &);
