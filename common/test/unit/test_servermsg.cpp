#include <array>
#include <cstring>
#include <catch2/catch_test_macros.hpp>
#include "servermsg.hpp"

TEST_CASE("CO disappearance is a registered fixed-size server message", "[servermsg]")
{
    const ServerMsg msg(SM_CODISAPPEAR);
    REQUIRE(ServerMsg::headCodeValid(SM_CODISAPPEAR) != nullptr);
    REQUIRE(msg.name() == "SM_CODISAPPEAR");
    REQUIRE(msg.type() == 1);
    REQUIRE(msg.dataLen() == sizeof(SMCODisappear));
    REQUIRE(sizeof(SMCODisappear) == 2 * sizeof(uint64_t));

    const SMCODisappear payload{.UID = 123, .mapUID = 456};
    REQUIRE(msg.checkData(&payload, sizeof(payload)));
    REQUIRE_FALSE(msg.checkData(&payload, sizeof(payload) - 1));
}

TEST_CASE("CO disappearance preserves the subject and observer map", "[servermsg]")
{
    const SMCODisappear payload{.UID = 0x123456789abcdef0, .mapUID = 0xfedcba9876543210};
    std::array<uint8_t, sizeof(payload)> buffer;
    std::memcpy(buffer.data(), &payload, sizeof(payload));

    const auto decoded = ServerMsg::conv<SMCODisappear>(buffer.data(), buffer.size());
    REQUIRE(decoded.UID == payload.UID);
    REQUIRE(decoded.mapUID == payload.mapUID);
}
